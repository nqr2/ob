#include <ob/Array.h>
#include <ob/Assert.h>
#include <ob/Bytecode.h>
#include <ob/Context.h>
#include <ob/Object.h>
#include <ob/Parse.h>
#include <ob/Serial.h>
#include <ob/String.h>

#define OB_LOG_MODULE "Parse"
#include <ob/Log.h>

#include <ctype.h>
#include <stdint.h>
#include <string.h>

static bool is_operator(char chr) {
  if (!ispunct(chr)) {
    return false;
  }

  switch (chr) {
  case '(':
  case ')':
  case '{':
  case '}':
  case '[':
  case ']':
  case '.':
  case '\'':
  case '"':
    return false;
  default:
    return true;
  }
}

static bool is_word_start(char chr) {
  return isalpha(chr);
}

static bool is_word_tail(char chr) {
  return is_word_start(chr);
}

typedef struct {
  ob_Context context;
  ob_ObjMethod *output;
  size_t remaining;
  const char *head;
} Reader;

Reader rdr_new(ob_Context context, ob_ObjMethod *output, size_t length,
               const char *text) {
  Reader rdr = {};
  rdr.context = context;
  rdr.output = output;
  rdr.remaining = length;
  rdr.head = text;
  return rdr;
}

void rdr_next(Reader *rdr) {
  rdr->head++;
  rdr->remaining--;
}

// i wish C had lambdas...
void rdr_takewhile(Reader *rdr, bool (*pred)(char)) {
  while (pred(*rdr->head)) {
    rdr_next(rdr);
  }
}

static void push_literal(Reader *rdr, ob_Obj obj) {
  auto index = obarr_length(&rdr->output->literals, sizeof(ob_Obj));
  obarr_push(&rdr->output->literals, sizeof(ob_Obj), (const void *)&obj);

  OB_DEBUG("push literal: %zu", index);

  index = obbc_append_index(&rdr->output->bytecode, index);
  obbc_append_insn(&rdr->output->bytecode, OBBC_MAKE(OBBC_PUSH_LITERAL, index));
}

/*
 * The grammar is more or less:
 *
 * file = { toplevel }.
 * toplevel = expression '.'.
 * expression = keyword-message.
 * keyword-message = [ binary-message ] { keyword binary-message }.
 * binary-message = [ unary-message ] { operator unary-message }.
 * unary-message = [ primary ] { word }.
 * primary = literal | '(' expressions ')'.
 * literal = <integer> | <float> | <string> | '(' ')'.
 * expressions = expression { '.' expression }.
 */

static void p_expression(Reader *rdr);

static void p_skip_blank(Reader *rdr) {
  while (rdr->remaining > 0) {
    if (isspace(*rdr->head)) {
      while (isspace(*rdr->head)) {
        rdr_next(rdr);
      }

      continue;
    }

    if (*rdr->head == '"') {
      rdr_next(rdr);

      while (*rdr->head != '"') {
        rdr_next(rdr);
      }

      rdr_next(rdr);

      continue;
    }

    break;
  }
}

static void p_paren(Reader *rdr) {
  rdr_next(rdr);

  p_skip_blank(rdr);

  if (*rdr->head == ')') {
    // () is the nil literal

    ob_Obj nil = NULL;
    push_literal(rdr, nil);
  } else {
    // expression { . expression }
    p_expression(rdr);
    p_skip_blank(rdr);

    while (*rdr->head == '.') {
      rdr_next(rdr);
      p_expression(rdr);
      p_skip_blank(rdr);
    }
  }

  ASSERT(*rdr->head == ')', "expected a closing ), got a %c", *rdr->head);

  rdr_next(rdr);
}

static void p_array(Reader *rdr) {
  size_t items = 0;

  rdr_next(rdr);

  p_skip_blank(rdr);

  if (*rdr->head != ']') {
    items += 1;

    // expression { . expression }
    p_expression(rdr);
    p_skip_blank(rdr);

    while (*rdr->head == '.') {
      items += 1;
      rdr_next(rdr);

      p_expression(rdr);
      p_skip_blank(rdr);
    }
  }

  ASSERT(*rdr->head == ']', "expected a closing `]`, got a %c", *rdr->head);
  rdr_next(rdr);

  items = obbc_append_index(&rdr->output->bytecode, items);
  obbc_append_insn(&rdr->output->bytecode, OBBC_MAKE(OBBC_ARRAY, items));
}

static void p_method(Reader *rdr) {
  rdr_next(rdr);

  p_skip_blank(rdr);

  auto original = rdr->output;

  auto new = ob_create_method(rdr->context);
  auto method = ob_cast_method(new);

  rdr->output = method;

  // | {word} |
  auto buf = (ob_Array){};
  obarr_init(&buf, rdr->context->allocator);

  if (*rdr->head == '|') {
    rdr_next(rdr);

    p_skip_blank(rdr);

    while (true) {
      if (*rdr->head == '|') {
        rdr_next(rdr);
        break;
      }

      obarr_clear(&buf);

      auto begin = rdr->head;
      rdr_takewhile(rdr, is_word_tail);

      obarr_push(&buf, sizeof(char) * (rdr->head - begin), begin);

      // TODO: also intern this
      auto str = obstr_create(rdr->context, buf.size, buf.data);
      obarr_push(&rdr->output->parameters, sizeof(ob_Str), (void *)&str);

      p_skip_blank(rdr);

      if (!((*rdr->head == '|') || isalpha(*rdr->head))) {
        ASSERT(false, "unexpected character in argument list: `%c`",
               *rdr->head);
      }
    }
  }

  obarr_free(&buf);

  // expression { . expression }
  p_expression(rdr);
  p_skip_blank(rdr);

  while (*rdr->head == '.') {
    rdr_next(rdr);
    p_expression(rdr);
    p_skip_blank(rdr);
  }

  rdr->output = original;

  push_literal(rdr, new);

  p_skip_blank(rdr);
  p_skip_blank(rdr);

  ASSERT(*rdr->head == '}', "expected a closing `}`, got a `%s`", rdr->head);

  rdr_next(rdr);
}

static ob_Str p_string_inner(Reader *rdr) {
  rdr_next(rdr);

  auto buf = (ob_Array){};
  obarr_init(&buf, rdr->context->allocator);

  auto begin = rdr->head;

  while (*rdr->head != '\'') {
    if (*rdr->head == '\\') {
      obarr_push(&buf, sizeof(char) * (rdr->head - begin - 1), begin - 1);

      rdr_next(rdr);

      auto escaped = *rdr->head;

      switch (*rdr->head) {
      case 'n':
        escaped = '\n';
        break;
      case 't':
        escaped = '\t';
        break;
      default:
        break;
      }

      obarr_push(&buf, sizeof(char), &escaped);
      rdr_next(rdr);

      begin = rdr->head;
      continue;
    }

    rdr_next(rdr);
  }

  obarr_push(&buf, sizeof(char) * (rdr->head - begin), begin);

  ASSERT(*rdr->head == '\'', "expected a `'`, got a %s", rdr->head);
  rdr_next(rdr);

  auto str = obstr_create(rdr->context, buf.size, buf.data);
  obarr_free(&buf);

  return str;
}

static void p_string(Reader *rdr) {
  auto str = p_string_inner(rdr);
  auto obj = ob_create_string(rdr->context, str);

  push_literal(rdr, obj);
}

static void p_symbol(Reader *rdr) {
  rdr_next(rdr);

  ob_Str sel = NULL;
  auto sym = (ob_Array){};
  obarr_init(&sym, rdr->context->allocator);

  auto begin = rdr->head;

  if (isalpha(*rdr->head)) {
    while (true) {
      rdr_takewhile(rdr, is_word_tail);

      if (*rdr->head != ':') {
        break;
      }

      rdr_next(rdr);
    }
  }

  else if (*rdr->head == '\'') {
    sel = p_string_inner(rdr);
    goto after_str;
  }

  else {
    while (is_operator(*rdr->head)) {
      rdr_next(rdr);
    }
  }

  obarr_push(&sym, sizeof(char) * (rdr->head - begin), begin);

  sel = obstr_create(rdr->context, sym.size, sym.data);

after_str:
  auto objsel = ob_create_symbol(rdr->context, sel);

  push_literal(rdr, objsel);
  obarr_free(&sym);
}

static bool p_primary(Reader *rdr) {
  /*
   * We know that explicit receivers can be one of:
   * -  Number literals, which always start with a digit,
   * -  Parenthesized expressions,
   * -  String literals, which always start with a ',
   * or Symbol literals, which start with a #.
   *
   * Since anything else is a message, we can just check for this only.
   */

  p_skip_blank(rdr);

  if (isdigit(*rdr->head)) {
    int64_t num = 0;
    double fnum = 0;

    while (isdigit(*rdr->head)) {
      auto digit = *rdr->head - '0';
      num = (num * 10) + digit;
      rdr_next(rdr);
    }

    // TODO: arbitrary radix literals (16r, 8r, 2r, etc)
    // TODO: float literals (NOTE: always has a decimal digit, so 1. =/= 1.0)

    auto obj = ob_create_integer(rdr->context, num);
    (void)fnum;

    push_literal(rdr, obj);
    return true;
  }

  switch (*rdr->head) {
  case '\'':
    p_string(rdr);
    break;
  case '#':
    p_symbol(rdr);
    break;
  case '(':
    p_paren(rdr);
    break;
  case '{':
    p_method(rdr);
    break;
  case '[':
    p_array(rdr);
    break;
  default:
    return false;
  }

  return true;
}

static void emit_send(Reader *rdr, size_t index, bool explicitp) {
  index = obbc_append_index(&rdr->output->bytecode, index);

  obbc_append_insn(
      &rdr->output->bytecode,
      OBBC_MAKE((explicitp ? OBBC_SEND : OBBC_IMPLICIT_SEND), index));
}

// primary . {unary-message}
// if we found a keyword instead, backtrack
static bool p_unary_send(Reader *rdr, bool explicitp) {
  while (true) {
    p_skip_blank(rdr);

    if (is_word_start(*rdr->head)) {
      auto here = rdr->head;
      auto rem = rdr->remaining;

      while (is_word_tail(*rdr->head)) {
        rdr_next(rdr);
      }

      if (*rdr->head == ':') {
        rdr->head = here;
        rdr->remaining = rem;
        return explicitp;
      }

      auto sel = obstr_create(rdr->context, (rdr->head - here), here);
      auto objsel = ob_create_symbol(rdr->context, sel);

      auto index = obarr_length(&rdr->output->literals, sizeof(ob_Obj));
      obarr_push(&rdr->output->literals, sizeof(ob_Obj), (const void *)&objsel);

      emit_send(rdr, index, explicitp);

      // recv of next message is already in stack
      explicitp = true;
      continue;
    }

    break;
  }

  return explicitp;
}

// primary [unary-send]
static bool p_unary(Reader *rdr) {
  auto explicitp = p_primary(rdr);
  return p_unary_send(rdr, explicitp);
}

// unary-send . { operator unary-send }
static bool p_binary_send(Reader *rdr, bool explicitp) {
  while (true) {
    p_skip_blank(rdr);
    auto here = rdr->head;

    if (is_operator(*rdr->head)) {
      while (is_operator(*rdr->head)) {
        rdr_next(rdr);
      }

      p_unary(rdr);

      auto sel = obstr_create(rdr->context, (rdr->head - here), here);
      auto objsel = ob_create_symbol(rdr->context, sel);

      auto index = obarr_length(&rdr->output->literals, sizeof(ob_Obj));
      obarr_push(&rdr->output->literals, sizeof(ob_Obj), (const void *)&objsel);

      emit_send(rdr, index, explicitp);

      explicitp = true;
      continue;
    }

    break;
  }

  return explicitp;
}

static bool p_binary(Reader *rdr) {
  auto explicitp = p_unary(rdr);
  return p_binary_send(rdr, explicitp);
}

static bool p_keyword_send(Reader *rdr, bool explicitp) {
  auto message = (ob_Array){};
  obarr_init(&message, rdr->context->allocator);

  while (true) {
    p_skip_blank(rdr);

    // we know that if this is not a word it cannot be a kw
    if (is_word_start(*rdr->head)) {
      auto here = rdr->head;

      while (is_word_tail(*rdr->head)) {
        rdr_next(rdr);
      }

      // this char can only be :
      ASSERT(*rdr->head == ':', "expected a `:`, got a %c", *rdr->head);
      rdr_next(rdr);

      obarr_push(&message, (rdr->head - here), here);

      (void)p_binary(rdr);

      continue;
    }

    break;
  }

  if (message.size != 0) {
    auto sel = obstr_create(rdr->context, message.size, message.data);
    auto objsel = ob_create_symbol(rdr->context, sel);

    auto index = obarr_length(&rdr->output->literals, sizeof(ob_Obj));
    obarr_push(&rdr->output->literals, sizeof(ob_Obj), (const void *)&objsel);

    emit_send(rdr, index, explicitp);
  }

  obarr_free(&message);
  return explicitp;
}

// binary . { keyword binary }
static bool p_keyword(Reader *rdr) {
  auto explicitp = p_binary(rdr);
  return p_keyword_send(rdr, explicitp);
}

static void p_expression(Reader *rdr) {
  p_skip_blank(rdr);

  if (*rdr->head == '^') {
    rdr_next(rdr);
    p_expression(rdr);

    obbc_append_insn(&rdr->output->bytecode, OBBC_RETURN);
    return;
  }

  (void)p_keyword(rdr);
}

static void p_toplevel(Reader *rdr) {
  p_expression(rdr);
  p_skip_blank(rdr);

  ASSERT(*rdr->head == '.', "expected a `.`, got a `%s`", rdr->head);
  rdr_next(rdr);
}

ob_Obj ob_load(ob_Context ctx, size_t length, const char *text) {
  if (strncmp(text, OB_SERIAL_HEADER, sizeof(OB_SERIAL_HEADER)) == 0) {
    auto srl = (ob_Serial){};
    obsrl_init(&srl, ctx);

    obsrl_load(&srl, length, (const uint8_t *)text);
    auto obj = obsrl_read(&srl);

    obsrl_free(&srl);

    return obj;
  }

  ob_Obj closure = ob_create_method(ctx);
  auto clos = ob_cast_method(closure);

  auto reader = rdr_new(ctx, clos, length, text);

  while (reader.remaining > 0) {
    p_skip_blank(&reader);

    if (reader.remaining == 0) {
      break;
    }

    auto previous = reader.head;
    p_toplevel(&reader);

    ASSERT(reader.head != previous, "didn't read anything");
  }

  return closure;
}

void ob_run(ob_Context ctx, size_t length, const char *text) {
  auto chunk = ob_load(ctx, length, text);
  auto method = ob_cast_method(chunk);

  obctx_enter_activation(ctx, chunk, ctx->known.shell);

  obbc_run(ctx, method->bytecode.size, method->bytecode.data);

  obctx_leave_activation(ctx);
}
