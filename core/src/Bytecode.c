#include <ob/Array.h>
#include <ob/Bytecode.h>
#include <ob/Context.h>
#include <ob/Object.h>

#define QL_LOG_MODULE "Bytecode"
#include <ob/Log.h>

#include <ctype.h>

static size_t nargs_for_sel(ob_Context ctx, ob_Str selector) {
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

void obbc_run(ob_Context ctx, size_t len, const uint8_t *code) {
  OB_DEBUG("running data from %p, length %zu", code, len);

  auto start = code;

  while ((size_t)(code - start) < len) {
    ob_Opcode opcode = 0;
    size_t data = 0;

    auto byte = *code;
    code = obbc_read_insn(code, &opcode, &data);

    auto act = ob_cast_activation(ctx->this_activation);
    auto method = ob_cast_method(act->method);

    ob_Obj literal =
        *(ob_Obj *)ql_array_at(&method->literals, sizeof(ob_Obj), data);

    OB_DEBUG("offset %ld: %02x with literal: %p", (code - start), byte,
             literal);

    switch (opcode) {
    case OBBC_PUSH_LITERAL: {
      auto obj = ((ob_Obj *)method->literals.data)[data];
      ob_push(ctx, obj);
      ql_array_push(&ctx->stack, sizeof(ob_Obj), (void *)&obj);
    }; break;

    case OBBC_SEND: {
      auto selector = *ob_cast_symbol(literal);

      OB_DEBUG("send: #'%.*s'", obstr_get_length(selector),
               obstr_get_data(ctx, selector));

      auto nargs = nargs_for_sel(ctx, selector);
      auto stack_len = ql_array_length(&ctx->stack, sizeof(ob_Obj));
      auto recv_index = stack_len - nargs - 1;

      ob_Obj recv =
          *(ob_Obj *)ql_array_at(&ctx->stack, sizeof(ob_Obj), recv_index);

      ql_array_remove(&ctx->stack, sizeof(ob_Obj), recv_index);

      ob_send(ctx, recv, selector);
    }; break;

    case OBBC_IMPLICIT_SEND: {
      auto selector = *ob_cast_symbol(literal);

      ob_send(ctx, ctx->this_activation, selector);
    }; break;

    // already handled by read_insn
    case OBBC_EXTEND:
      break;

    case OBBC_CASCADE: {
      auto selector = *ob_cast_symbol(literal);
      auto nargs = nargs_for_sel(ctx, selector);

      auto stack_len = ql_array_length(&ctx->stack, sizeof(ob_Obj));

      ob_Obj recv = *(ob_Obj *)ql_array_at(&ctx->stack, sizeof(ob_Obj),
                                           stack_len - nargs - 1);

      ql_array_remove(&ctx->stack, sizeof(ob_Obj), stack_len - nargs - 1);

      ob_send(ctx, recv, selector);

      (void)ob_pop(ctx);

      ob_push(ctx, recv);
    }; break;

      // TODO: OP_RETURN

    case OBBC_ARRAY: {
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

    ob_gc(ctx);
  }
}

void obbc_append_insn(ql_Array *out, ob_Instruction insn) {
  ql_array_push(out, sizeof(ob_Instruction), &insn);
}

uint8_t obbc_append_index(ql_Array *out, uint64_t index) {
  while (index > 15) {
    obbc_append_insn(out, OBBC_MAKE(OBBC_EXTEND, index & 0xf));
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

    if (code == OBBC_EXTEND) {
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
