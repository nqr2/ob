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

typedef struct String String;
void str_mark(String *str);

void str_sweep();

void sweep() {
  str_sweep();

  Object *newlive = NULL;

  while (live != NULL) {
    Object *next = live->next;

    if (HEADER_GET_MARK(live->header)) {
      live->header = HEADER_SET_MARK(live->header, false);
    } else {
      obj__destroy(live);
      deallocate(allocator, live);
      live = NULL;
    }

    if (live != NULL) {
      live->next = newlive;
      newlive = live;
    }

    live = next;
  }

  live = newlive;
}

#include "Hash.h"

#include "Table.h"

#include "String.h"

#include "Interner.h"

typedef struct {
  String *inner;
} ObjSymbol, ObjString;

typedef struct {
  Object *prototype;
  Table slots;
} ObjSlots;

typedef struct {
  Object *env;
  Array parameters;
  Array bytecode;
  Array literals;
} ObjMethod;

typedef bool (*FnCMethod)();

typedef struct {
  FnCMethod cfunction;
} ObjCMethod;

typedef struct {
  void *cdata;
} ObjCData;

typedef struct {
  int64_t number;
} ObjInteger;

typedef struct {
  double number;
} ObjReal;

typedef struct {
  Object *parent;   // the parent activation
  Object *caller;   // the method's caller
  Object *method;   // this method
  Object *receiver; // this method's receiver
  Object *env;      // this context's environment
} ObjActivation;

// TODO: add every case
void obj__destroy(Object *obj) {
  obj_visit(obj, obj__unref_);

  switch (HEADER_GET_TAG(obj->header)) {

  case OT_SLOTS: {
    ObjSlots *data = obj_payload(obj);
    tbl_free(&data->slots);
  } break;

  case OT_METHOD: {
    ObjMethod *data = obj_payload(obj);
    arr_free(&data->parameters);
    arr_free(&data->literals);
    arr_free(&data->bytecode);
  } break;

    // TODO: implement uninterning afterwards

  default:
    break;
  }
}

void obj_visit(Object *obj, FnVisitor visit) {
  // TODO: properly handle NULLs.
  if (obj == NULL) {
    return;
  }

  visit(obj);

  switch (HEADER_GET_TAG(obj->header)) {
  case OT_SLOTS: {
    ObjSlots *data = obj_payload(obj);

    Object *ref = NULL;
    uint64_t index = 0;

    while (tbl_iterate(&data->slots, &index, NULL, (void **)&ref)) {
      obj_visit(ref, visit);
    }

    obj_visit(data->prototype, visit);
  } break;

  case OT_METHOD: {
    ObjMethod *data = obj_payload(obj);

    for (size_t i = 0; i < data->literals.size / sizeof(Object *); i++) {
      Object *item = ((Object **)data->literals.data)[i];

      obj_visit(item, visit);
    }

    obj_visit(data->env, visit);
  }; break;

  case OT_ACTIVATION: {
    ObjActivation *data = obj_payload(obj);
    obj_visit(data->parent, visit);
    obj_visit(data->caller, visit);
    obj_visit(data->method, visit);
    obj_visit(data->receiver, visit);
    obj_visit(data->env, visit);
  }; break;

  default:
    break;
  }
}

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

Object *obj_create_slots(Object *prototype) {
  Object *obj = obj_create(sizeof(ObjSlots));

  obj->header = HEADER_SET_TAG(0, OT_SLOTS);

  ObjSlots *data = obj_payload(obj);
  data->prototype = prototype;

  tbl_init(&data->slots, allocator);

  return obj;
}

void init_prototypes() {
  for (int i = 0; i < MAX_PROTOTYPES; i++) {
    base_prototypes[i] = obj_create_slots(NULL);
  }
}

Object *activation;

void mark() {
  obj_mark(activation);

  for (int i = 0; i < MAX_PROTOTYPES; i++) {
    obj_mark(base_prototypes[i]);
  }

  auto data = (Object **)stack.data;

  for (size_t i = 0; i < stack.size / sizeof(Object *); i++) {
    obj_mark(data[i]);
  }
}

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

void enter_activation(Object *caller, Object *method, Object *receiver) {
  Object *act = obj_create(sizeof(ObjActivation));

  ObjActivation *data = obj_payload(act);
  data->parent = activation;
  data->caller = caller;
  data->method = method;
  data->receiver = receiver;
  data->env = obj_create_slots(NULL);

  activation = act;
}

void leave_activation() {
  ObjActivation *data = obj_payload(activation);
  activation = data->parent;
}

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
  return stack.size / sizeof(Object *);
}

bool checkstack(size_t n) {
  return stack_len() >= n;
}

bool obj_is_invokable(Object *obj) {
  auto tag = HEADER_GET_TAG(obj->header);

  return (tag == OT_METHOD) || (tag == OT_CMETHOD);
}

void run_bytecode(size_t len, const uint8_t *code);

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

  enter_activation(activation, invoked, recv);

  auto tag = HEADER_GET_TAG(invoked->header);

  if (tag == OT_CMETHOD) {
    ObjCMethod *data = obj_payload(invoked);
    data->cfunction();
  }

  if (tag == OT_METHOD) {
    ObjMethod *data = obj_payload(invoked);
    run_bytecode(data->bytecode.size, data->bytecode.data);
  }

  // TODO: call closure

  leave_activation();
}

ObjActivation *get_activation() {
  return obj_payload(activation);
}

void run_bytecode(size_t len, const uint8_t *code) {
  uint64_t index = 0;

  for (size_t pc = 0; pc < len; pc++) {
    auto opcode = INSN_GET_OPCODE(code[pc]);
    auto data = INSN_GET_DATA(code[pc]);

    auto this_index = (index << 4) | data;

    ObjActivation *act = obj_payload(activation);
    ObjMethod *method = obj_payload(act->method);

    Object *literal = ((Object **)method->literals.data)[this_index];

    switch (opcode) {
    case OP_PUSH_LITERAL: {
      auto obj = &((Object **)method->literals.data)[this_index];
      arr_push(&stack, sizeof(Object *), (void *)obj);
    }; break;

    case OP_SEND: {
      Object *recv = NULL;
      arr_pop(&stack, sizeof(Object *), (void *)&recv);

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
      arr_push(&stack, sizeof(Object *), (void *)act->receiver);
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
  printf("activation:%p\n", (void *)activation);

  return false;
}

Object *obj_create_cfunction(FnCMethod fun) {
  Object *obj = obj_create(sizeof(ObjCMethod));

  obj->header = HEADER_SET_TAG(0, OT_CMETHOD);

  ObjCMethod *data = obj_payload(obj);
  data->cfunction = fun;

  return obj;
}

int main() {
  auto alloc = get_libc_allocator();
  allocator = &alloc;

  init_prototypes();

  arr_init(&string_data, &alloc);
  arr_init(&string_available, &alloc);
  arr_init(&stack, &alloc);

  auto obj = obj_create_slots(NULL);

  ObjSlots *data = obj_payload(obj);

  auto sel = str_create(strlen("print"), "print");

  auto print = obj_create_cfunction(o__print);

  tbl_set(&data->slots, hash_start(sel->length, sel->data), (void *)print);

  obj_send(obj, sel);

  sweep();

  arr_free(&stack);
  arr_free(&string_data);
  arr_free(&string_available);

  return 0;
}
