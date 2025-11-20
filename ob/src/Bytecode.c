#include "Bytecode.h"

#include "Context.h"
#include "Object.h"

void bc_run(Context ctx, size_t len, const uint8_t *code) {
  uint64_t index = 0;

  for (size_t pc = 0; pc < len; pc++) {
    auto opcode = INSN_GET_OPCODE(code[pc]);
    auto data = INSN_GET_DATA(code[pc]);

    auto this_index = (index << 4) | data;

    ObjActivation *act = obj_payload(ctx->activation);
    ObjMethod *method = obj_payload(act->method);

    Object *literal = ((Object **)method->literals.data)[this_index];

    switch (opcode) {
    case OP_PUSH_LITERAL: {
      auto obj = &((Object **)method->literals.data)[this_index];
      arr_push(&ctx->stack, sizeof(Object *), (void *)obj);
    }; break;

    case OP_SEND: {
      Object *recv = NULL;
      arr_pop(&ctx->stack, sizeof(Object *), (void *)&recv);

      ObjString *selector = obj_payload(literal);

      obj_send(ctx, recv, selector->inner);
    }; break;

    case OP_IMPLICIT_SEND: {
      ObjString *selector = obj_payload(literal);

      obj_send(ctx, act->env, selector->inner);
    }; break;

    case OP_SELF_SEND: {
      ObjString *selector = obj_payload(literal);

      obj_send(ctx, act->receiver, selector->inner);
    }; break;

    case OP_EXTEND: {
      index = this_index;
      continue;
    }; break;

    case OP_SELF: {
      arr_push(&ctx->stack, sizeof(Object *), (void *)act->receiver);
    }; break;

      // TODO: OP_RETURN

    default:
      break;
    }

    index = 0;
  }
}

void bc_append_insn(Array *out, Instruction insn) {
  arr_push(out, sizeof(Instruction), &insn);
}

uint8_t bc_append_index(Array *out, uint64_t index) {
  while (index > 15) {
    uint8_t lit = (index & 0xf) << 4;
    bc_append_insn(out, OP_EXTEND | lit);
    index >>= 4;
  }

  return index;
}
