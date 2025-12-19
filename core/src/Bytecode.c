#include "ob/String.h"
#include <ob/Array.h>
#include <ob/Bytecode.h>
#include <ob/Context.h>
#include <ob/Object.h>

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
  uint64_t index = 0;

  for (size_t pc = 0; pc < len; pc++) {
    auto opcode = OBBC_GET_OPCODE(code[pc]);
    auto data = OBBC_GET_DATA(code[pc]);

    auto this_index = (index << 4) | data;

    ob_ObjActivation *act = obobj_get_data(ctx->activation);
    ob_ObjMethod *method = obobj_get_data(act->method);

    ob_Obj literal = ((ob_Obj *)method->literals.data)[this_index];

    switch (opcode) {
    case OBBC_PUSH_LITERAL: {
      auto obj = ((ob_Obj *)method->literals.data)[this_index];
      obarr_push(&ctx->stack, sizeof(ob_Obj), (void *)&obj);
    }; break;

    case OBBC_SEND: {
      auto selector = *(ob_Str *)obobj_get_data(literal);
      auto nargs = nargs_for_sel(ctx, selector);

      auto stack_len = obarr_length(&ctx->stack, sizeof(ob_Obj));

      ob_Obj recv = *(ob_Obj *)obarr_at(&ctx->stack, sizeof(ob_Obj),
                                        stack_len - nargs - 1);

      obarr_remove(&ctx->stack, sizeof(ob_Obj), stack_len - nargs - 1);

      obctx_send(ctx, recv, selector);
    }; break;

    case OBBC_IMPLICIT_SEND: {
      auto selector = *(ob_Str *)obobj_get_data(literal);

      obctx_send(ctx, act->env, selector);
    }; break;

    case OBBC_EXTEND: {
      index = this_index;
      continue;
    }; break;

      // TODO: OP_RETURN

    case OBBC_SELF: {
      obarr_push(&ctx->stack, sizeof(ob_Obj), (void *)act->receiver);
    }; break;

    case OBBC_ARRAY: {
      auto obj = obctx_alloc_array(ctx);
      ob_Array *arr = obobj_get_data(obj);

      obarr_reserve(arr, this_index * sizeof(ob_Obj));

      obarr_pop(&ctx->stack, this_index * sizeof(ob_Obj), arr->data);
      arr->size = this_index * sizeof(ob_Obj);

      obctx_push(ctx, obj);
    }; break;

    default:
      break;
    }

    obctx_gc(ctx);

    index = 0;
  }
}

void obbc_append_insn(ob_Array *out, ob_Instruction insn) {
  obarr_push(out, sizeof(ob_Instruction), &insn);
}

// NOLINTBEGIN
uint8_t obbc_append_index(ob_Array *out, uint64_t index) {
  while (index > 15) {
    obbc_append_insn(out, OBBC_MAKE(OBBC_EXTEND, index & 0xf));
    index >>= 4;
  }

  return index;
}
// NOLINTEND
