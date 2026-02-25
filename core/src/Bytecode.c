#include "ob/Core.h"
#define OB_LOG_MODULE "Bytecode"

#include <ob/base/Array.h>
#include <ob/base/Log.h>
#include <ob/core/Bytecode.h>
#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>

#include <ctype.h>
#include <stdbit.h>

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

static uint8_t const *read_int(uint8_t const *bytes, uint64_t *result) {
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

char const *obbc_opcode_name(int operation) {
  switch (operation) {
  case OB_OP_PUSH:
    return "PUSH";
  case OB_OP_SEND:
    return "SEND";
  case OB_OP_IMPLICIT:
    return "IMPLICIT";
  case OB_OP_EXTEND:
    return "EXTEND";
  case OB_OP_EXTRA:
    return "EXTRA";
  case OB_OP_ARRAY:
    return "ARRAY";
  case OB_OP_DEBUG:
    return "DEBUG";
  case OB_OP_FILENAME:
    return "FILENAME";

  default:
    return "???";
  }
}

char const *obbc_extopcode_name(int ext) {
  switch (ext) {
  case OB_OP_EXT_RETURN:
    return "RETURN";
  case OB_OP_EXT_DUPLICATE:
    return "DUPLICATE";
  case OB_OP_EXT_POP:
    return "POP";
  default:
    return "???";
  }
}

void obbc_run(ob_Ctx ctx, size_t len, uint8_t const *code) {
  QL_DEBUG("running data from %p, length %zu", code, len);

  auto here = ob_cast_activation(ctx->this_activation);

  auto start = code;

  int size = stdc_bit_width(len) / 4 + 1;

  while ((size_t)(code - start) < len) {
    ob_Opcode opcode = 0;
    size_t data = 0;

    auto byte = *code;
    auto offset = code - start;
    code = obbc_read_insn(code, &opcode, &data);

    auto act = ob_cast_activation(ctx->this_activation);
    auto method = ob_cast_method(act->method);

    QL_DEBUG("offset %.*x: %02x with data %zu", size, offset, byte, data);

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

        here->this_line = (char const *)code;
        here->line += line_delta;
        here->column = col_delta;

        code += data;
      }
      break;

    case OB_OP_FILENAME:
      here->path = (char const *)code;
      here->line = 0;
      here->column = 0;

      code += data;
      break;

    case OB_OP_EXTRA: {
      switch (data) {
      case OB_OP_EXT_RETURN:
        break; // TODO: OP_RETURN
      case OB_OP_EXT_DUPLICATE: {
        auto top = ob_pop(ctx);
        ob_push(ctx, top);
        ob_push(ctx, top);
      } break;
      case OB_OP_EXT_POP:
        (void)ob_pop(ctx);
        break;
      default:
        break;
      }
    }; break;

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
  while (index > 31) {
    obbc_append_insn(out, OBBC_MAKE(OB_OP_EXTEND, index & 0x1f));
    index >>= 5;
  }

  return index;
}

uint8_t const *obbc_read_insn(uint8_t const *source, ob_Opcode *opcode,
                              size_t *data) {
  size_t this_index = 0;
  size_t shift = 0;
  ob_Opcode code = 0;

  while (true) {
    ob_Instruction insn = *source;

    size_t payload = OBBC_GET_DATA(insn);
    code = OBBC_GET_OPCODE(insn);

    this_index |= (payload << 5 * shift);

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
