#include <ob/Array.h>
#include <ob/Bytecode.h>
#include <ob/Context.h>
#include <ob/Object.h>

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
      ob_Obj recv = NULL;
      obarr_pop(&ctx->stack, sizeof(ob_Obj), (void *)&recv);

      ob_ObjString *selector = obobj_get_data(literal);

      obctx_send(ctx, recv, selector->inner);
    }; break;

    case OBBC_IMPLICIT_SEND: {
      ob_ObjString *selector = obobj_get_data(literal);

      obctx_send(ctx, act->env, selector->inner);
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
      ob_ObjArray *oarr = obobj_get_data(obj);

      auto arr = &oarr->items;
      obarr_reserve(arr, this_index * sizeof(ob_Obj));

      obarr_pop(&ctx->stack, this_index * sizeof(ob_Obj), arr->data);
      arr->size = this_index * sizeof(ob_Obj);

      obctx_push(ctx, obj);
    }; break;

    default:
      break;
    }

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
