#include "Parse.h"
#include "Array.h"
#include "Assert.h"
#include "Bytecode.h"
#include "Context.h"
#include "Macros.h"
#include "Object.h"
#include "Serial.h"
#include "String.h"

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
  Context context;
  ObjMethod *output;
  size_t remaining;
  const char *head;
} Reader;

Reader rdr_new(Context context, ObjMethod *output, size_t length,
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

static void push_literal(Reader *rdr, Obj obj) {
  auto index = arr_length(&rdr->output->literals, sizeof(Obj));
  arr_push(&rdr->output->literals, sizeof(Obj), (const void *)&obj);

  index = bc_append_index(&rdr->output->bytecode, index);
  bc_append_insn(&rdr->output->bytecode, INSN_MAKE(OP_PUSH_LITERAL, index));
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
    } else if (*rdr->head == '"') {
      rdr_next(rdr);

      while (*rdr->head != '"') {
        rdr_next(rdr);
      }

      rdr_next(rdr);
    } else {
      break;
    }
  }
}

static void p_paren(Reader *rdr) {
  rdr_next(rdr);

  p_skip_blank(rdr);

  if (*rdr->head == ')') {
    // () is the nil literal

    Obj nil = NULL;
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

  ASSERT(*rdr->head == ']', "expected a closing ], got a %c", *rdr->head);

  items = bc_append_index(&rdr->output->bytecode, items);
  bc_append_insn(&rdr->output->bytecode, INSN_MAKE(OP_ARRAY, items));

  rdr_next(rdr);
}

static void p_method(Reader *rdr) {
  rdr_next(rdr);

  p_skip_blank(rdr);

  auto original = rdr->output;

  auto new = ctx_alloc_method(rdr->context);
  ObjMethod *method = obj_get_data(new);

  rdr->output = method;

  // | {word} |
  auto buf = (Array){};
  arr_init(&buf, rdr->context->allocator);

  if (*rdr->head == '|') {
    rdr_next(rdr);

    p_skip_blank(rdr);

    while (true) {
      if (*rdr->head == '|') {
        rdr_next(rdr);
        break;
      }

      arr_clear(&buf);

      auto begin = rdr->head;
      rdr_takewhile(rdr, is_word_tail);

      arr_push(&buf, sizeof(char) * (rdr->head - begin), begin);

      // TODO: also intern this
      auto str = str_create(rdr->context, buf.size, buf.data);
      arr_push(&rdr->output->parameters, sizeof(Str), (void *)&str);

      p_skip_blank(rdr);

      if (!((*rdr->head == '|') || isalpha(*rdr->head))) {
        ASSERT(false, "unexpected character in argument list: `%c`",
               *rdr->head);
      }
    }
  }

  arr_free(&buf);

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

  ASSERT(*rdr->head == '}', "expected a closing }, got a %c", *rdr->head);

  rdr_next(rdr);
}

static Str p_string_inner(Reader *rdr) {
  rdr_next(rdr);

  auto buf = (Array){};
  arr_init(&buf, rdr->context->allocator);

  auto begin = rdr->head;

  while (*rdr->head != '\'') {
    if (*rdr->head == '\\') {
      arr_push(&buf, sizeof(char) * (rdr->head - begin - 1), begin - 1);

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

      arr_push(&buf, sizeof(char), &escaped);
      rdr_next(rdr);

      begin = rdr->head;
      continue;
    }

    rdr_next(rdr);
  }

  arr_push(&buf, sizeof(char) * (rdr->head - begin), begin);

  rdr_next(rdr);

  auto str = str_create(rdr->context, buf.size, buf.data);
  arr_free(&buf);

  return str;
}

static void p_string(Reader *rdr) {
  auto str = p_string_inner(rdr);
  auto obj = ctx_alloc_string(rdr->context, str);

  push_literal(rdr, obj);
}

static void p_symbol(Reader *rdr) {
  rdr_next(rdr);

  Str sel = NULL;
  auto sym = (Array){};
  arr_init(&sym, rdr->context->allocator);

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

  arr_push(&sym, sizeof(char) * (rdr->head - begin), begin);

  sel = str_create(rdr->context, sym.size, sym.data);

after_str:
  auto objsel = ctx_alloc_symbol(rdr->context, sel);

  push_literal(rdr, objsel);
  arr_free(&sym);
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

    auto obj = ctx_alloc_integer(rdr->context, num);
    IGNORE fnum;

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
  case '@':
    rdr_next(rdr);
    bc_append_insn(&rdr->output->bytecode, OP_SELF);
    break;
  default:
    return false;
  }

  return true;
}

static void emit_send(Reader *rdr, int index, bool explicitp) {
  bc_append_insn(&rdr->output->bytecode,
                 INSN_MAKE((explicitp ? OP_SEND : OP_IMPLICIT_SEND), index));
}

static int p_message(Reader *rdr, bool explicitp) {
  auto msg = (Array){};
  arr_init(&msg, rdr->context->allocator);

  while (true) {
    arr_clear(&msg);
    p_skip_blank(rdr);

    if (is_word_start(*rdr->head)) {
      auto is_keyword = false;
      // if we have an alphanumeric, it may be either a unary or keyword
      // message.

      auto begin = rdr->head;
      rdr_takewhile(rdr, is_word_tail);

      arr_push(&msg, sizeof(char) * (rdr->head - begin), begin);

      // if this is a : then
      // keyword:   word { : word }
      while (*rdr->head == ':') {
        is_keyword = true;
        arr_push(&msg, sizeof(char), rdr->head);
        rdr_next(rdr);

        p_expression(rdr);

        p_skip_blank(rdr);

        begin = rdr->head;
        rdr_takewhile(rdr, is_word_tail);

        arr_push(&msg, sizeof(char) * (rdr->head - begin), begin);
      }

      // TODO: actually intern this
      auto sel = str_create(rdr->context, msg.size, msg.data);
      auto objsel = ctx_alloc_string(rdr->context, sel);

      auto index = arr_length(&rdr->output->literals, sizeof(Obj));
      arr_push(&rdr->output->literals, sizeof(Obj), (const void *)&objsel);

      if (is_keyword) {
        arr_free(&msg);
        return bc_append_index(&rdr->output->bytecode, index);
      }

      index = bc_append_index(&rdr->output->bytecode, index);
      emit_send(rdr, index, explicitp);
      explicitp = false;
      continue;
    }

    if (is_operator(*rdr->head)) {
      // if ... punctuation, it is a binary message

      auto begin = rdr->head;
      rdr_takewhile(rdr, is_operator);

      auto sel = str_create(rdr->context, rdr->head - begin, begin);
      auto objsel = ctx_alloc_string(rdr->context, sel);

      p_expression(rdr);

      auto index = arr_length(&rdr->output->literals, sizeof(Obj));
      arr_push(&rdr->output->literals, sizeof(Obj), (const void *)&objsel);

      index = bc_append_index(&rdr->output->bytecode, index);
      emit_send(rdr, index, explicitp);
      explicitp = false;
      continue;
    }

    break;
  }

  // else, there was no message.
  arr_free(&msg);
  return -1;
}

static void p_expression(Reader *rdr) {
  p_skip_blank(rdr);

  if (*rdr->head == '^') {
    rdr_next(rdr);
    p_expression(rdr);

    bc_append_insn(&rdr->output->bytecode, OP_RETURN);
    return;
  }

  auto explicit_receiver = p_primary(rdr);
  auto msg_index = p_message(rdr, explicit_receiver);

  if (msg_index == -1) {
    return;
  }

  if (explicit_receiver) {
    bc_append_insn(&rdr->output->bytecode, INSN_MAKE(OP_SEND, msg_index));
  } else {
    bc_append_insn(&rdr->output->bytecode,
                   INSN_MAKE(OP_IMPLICIT_SEND, msg_index));
  }
}

static void p_toplevel(Reader *rdr) {
  p_expression(rdr);
  p_skip_blank(rdr);

  ASSERT(*rdr->head == '.', "expected a `.`, got a `%c`", *rdr->head);
  rdr_next(rdr);
}

Obj load_file(Context ctx, size_t length, const char *text) {
  if (strncmp(text, SERIAL_HEADER, sizeof(SERIAL_HEADER)) == 0) {
    auto srl = (Serial){};
    srl_init(&srl, ctx);

    srl_load(&srl, length, (const uint8_t *)text);
    auto obj = srl_read(&srl);

    srl_free(&srl);

    return obj;
  }

  Obj closure = ctx_alloc_method(ctx);
  ObjMethod *clos = obj_get_data(closure);

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

void run_file(Context ctx, size_t length, const char *text) {
  auto chunk = load_file(ctx, length, text);
  ObjMethod *method = obj_get_data(chunk);

  ctx_enter_activation(ctx, NULL, chunk, ctx->shell);

  bc_run(ctx, method->bytecode.size, method->bytecode.data);

  ctx_leave_activation(ctx);
}
