#include <ctype.h>
#include <stdbit.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "Allocator.h"
#include "Array.h"
#include "Macros.h"
#include "Object.h"

[[deprecated("use Context.allocator")]]
Allocator *allocator;

[[deprecated("use a parameter of type Context")]]
Context gctx;

#include "Hash.h"

#include "Table.h"

#include "String.h"

#include "Interner.h"

#include "Context.h"

// TODO: add every case

#define MAX_PROTOTYPES 16
[[deprecated("use Context.prototypes")]]
Object *base_prototypes[MAX_PROTOTYPES];

Object *obj_getproto(Object *obj) {
  auto tag = HEADER_GET_TAG(obj->header);

  if (tag == OT_SLOTS) {
    ObjSlots *data = obj_payload(obj);

    if (data->prototype != NULL) {
      return data->prototype;
    }
  }

  return base_prototypes[tag];
}

void init_prototypes() {
  for (int i = 0; i < MAX_PROTOTYPES; i++) {
    base_prototypes[i] = obj_create_slots(gctx, NULL);
  }
}

Object *activation;

typedef uint8_t Instruction;

#define INSN_GET_OPCODE(I) ((I) & 0xf)
#define INSN_GET_DATA(I) ((I) >> 4)

typedef enum {
  OP_PUSH_LITERAL = 0,  // push a literal
  OP_SEND = 1,          // send a message to a known receiver
  OP_IMPLICIT_SEND = 2, // send a message to the implicit receiver
  OP_EXTEND = 3,        // extend the payload by prepending 4 bits
  OP_RETURN = 4,        // implements ^
  OP_SELF = 5,          // push the explicit receiver
  OP_SELF_SEND = 6,     // send a message to the explicit receiver
  OP_R7 = 7,
  OP_R8 = 8,
  OP_R9 = 9,
  OP_Ra = 10,
  OP_Rb = 11,
  OP_Rc = 12,
  OP_Rd = 13,
  OP_Re = 14,
  OP_Rf = 15,
} Opcode;

[[deprecated("use Context.activation")]]
Object *activation = NULL;

bool obj_isa(Object *obj, ObjectTag tag) {
  return HEADER_GET_TAG(obj->header) == tag;
}

Object *obj_get(Object *obj, String *selector) {
  if (obj == NULL) {
    return NULL;
  }

  if (obj_isa(obj, OT_SLOTS)) {
    ObjSlots *data = obj_payload(obj);
    IGNORE data;

    auto hash = hash_start(selector->length, selector->data);

    if (tbl_get(&data->slots, hash, (void **)&obj)) {
      return obj;
    }
  }

  return obj_get(obj_getproto(obj), selector);
}

size_t stack_len() {
  return gctx->stack.size / sizeof(Object *);
}

bool checkstack(size_t n) {
  return stack_len() >= n;
}

bool obj_is_invokable(Object *obj) {
  auto tag = HEADER_GET_TAG(obj->header);

  return (tag == OT_METHOD) || (tag == OT_CMETHOD);
}

void run_bytecode(Context ctx, size_t len, const uint8_t *code);

void obj_send(Object *recv, String *selector) {
  size_t n_args = 0;

  if (ispunct(selector->data[0])) {
    n_args = 1;
  } else {

    for (size_t i = 0; i < selector->length; i++) {
      if (selector->data[i] == ':') {
        n_args++;
      }
    }
  }

  IGNORE /*TODO: assert its true*/ checkstack(n_args);

  auto invoked = obj_get(recv, selector);

  IGNORE /*TODO: also this assert*/ obj_is_invokable(invoked);

  ctx_enter_activation(gctx, gctx->activation, invoked, recv);

  auto tag = HEADER_GET_TAG(invoked->header);

  if (tag == OT_CMETHOD) {
    ObjCMethod *data = obj_payload(invoked);
    data->method();
  }

  if (tag == OT_METHOD) {
    ObjMethod *data = obj_payload(invoked);
    run_bytecode(gctx, data->bytecode.size, data->bytecode.data);
  }

  // TODO: call closure

  ctx_leave_activation(gctx);
}

ObjActivation *get_activation() {
  return obj_payload(gctx->activation);
}

void run_bytecode(Context ctx, size_t len, const uint8_t *code) {
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

      obj_send(recv, selector->inner);
    }; break;

    case OP_IMPLICIT_SEND: {
      ObjString *selector = obj_payload(literal);

      obj_send(act->env, selector->inner);
    }; break;

    case OP_SELF_SEND: {
      ObjString *selector = obj_payload(literal);

      obj_send(act->receiver, selector->inner);
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

#include <stdio.h>

bool o__print() {
  printf("activation:%p\n", (void *)gctx->activation);

  return false;
}

int main() {
  auto alloc = get_libc_allocator();
  allocator = &alloc;

  auto ctx = ctx_create(&alloc);
  gctx = ctx;

  init_prototypes();

  auto obj = obj_create_slots(ctx, NULL);

  ObjSlots *data = obj_payload(obj);

  auto sel = str_create(ctx, strlen("print"), "print");

  auto print = obj_create_cmethod(ctx, o__print);

  tbl_set(&data->slots, hash_start(sel->length, sel->data), (void *)print);

  obj_send(obj, sel);

  ctx_sweep(ctx);

  ctx_destroy(ctx);

  return 0;
}
