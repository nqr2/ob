#include "ob/Core.h"
#include <ob/core/Bytecode.h>
#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>

#define QL_LOG_MODULE "Bytecode"
#include <ql/Array.h>
#include <ql/Log.h>

#include <ctype.h>

static size_t nargs_for_sel(ob_Ctx ctx, ob_Str selector) {
  size_t n_args = 0;

  auto sel = obstr_get_data(ctx, selector);

  if (ispunct(sel[0])) {
    n_args = 1;
  } else {
    for (size_t i = 0; i < selector->length; i++) {
      if (sel[i] == ':') {
        n_args++;
      }
    }
  }

  return n_args;
}

static const uint8_t *read_int(const uint8_t *bytes, uint64_t *result) {
  auto shift = 0;
  auto byte = *bytes;

  *result = 0;

  do {
    byte = *bytes;
    *result |= (byte & 0x7f) << (shift * 7);
    shift++;
    bytes++;
  } while ((byte & 0x80) != 0);

  return bytes;
}

void obbc_run(ob_Ctx ctx, size_t len, const uint8_t *code) {
  QL_DEBUG("running data from %p, length %zu", code, len);

  auto here = ob_cast_activation(ctx->this_activation);

  auto start = code;

  while ((size_t)(code - start) < len) {
    ob_Opcode opcode = 0;
    size_t data = 0;

    auto byte = *code;
    code = obbc_read_insn(code, &opcode, &data);

    auto act = ob_cast_activation(ctx->this_activation);
    auto method = ob_cast_method(act->method);

    QL_DEBUG("offset %ld: %02x with data %zu", (code - start), byte, data);

    switch (opcode) {
    case OB_OP_PUSH: {
      auto literal =
          *(ob_Obj *)ql_array_at(&method->literals, sizeof(ob_Obj), data);

      ob_push(ctx, literal);
    }; break;

    case OB_OP_SEND: {
      auto literal =
          *(ob_Obj *)ql_array_at(&method->literals, sizeof(ob_Obj), data);

      auto selector = *ob_cast_symbol(literal);

      QL_DEBUG("send: #'%.*s'", obstr_get_length(selector),
               obstr_get_data(ctx, selector));

      auto nargs = nargs_for_sel(ctx, selector);
      auto stack_len = ql_array_length(&ctx->stack, sizeof(ob_Obj));
      auto recv_index = stack_len - nargs - 1;

      ob_Obj recv =
          *(ob_Obj *)ql_array_at(&ctx->stack, sizeof(ob_Obj), recv_index);

      ql_array_remove(&ctx->stack, sizeof(ob_Obj), recv_index);

      ob_send(ctx, recv, selector);
    }; break;

    case OB_OP_IMPLICIT: {
      auto literal =
          *(ob_Obj *)ql_array_at(&method->literals, sizeof(ob_Obj), data);

      auto selector = *ob_cast_symbol(literal);

      ob_send(ctx, ctx->this_activation, selector);
    }; break;

    case OB_OP_EXTEND:
      // already handled by read_insn
      break;

    case OB_OP_DEBUG:
      if (data == 0) {
        size_t col_delta = 0;
        code = read_int(code, &col_delta);

        here->column += col_delta;
      } else {
        size_t col_delta = 0;
        size_t line_delta = 0;

        code = read_int(code, &col_delta);
        code = read_int(code, &line_delta);

        here->this_line = (const char *)code;
        here->line += line_delta;
        here->column = col_delta;

        code += data;
      }
      break;

    case OB_OP_FILENAME:
      here->path = (const char *)code;
      here->line = 0;
      here->column = 0;

      code += data;
      break;

      // TODO: OP_RETURN

    case OB_OP_ARRAY: {
      auto obj = ob_create_array(ctx);
      auto arr = ob_cast_array(obj);

      ql_array_reserve(arr, data * sizeof(ob_Obj));

      ql_array_pop(&ctx->stack, data * sizeof(ob_Obj), arr->data);
      arr->size = data * sizeof(ob_Obj);

      ob_push(ctx, obj);
    }; break;

    default:
      break;
    }

    ob_gc(ctx, false);
  }
}

void obbc_append_insn(ql_Array *out, ob_Instruction insn) {
  ql_array_push(out, sizeof(ob_Instruction), &insn);
}

uint8_t obbc_append_index(ql_Array *out, uint64_t index) {
  while (index > 15) {
    obbc_append_insn(out, OBBC_MAKE(OB_OP_EXTEND, index & 0xf));
    index >>= 4;
  }

  return index;
}

const uint8_t *obbc_read_insn(const uint8_t *source, ob_Opcode *opcode,
                              size_t *data) {
  size_t this_index = 0;
  size_t shift = 0;
  ob_Opcode code = 0;

  while (true) {
    ob_Instruction insn = *source;

    size_t payload = OBBC_GET_DATA(insn);
    code = OBBC_GET_OPCODE(insn);

    this_index |= (payload << 4 * shift);

    source++;
    shift++;

    if (code == OB_OP_EXTEND) {
      // this_index <<= 4;
      continue;
    }

    break;
  }

  if (opcode != NULL) {
    *opcode = code;
  }

  if (data != NULL) {
    *data = this_index;
  }

  return source;
}
