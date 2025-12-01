#include "Parse.h"
#include "Array.h"
#include "Assert.h"
#include "Bytecode.h"
#include "Context.h"
#include "Macros.h"
#include "Object.h"
#include "String.h"

#include <ctype.h>

static const char *p_expression(Context ctx, ObjMethod *output, size_t *length,
                                const char *text);

static void p_next(size_t *length, const char **text) {
  *text += 1;
  *length -= 1;
}

static const char *p_skip_blank(size_t *length, const char *text) {
  while (*length > 0) {
    if (isspace(*text)) {
      while (isspace(*text)) {
        p_next(length, &text);
      }
    } else if (*text == '"') {
      p_next(length, &text);

      while (*text != '"') {
        p_next(length, &text);
      }

      p_next(length, &text);
    } else {
      break;
    }
  }

  return text;
}

static const char *p_parens(Context ctx, ObjMethod *output, size_t *length,
                            const char *text) {
  p_next(length, &text);

  text = p_skip_blank(length, text);

  if (*text == ')') {
    // () is the nil literal

    Obj nil = NULL;
    auto index = arr_length(&output->literals, sizeof(Obj));
    arr_push(&output->literals, sizeof(Obj), (const void *)&nil);

    index = bc_append_index(&output->bytecode, index);
    bc_append_insn(&output->bytecode, INSN_MAKE(OP_PUSH_LITERAL, index));
  } else {
    // expression { . expression }
    text = p_expression(ctx, output, length, text);
    text = p_skip_blank(length, text);

    while (*text == '.') {
      p_next(length, &text);
      text = p_expression(ctx, output, length, text);
      text = p_skip_blank(length, text);
    }
  }

  ASSERT(*text == ')', "expected a closing ), got a %c", *text);

  p_next(length, &text);
  return text;
}

static const char *p_receiver(Context ctx, ObjMethod *output, size_t *length,
                              const char *text, bool *explicitp) {
  /*
   * We know that explicit receivers can be one of:
   * -  Number literals, which always start with a digit,
   * -  Parenthesized expressions,
   * -  String literals, which always start with a ',
   * or Symbol literals, which start with a #.
   *
   * Since anything else is a message, we can just check for this only.
   */

  text = p_skip_blank(length, text);

  *explicitp = true;

  switch (*text) {
  case '0' ... '9': {
    int64_t num = 0;
    double fnum = 0;

    while (isdigit(*text)) {
      auto digit = *text - '0';
      num = (num * 10) + digit;
      p_next(length, &text);
    }

    // TODO: arbitrary radix literals (16r, 8r, 2r, etc)
    // TODO: float literals (NOTE: always has a decimal digit, so 1. =/= 1.0)

    auto obj = ctx_alloc_integer(ctx, num);
    IGNORE obj_ref(obj);
    IGNORE fnum;

    auto index = arr_length(&output->literals, sizeof(Obj));
    arr_push(&output->literals, sizeof(Obj), (const void *)&obj);

    index = bc_append_index(&output->bytecode, index);
    bc_append_insn(&output->bytecode, INSN_MAKE(OP_PUSH_LITERAL, index));
  } break;
  case '\'':
    // parse string
    break;
  case '#':
    // parse symbol
    break;
  case '(':
    text = p_parens(ctx, output, length, text);
    // parse paren expr
    break;
  default:
    *explicitp = false;
  }

  return text;
}

static const char *p_message(Context ctx, ObjMethod *output, size_t *length,
                             const char *text, bool explicitp) {
  text = p_skip_blank(length, text);

  if (isalpha(*text)) {
    // if we have an alphanumeric, it may be either a unary or keyword message.
    // TODO: keyword messages

    Array msg = {};
    arr_init(&msg, ctx->allocator);

    auto begin = text;
    while (isalpha(*text)) {
      p_next(length, &text);
    }
    arr_push(&msg, sizeof(char) * (text - begin), begin);

    // if this is a : then
    // keyword:   word { : word }
    while (*text == ':') {
      arr_push(&msg, sizeof(char), text);
      p_next(length, &text);

      text = p_expression(ctx, output, length, text);

      text = p_skip_blank(length, text);

      begin = text;
      while (isalpha(*text)) {
        p_next(length, &text);
      }
      arr_push(&msg, sizeof(char) * (text - begin), begin);
    }

    // TODO: actually intern this
    auto sel = str_create(ctx, msg.size, msg.data);
    auto objsel = ctx_alloc_string(ctx, sel);

    auto index = arr_length(&output->literals, sizeof(Obj));
    arr_push(&output->literals, sizeof(Obj), (const void *)&objsel);

    index = bc_append_index(&output->bytecode, index);

    if (explicitp) {
      bc_append_insn(&output->bytecode, INSN_MAKE(OP_SEND, index));
    } else {
      bc_append_insn(&output->bytecode, INSN_MAKE(OP_IMPLICIT_SEND, index));
    }

    arr_free(&msg);
  } else if (ispunct(*text)) {
    // if ... punctuation, it is a binary message
    // TODO: binary messages
  }
  // else, there was no message.

  return text;
}

static const char *p_expression(Context ctx, ObjMethod *output, size_t *length,
                                const char *text) {
  auto explicitp = false;
  text = p_receiver(ctx, output, length, text, &explicitp);

  // parse the message here

  text = p_message(ctx, output, length, text, explicitp);

  return text;
}

static const char *p_toplevel(Context ctx, ObjMethod *output, size_t *length,
                              const char *text) {
  IGNORE ctx;
  IGNORE output;
  IGNORE length;
  IGNORE text;

  text = p_expression(ctx, output, length, text);
  text = p_skip_blank(length, text);

  // assert a '.' is here
  ASSERT(*text == '.', "expected a `.`, got a `%c`", *text);
  p_next(length, &text);

  return text;
}

Obj load_file(Context ctx, size_t length, const char *text) {
  Obj closure = ctx_alloc_method(ctx);
  ObjMethod *clos = obj_get_data(closure);

  while (length > 0) {
    auto next = p_toplevel(ctx, clos, &length, text);

    ASSERT(next != text, "didn't read anything");

    text = next;
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
