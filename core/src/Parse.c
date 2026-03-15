#include "ob/Core.h"
#include "ob/base/Table.h"
#define OB_LOG_MODULE "Parse"

#include <ob/base/Array.h>
#include <ob/base/Assert.h>
#include <ob/base/Log.h>
#include <ob/core/Bytecode.h>
#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/Parse.h>
#include <ob/core/Serial.h>
#include <ob/core/String.h>

#include <ctype.h>
#include <stdbit.h>
#include <stdint.h>
#include <string.h>

static bool is_operator(char chr) {
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
    return ispunct(chr);
  }
}

static bool is_word_start(char chr) {
  return isalpha(chr) || chr == '_';
}

static bool is_word_tail(char chr) {
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
  case ':':
  case '|':
    return false;
  default:
    return is_word_start(chr) || isdigit(chr) || is_operator(chr);
  }
}

struct ob_Reader {
  ob_Ctx context;
  ql_Table known;
  ob_ObjMethod *output;
  size_t remaining;
  char const *head;

  char const *path;
  size_t line;
  size_t column;
};

static void write_int(ql_Array *arr, uint64_t n) {
  auto len = stdc_bit_width(n);
  QL_ASSERT(len <= 63, "cannot encode a num > 64 bits.");

  do {
    uint8_t byte = n & 0x7f;
    n >>= 7;

    if (n != 0) {
      byte |= 0x80;
    }

    ql_array_push(arr, sizeof(uint8_t), &byte);
  } while (n != 0);
}

void rdr_emit_debug_file(ob_Rdr rdr) {
  auto len = strlen(rdr->path) + 1;
  auto tail = obbc_append_index(&rdr->output->bytecode, len);
  obbc_append_insn(&rdr->output->bytecode, OBBC_MAKE(OB_OP_FILENAME, tail));

  ql_array_push(&rdr->output->bytecode, len - 1, rdr->path);
  ql_array_push(&rdr->output->bytecode, 1, (char[]){0});
}

void rdr_emit_debug_column(ob_Rdr rdr, size_t cdelta) {
  if (cdelta == 1) {
    obbc_append_insn(&rdr->output->bytecode,
                     OBBC_MAKE(OB_OP_EXTRA, OB_OP_EXT_DEBUG_CD1));
  } else if (cdelta == 2) {
    obbc_append_insn(&rdr->output->bytecode,
                     OBBC_MAKE(OB_OP_EXTRA, OB_OP_EXT_DEBUG_CD2));
  } else {
    obbc_append_insn(&rdr->output->bytecode, OBBC_MAKE(OB_OP_DEBUG, 0));

    write_int(&rdr->output->bytecode, cdelta);
  }
}

void rdr_emit_debug_line(ob_Rdr rdr, size_t column, size_t ldelta) {
  auto eol = memchr(rdr->head, '\n', rdr->remaining);
  auto len = (size_t)((char const *)eol - rdr->head);

  if (eol == NULL) {
    len = strlen(rdr->head);
  }

  len++;

  auto tail = obbc_append_index(&rdr->output->bytecode, len);
  obbc_append_insn(&rdr->output->bytecode, OBBC_MAKE(OB_OP_DEBUG, tail));

  write_int(&rdr->output->bytecode, column);
  write_int(&rdr->output->bytecode, ldelta);

  ql_array_push(&rdr->output->bytecode, len - 1, rdr->head);
  ql_array_push(&rdr->output->bytecode, 1, (char[]){0});
}

void rdr_next(ob_Rdr rdr) {
  QL_ASSERT(rdr->remaining > 0, "unexpected end of file");
  rdr->remaining--;
  rdr->head++;

  rdr->column++;

  if (*(rdr->head - 1) == '\n') {
    rdr->line++;
    rdr->column = 0;

    rdr_emit_debug_line(rdr, 0, 1);
  }
}

static void rdr_expect1(ob_Rdr rdr, char chr) {
  if (rdr->remaining == 0) {
    QL_ASSERT(*rdr->head == chr, "at '%s' %zu:%zu: expected `%c`, got EOF",
              rdr->path, rdr->line + 1, rdr->column + 1, chr, *rdr->head);
  }

  QL_ASSERT(*rdr->head == chr, "at '%s' %zu:%zu: expected `%c`, got `%c`",
            rdr->path, rdr->line + 1, rdr->column + 1, chr, *rdr->head);

  rdr_next(rdr);
}

static void rdr_expectn(ob_Rdr rdr, char const *chrs) {
  while (*chrs != 0) {
    if (*rdr->head == *chrs) {
      rdr_next(rdr);
      return;
    }

    chrs++;
  }

  QL_ASSERT(false, "at '%s' %zu:%zu: expected one of [%s], got `%c`", rdr->path,
            rdr->line + 1, rdr->column + 1, chrs, *rdr->head);
}

// i wish C had lambdas...
void rdr_takewhile(ob_Rdr rdr, bool (*pred)(char)) {
  while (pred(*rdr->head)) {
    rdr_next(rdr);
  }
}

static void push_literal(ob_Rdr rdr, ob_Obj obj) {
  size_t idx = 0;

  if (ql_table_get(&rdr->known, (uint64_t)obj, (void **)&idx)) {
    idx = obbc_append_index(&rdr->output->bytecode, idx);
    obbc_append_insn(&rdr->output->bytecode, OBBC_MAKE(OB_OP_PUSH, idx));
    return;
  }

  idx = ql_array_length(&rdr->output->literals, sizeof(ob_Obj));
  ql_array_push(&rdr->output->literals, sizeof(ob_Obj), (void const *)&obj);

  ql_table_set(&rdr->known, (uint64_t)obj, (void *)idx);

  QL_DEBUG("push literal: %zu", idx);

  idx = obbc_append_index(&rdr->output->bytecode, idx);
  obbc_append_insn(&rdr->output->bytecode, OBBC_MAKE(OB_OP_PUSH, idx));
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

static void p_expression(ob_Rdr rdr);

static void p_skip_blank(ob_Rdr rdr) {
  auto here = rdr->head;

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

  if (here != rdr->head) {
    rdr_emit_debug_column(rdr, rdr->head - here);
  }
}

static void p_paren(ob_Rdr rdr) {
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

  rdr_expect1(rdr, ')');
}

static void p_array(ob_Rdr rdr) {
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

  rdr_expect1(rdr, ']');

  items = obbc_append_index(&rdr->output->bytecode, items);
  obbc_append_insn(&rdr->output->bytecode, OBBC_MAKE(OB_OP_ARRAY, items));
}

static void p_method(ob_Rdr rdr) {
  auto original = rdr->output;

  auto old_known = rdr->known;

  auto new = ob_create_method(rdr->context);
  auto method = ob_cast_method(new);
  ql_table_init(&rdr->known, rdr->context->allocator);

  // TODO: Handle self-referential objects in obj_visit, then uncomment this.
  // method->parent = obrdr_get_method(rdr);
  rdr->output = method;

  // also record debug info here
  rdr_emit_debug_file(rdr);
  rdr_emit_debug_line(rdr, rdr->column, rdr->line);

  // we jump AFTER recording debug info to capture the whole closure's source
  rdr_next(rdr);

  p_skip_blank(rdr);

  QL_DEBUG("head here is: %c", *rdr->head);
  // | {word} |
  if (*rdr->head == '|') {
    rdr_next(rdr);

    p_skip_blank(rdr);

    while (true) {
      if (*rdr->head == '|') {
        rdr_next(rdr);
        break;
      }

      auto begin = rdr->head;
      rdr_takewhile(rdr, is_word_tail);

      // TODO: also intern this
      auto str = obstr_create(rdr->context, (rdr->head - begin), begin);

      QL_DEBUG("add parameter: '%.*s'", (rdr->head - begin), begin);

      ql_array_push(&rdr->output->parameters, sizeof(ob_Str), (void *)&str);

      QL_DEBUG("post push: %zu", rdr->output->parameters.size);

      p_skip_blank(rdr);

      if (!((*rdr->head == '|') || isalpha(*rdr->head))) {
        QL_ASSERT(false, "unexpected character in argument list: `%c`",
                  *rdr->head);
      }
    }
  }

  // expression { . expression }
  p_expression(rdr);
  p_skip_blank(rdr);

  while (*rdr->head == '.') {
    rdr_next(rdr);
    p_expression(rdr);
    p_skip_blank(rdr);
  }

  ql_table_free(&rdr->known);

  rdr->output = original;
  rdr->known = old_known;

  push_literal(rdr, new);

  p_skip_blank(rdr);
  p_skip_blank(rdr);

  rdr_expect1(rdr, '}');
}

static ob_Str p_string_inner(ob_Rdr rdr) {
  rdr_next(rdr);

  auto buf = (ql_Array){};
  ql_array_init(&buf, rdr->context->allocator);

  auto begin = rdr->head;

  while (*rdr->head != '\'') {
    if (*rdr->head == '\\') {
      ql_array_push(&buf, sizeof(char) * (rdr->head - begin - 1), begin - 1);

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

      ql_array_push(&buf, sizeof(char), &escaped);
      rdr_next(rdr);

      begin = rdr->head;
      continue;
    }

    rdr_next(rdr);
  }

  QL_DEBUG("string: '%.*s'", (rdr->head - begin), begin);
  ql_array_push(&buf, sizeof(char) * (rdr->head - begin), begin);

  rdr_expect1(rdr, '\'');

  auto str = obstr_create(rdr->context, buf.size, buf.data);
  ql_array_free(&buf);

  return str;
}

static void p_string(ob_Rdr rdr) {
  auto str = p_string_inner(rdr);
  auto obj = ob_wrap_string(rdr->context, str);

  push_literal(rdr, obj);
}

static void p_symbol(ob_Rdr rdr) {
  rdr_next(rdr);

  if (*rdr->head == '\'') {
    auto sel = p_string_inner(rdr);
    auto objsel = ob_wrap_string(rdr->context, sel);
    push_literal(rdr, objsel);
    return;
  }

  auto begin = rdr->head;

  if (is_word_start(*rdr->head)) {
    while (true) {
      rdr_takewhile(rdr, is_word_tail);

      if (*rdr->head != ':') {
        break;
      }

      rdr_next(rdr);
    }
  }

  auto sym = (ql_Array){};
  ql_array_init(&sym, rdr->context->allocator);

  QL_DEBUG("symbol: '%.*s'", (rdr->head - begin), begin);
  ql_array_push(&sym, sizeof(char) * (rdr->head - begin), begin);

  auto objsel = ob_create_string(rdr->context, sym.size, sym.data);

  push_literal(rdr, objsel);
  ql_array_free(&sym);
}

static bool p_primary(ob_Rdr rdr) {
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

static void emit_send(ob_Rdr rdr, size_t index, bool explicitp) {
  index = obbc_append_index(&rdr->output->bytecode, index);

  obbc_append_insn(&rdr->output->bytecode,
                   OBBC_MAKE((explicitp ? OB_OP_SEND : OB_OP_IMPLICIT), index));
}

static void emit_send2(ob_Rdr rdr, ob_Obj sel, bool explicitp) {
  size_t index = 0;

  if (!ql_table_get(&rdr->known, (uint64_t)sel, (void **)&index)) {
    index = ql_array_length(&rdr->output->literals, sizeof(ob_Obj));
    ql_array_push(&rdr->output->literals, sizeof(ob_Obj), (void const *)&sel);
    ql_table_set(&rdr->known, (uint64_t)sel, (void *)index);
  }

  emit_send(rdr, index, explicitp);
}

// primary . {unary-message}
// if we found a keyword instead, backtrack
static bool p_unary_send(ob_Rdr rdr, bool explicitp) {
  while (true) {
    p_skip_blank(rdr);

    if (is_word_start(*rdr->head)) {
      auto here = rdr->head;
      auto rem = rdr->remaining;
      auto line = rdr->line;
      auto col = rdr->column;

      while (is_word_tail(*rdr->head)) {
        rdr_next(rdr);
      }

      if (*rdr->head == ':') {
        rdr->head = here;
        rdr->remaining = rem;
        rdr->line = line;
        rdr->column = col;
        return explicitp;
      }

      auto objsel = ob_create_string(rdr->context, (rdr->head - here), here);

      emit_send2(rdr, objsel, explicitp);

      // recv of next message is already in stack
      explicitp = true;
      continue;
    }

    break;
  }

  return explicitp;
}

// primary [unary-send]
static bool p_unary(ob_Rdr rdr) {
  auto explicitp = p_primary(rdr);
  return p_unary_send(rdr, explicitp);
}

// unary-send . { operator unary-send }
static bool p_binary_send(ob_Rdr rdr, bool explicitp) {
  while (true) {
    p_skip_blank(rdr);
    auto here = rdr->head;

    if (is_operator(*rdr->head)) {
      while (is_operator(*rdr->head)) {
        rdr_next(rdr);
      }

      auto sel = ob_create_string(rdr->context, (rdr->head - here), here);

      p_unary(rdr);

      emit_send2(rdr, sel, explicitp);

      explicitp = true;
      continue;
    }

    break;
  }

  return explicitp;
}

static bool p_binary(ob_Rdr rdr) {
  auto explicitp = p_unary(rdr);
  return p_binary_send(rdr, explicitp);
}

static bool p_keyword_send(ob_Rdr rdr, bool explicitp) {
  auto message = (ql_Array){};
  ql_array_init(&message, rdr->context->allocator);

  while (true) {
    p_skip_blank(rdr);

    // we know that if this is not a word it cannot be a kw
    if (is_word_start(*rdr->head)) {
      auto here = rdr->head;

      while (is_word_tail(*rdr->head)) {
        rdr_next(rdr);
      }

      // this char can only be :
      rdr_expect1(rdr, ':');

      ql_array_push(&message, (rdr->head - here), here);

      (void)p_binary(rdr);

      continue;
    }

    break;
  }

  if (message.size != 0) {
    auto sel = ob_create_string(rdr->context, message.size, message.data);
    emit_send2(rdr, sel, explicitp);
  }

  ql_array_free(&message);
  return explicitp;
}

// binary . { keyword binary }
static bool p_keyword(ob_Rdr rdr) {
  auto explicitp = p_binary(rdr);
  return p_keyword_send(rdr, explicitp);
}

static void p_expression(ob_Rdr rdr) {
  p_skip_blank(rdr);

  if (*rdr->head == '^') {
    rdr_next(rdr);
    p_expression(rdr);

    obbc_append_insn(&rdr->output->bytecode,
                     OBBC_MAKE(OB_OP_EXTRA, OB_OP_EXT_RETURN));
    return;
  }

  (void)p_keyword(rdr);
}

static void p_toplevel(ob_Rdr rdr) {
  p_expression(rdr);
  p_skip_blank(rdr);

  rdr_expect1(rdr, '.');
}

ob_Obj ob_load_ext(ob_Ctx ctx, char const *file, size_t length,
                   char const *text) {
  if (strncmp(text, OB_SERIAL_HEADER, sizeof(OB_SERIAL_HEADER)) == 0) {
    auto srl = (ob_Serial){};
    obsrl_init(&srl, ctx);

    obsrl_load(&srl, length, (uint8_t const *)text);
    auto obj = obsrl_read(&srl);

    obsrl_free(&srl);

    return obj;
  }

  auto rdr = obrdr_create(ctx);
  obrdr_load(rdr, file, length, text);

  auto method = obrdr_get_method(rdr);

  obrdr_free(rdr);

  return method;
}

ob_Obj ob_load(ob_Ctx ctx, size_t length, char const *text) {
  return ob_load_ext(ctx, "*unknown*", length, text);
}

void ob_run_ext(ob_Ctx ctx, char const *file, size_t length, char const *text) {
  auto chunk = ob_load_ext(ctx, file, length, text);
  auto method = ob_cast_method(chunk);

  obctx_enter_activation(ctx, chunk, ctx->known.shell);

  obbc_run(ctx, method->bytecode.size, method->bytecode.data);

  obctx_leave_activation(ctx);
}

void ob_run(ob_Ctx ctx, size_t length, char const *text) {
  ob_run_ext(ctx, "*unknown*", length, text);
}

/// from Parse.h

ob_Rdr obrdr_create(ob_Ctx ctx) {
  auto rdr = (ob_Rdr)ql_allocate(ctx->allocator, sizeof(struct ob_Reader));

  rdr->context = ctx;
  rdr->output = ob_get_payload(ob_create_method(ctx));
  ql_table_init(&rdr->known, ctx->allocator);

  return rdr;
}

void obrdr_free(ob_Rdr rdr) {
  ql_table_free(&rdr->known);

  ql_deallocate(rdr->context->allocator, sizeof(struct ob_Reader), rdr);

  // rdr->output is taken by the GC
  // rdr->head and rdr->path are not managed by this
}

void obrdr_load(ob_Rdr rdr, char const *path, size_t length, char const *data) {
  rdr->path = path;
  rdr->remaining = length;
  rdr->head = data;

  rdr->line = 0;
  rdr->column = 0;

  rdr_emit_debug_file(rdr);
  rdr_emit_debug_line(rdr, 0, 0);

  while (rdr->remaining > 0) {
    p_skip_blank(rdr);

    if (rdr->remaining == 0) {
      break;
    }

    auto previous = rdr->head;
    p_toplevel(rdr);

    QL_ASSERT(rdr->head != previous, "didn't read anything");
  }
}

ob_Obj obrdr_get_method(ob_Rdr rdr) {
  auto as_bytes = (uint8_t *)rdr->output;
  return (ob_Obj)(as_bytes - sizeof(struct ob_Object));
}
