#include "Bytecode.h"

#include "Array.h"
#include "Context.h"
#include "Object.h"

void bc_run(Context ctx, size_t len, const uint8_t *code) {
  uint64_t index = 0;

  for (size_t pc = 0; pc < len; pc++) {
    auto opcode = INSN_GET_OPCODE(code[pc]);
    auto data = INSN_GET_DATA(code[pc]);

    auto this_index = (index << 4) | data;

    ObjActivation *act = obj_get_data(ctx->activation);
    ObjMethod *method = obj_get_data(act->method);

    Obj literal = ((Obj *)method->literals.data)[this_index];

    switch (opcode) {
    case OP_PUSH_LITERAL: {
      auto obj = ((Obj *)method->literals.data)[this_index];
      arr_push(&ctx->stack, sizeof(Obj), (void *)&obj);
    }; break;

    case OP_SEND: {
      Obj recv = NULL;
      arr_pop(&ctx->stack, sizeof(Obj), (void *)&recv);

      ObjString *selector = obj_get_data(literal);

      ctx_send(ctx, recv, selector->inner);
    }; break;

    case OP_IMPLICIT_SEND: {
      ObjString *selector = obj_get_data(literal);

      ctx_send(ctx, act->env, selector->inner);
    }; break;

    case OP_EXTEND: {
      index = this_index;
      continue;
    }; break;

      // TODO: OP_RETURN

    case OP_SELF: {
      arr_push(&ctx->stack, sizeof(Obj), (void *)act->receiver);
    }; break;

    case OP_ARRAY: {
      auto obj = ctx_alloc_array(ctx);
      ObjArray *oarr = obj_get_data(obj);

      auto arr = &oarr->items;
      arr_reserve(arr, this_index * sizeof(Obj));

      arr_pop(&ctx->stack, this_index * sizeof(Obj), arr->data);
      arr->size = this_index * sizeof(Obj);

      ctx_push(ctx, obj);
    }; break;

    default:
      break;
    }

    index = 0;
  }
}

void bc_append_insn(Array *out, Instruction insn) {
  arr_push(out, sizeof(Instruction), &insn);
}

// NOLINTBEGIN
uint8_t bc_append_index(Array *out, uint64_t index) {
  while (index > 15) {
    bc_append_insn(out, INSN_MAKE(OP_EXTEND, index & 0xf));
    index >>= 4;
  }

  return index;
}
// NOLINTEND
