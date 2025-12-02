#include "Parse.h"
#include "Array.h"
#include "Assert.h"
#include "Bytecode.h"
#include "Context.h"
#include "Macros.h"
#include "Object.h"
#include "String.h"

#include <ctype.h>

typedef struct {
  size_t remaining;
  const char *head;
} Reader;

Reader rdr_new(size_t length, const char *text) {
  Reader rdr = {};
  rdr.remaining = length;
  rdr.head = text;
  return rdr;
}

void rdr_next(Reader *rdr) {
  rdr->head++;
  rdr->remaining--;
}

static void p_expression(Context ctx, ObjMethod *output, Reader *rdr);

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

static void p_parens(Context ctx, ObjMethod *output, Reader *rdr) {
  rdr_next(rdr);

  p_skip_blank(rdr);

  if (*rdr->head == ')') {
    // () is the nil literal

    Obj nil = NULL;
    auto index = arr_length(&output->literals, sizeof(Obj));
    arr_push(&output->literals, sizeof(Obj), (const void *)&nil);

    index = bc_append_index(&output->bytecode, index);
    bc_append_insn(&output->bytecode, INSN_MAKE(OP_PUSH_LITERAL, index));
  } else {
    // expression { . expression }
    p_expression(ctx, output, rdr);
    p_skip_blank(rdr);

    while (*rdr->head == '.') {
      rdr_next(rdr);
      p_expression(ctx, output, rdr);
      p_skip_blank(rdr);
    }
  }

  ASSERT(*rdr->head == ')', "expected a closing ), got a %c", *rdr->head);

  rdr_next(rdr);
}

static void p_receiver(Context ctx, ObjMethod *output, Reader *rdr,
                       bool *explicitp) {
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

  *explicitp = true;

  switch (*rdr->head) {
  case '0' ... '9': {
    int64_t num = 0;
    double fnum = 0;

    while (isdigit(*rdr->head)) {
      auto digit = *rdr->head - '0';
      num = (num * 10) + digit;
      rdr_next(rdr);
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
    p_parens(ctx, output, rdr);
    // parse paren expr
    break;
  default:
    *explicitp = false;
  }
}

static bool isop(char chr) {
  if (!ispunct(chr)) {
    return false;
  }

  switch (chr) {
  case '(':
  case ')':
  case '.':
    return false;
  default:
    return true;
  }
}

static void p_message(Context ctx, ObjMethod *output, Reader *rdr,
                      bool explicitp) {
  p_skip_blank(rdr);

  if (isalpha(*rdr->head)) {
    // if we have an alphanumeric, it may be either a unary or keyword message.

    Array msg = {};
    arr_init(&msg, ctx->allocator);

    auto begin = rdr->head;
    while (isalpha(*rdr->head)) {
      rdr_next(rdr);
    }

    arr_push(&msg, sizeof(char) * (rdr->head - begin), begin);

    // if this is a : then
    // keyword:   word { : word }
    while (*rdr->head == ':') {
      arr_push(&msg, sizeof(char), rdr->head);
      rdr_next(rdr);

      p_expression(ctx, output, rdr);

      p_skip_blank(rdr);

      begin = rdr->head;
      while (isalpha(*rdr->head)) {
        rdr_next(rdr);
      }
      arr_push(&msg, sizeof(char) * (rdr->head - begin), begin);
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
  } else if (isop(*rdr->head)) {
    // if ... punctuation, it is a binary message

    auto begin = rdr->head;
    while (isop(*rdr->head)) {
      rdr_next(rdr);
    }

    auto sel = str_create(ctx, rdr->head - begin, begin);
    auto objsel = ctx_alloc_string(ctx, sel);

    p_expression(ctx, output, rdr);

    auto index = arr_length(&output->literals, sizeof(Obj));
    arr_push(&output->literals, sizeof(Obj), (const void *)&objsel);

    index = bc_append_index(&output->bytecode, index);

    if (explicitp) {
      bc_append_insn(&output->bytecode, INSN_MAKE(OP_SEND, index));
    } else {
      bc_append_insn(&output->bytecode, INSN_MAKE(OP_IMPLICIT_SEND, index));
    }
  }
  // else, there was no message.
}

static void p_expression(Context ctx, ObjMethod *output, Reader *rdr) {
  auto explicitp = false;
  p_receiver(ctx, output, rdr, &explicitp);

  p_message(ctx, output, rdr, explicitp);
}

static void p_toplevel(Context ctx, ObjMethod *output, Reader *rdr) {
  p_expression(ctx, output, rdr);
  p_skip_blank(rdr);

  // assert a '.' is here
  ASSERT(*rdr->head == '.', "expected a `.`, got a `%c`", *rdr->head);
  rdr_next(rdr);
}

Obj load_file(Context ctx, size_t length, const char *text) {
  Obj closure = ctx_alloc_method(ctx);
  ObjMethod *clos = obj_get_data(closure);

  auto reader = rdr_new(length, text);

  while (reader.remaining > 0) {
    auto previous = reader.head;
    p_toplevel(ctx, clos, &reader);

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
