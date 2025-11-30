#include "Context.h"
#include "Allocator.h"
#include "Array.h"
#include "Assert.h"
#include "Bytecode.h"
#include "Hash.h"
#include "Object.h"

#include <ctype.h>
#include <string.h>

Context ctx_create(Allocator *alloc) {
  Context ctx = allocate(alloc, sizeof(struct Context));

  memset(ctx, 0, sizeof(struct Context));

  arr_init(&ctx->stack, alloc);
  arr_init(&ctx->string_data, alloc);
  arr_init(&ctx->string_available, alloc);

  ctx->allocator = alloc;

  ctx->proto_nil = ctx_alloc_slots(ctx, NULL);
  ctx->proto_symbol = ctx_alloc_slots(ctx, NULL);
  ctx->proto_string = ctx_alloc_slots(ctx, NULL);
  ctx->proto_slots = ctx_alloc_slots(ctx, NULL);
  ctx->proto_integer = ctx_alloc_slots(ctx, NULL);
  ctx->proto_real = ctx_alloc_slots(ctx, NULL);
  ctx->proto_method = ctx_alloc_slots(ctx, NULL);
  ctx->proto_cmethod = ctx_alloc_slots(ctx, NULL);
  ctx->proto_cdata = ctx_alloc_slots(ctx, NULL);
  ctx->proto_activation = ctx_alloc_slots(ctx, NULL);

  return ctx;
}

void ctx_destroy(Context ctx) {
  auto alloc = ctx->allocator;

  // TODO: get rid of objects and strings here

  ctx_sweep(ctx);
  ctx_sweep(ctx);

  arr_free(&ctx->stack);
  arr_free(&ctx->string_data);
  arr_free(&ctx->string_available);

  deallocate(alloc, ctx);
}

Obj ctx_allocate(Context ctx, size_t payload_size) {
  auto obj = (Obj)allocate(ctx->allocator, sizeof(Object) + payload_size);

  obj->header = 0;
  obj->next = ctx->objects;

  ctx->objects = obj;

  return obj;
}

Obj ctx_alloc_symbol(Context ctx, String *symbol) {
  auto obj = ctx_allocate(ctx, sizeof(ObjSymbol));
  obj->header = HEADER_SET_TAG(0, OT_SYMBOL);

  ObjSymbol *sym = obj_get_data(obj);
  sym->inner = symbol;

  return obj;
}

Obj ctx_alloc_string(Context ctx, String *string) {
  auto obj = ctx_allocate(ctx, sizeof(ObjString));
  obj->header = HEADER_SET_TAG(0, OT_STRING);

  ObjString *str = obj_get_data(obj);
  str->inner = string;

  return obj;
}

Obj ctx_alloc_slots(Context ctx, Obj prototype) {
  auto obj = ctx_allocate(ctx, sizeof(ObjSlots));
  obj->header = HEADER_SET_TAG(0, OT_SLOTS);

  ObjSlots *slots = obj_get_data(obj);

  tbl_init(&slots->slots, ctx->allocator);
  slots->prototype = prototype;

  return obj;
}

Obj ctx_alloc_integer(Context ctx, int64_t number) {
  auto obj = ctx_allocate(ctx, sizeof(ObjInteger));
  obj->header = HEADER_SET_TAG(0, OT_INTEGER);

  ObjInteger *num = obj_get_data(obj);
  num->number = number;

  return obj;
}

Obj ctx_alloc_real(Context ctx, double number) {
  auto obj = ctx_allocate(ctx, sizeof(ObjReal));
  obj->header = HEADER_SET_TAG(0, OT_REAL);

  ObjReal *num = obj_get_data(obj);
  num->number = number;

  return obj;
}

Obj ctx_alloc_method(Context ctx) {
  auto obj = ctx_allocate(ctx, sizeof(ObjMethod));
  obj->header = HEADER_SET_TAG(0, OT_SLOTS);

  ObjMethod *method = obj_get_data(obj);

  // TODO: bind any later activation environments to this env
  method->env = ctx->activation;

  arr_init(&method->bytecode, ctx->allocator);
  arr_init(&method->literals, ctx->allocator);
  arr_init(&method->parameters, ctx->allocator);

  return obj;
}

Obj ctx_alloc_cmethod(Context ctx, FnCMethod method) {
  auto obj = ctx_allocate(ctx, sizeof(ObjCMethod));
  obj->header = HEADER_SET_TAG(0, OT_CMETHOD);

  ObjCMethod *data = obj_get_data(obj);
  data->method = method;

  return obj;
}

Obj ctx_alloc_cdata(Context ctx, void *cdata) {
  auto obj = ctx_allocate(ctx, sizeof(ObjCData));
  obj->header = HEADER_SET_TAG(0, OT_CMETHOD);

  ObjCData *data = obj_get_data(obj);
  data->cdata = cdata;

  return obj;
}

void ctx_mark(Context ctx) {
  obj_mark(ctx->activation);

  obj_mark(ctx->proto_nil);
  obj_mark(ctx->proto_symbol);
  obj_mark(ctx->proto_string);
  obj_mark(ctx->proto_slots);
  obj_mark(ctx->proto_integer);
  obj_mark(ctx->proto_real);
  obj_mark(ctx->proto_method);
  obj_mark(ctx->proto_cmethod);
  obj_mark(ctx->proto_cdata);
  obj_mark(ctx->proto_activation);

  auto data = (Object **)ctx->stack.data;

  for (size_t i = 0; i < ctx->stack.size / sizeof(Object *); i++) {
    obj_mark(data[i]);
  }
}

void ctx_sweep(Context ctx) {
  str_sweep(ctx);

  Object *newlive = NULL;
  auto live = ctx->objects;

  while (live != NULL) {
    Object *next = live->next;

    if (HEADER_GET_MARK(live->header)) {
      live->header = HEADER_SET_MARK(live->header, false);
    } else {
      obj_destroy(live);
      deallocate(ctx->allocator, live);
      live = NULL;
    }

    if (live != NULL) {
      live->next = newlive;
      newlive = live;
    }

    live = next;
  }

  ctx->objects = newlive;
}

void ctx_enter_activation(Context ctx, Obj caller, Obj method, Obj receiver) {
  Object *act = ctx_allocate(ctx, sizeof(ObjActivation));

  ObjActivation *data = obj_get_data(act);
  data->parent = ctx->activation;
  data->caller = caller;
  data->method = method;
  data->receiver = receiver;
  data->env = ctx_alloc_slots(ctx, NULL);

  ctx->activation = act;
}

void ctx_leave_activation(Context ctx) {
  ObjActivation *data = obj_get_data(ctx->activation);
  ctx->activation = data->parent;
}

void ctx_push(Context ctx, Obj obj) {
  arr_push(&ctx->stack, sizeof(Obj), (const void *)&obj);
}

Obj ctx_pop(Context ctx) {
  Obj obj;

  if (!arr_pop(&ctx->stack, sizeof(Obj), (void *)&obj)) {
    // TODO: fail? cannot pop empty stack.
  }

  return obj;
}

bool ctx_checkstack(Context ctx, size_t narg) {
  return (ctx->stack.size / sizeof(Object *)) >= narg;
}

Obj ctx_get_prototype(Context ctx, Obj obj) {
  if (obj == ctx->proto_nil) {
    return NULL;
  }

  switch (obj_get_tag(obj)) {
  case OT_NIL:
    return ctx->proto_nil;
  case OT_SYMBOL:
    return ctx->proto_symbol;
  case OT_STRING:
    return ctx->proto_string;
  case OT_SLOTS: {
    ObjSlots *slots = obj_get_data(obj);

    if (slots->prototype != NULL) {
      return slots->prototype;
    }

    return ctx->proto_slots;
  }
  case OT_INTEGER:
    return ctx->proto_integer;
  case OT_REAL:
    return ctx->proto_real;
  case OT_METHOD:
    return ctx->proto_method;
  case OT_CMETHOD:
    return ctx->proto_cmethod;
  case OT_CDATA:
    return ctx->proto_cdata;
  case OT_ACTIVATION:
    return ctx->proto_activation;
  case OT_Ra:
  case OT_Rb:
  case OT_Rc:
  case OT_Rd:
  case OT_Re:
  case OT_Rf:
    break;
  }
  return ctx_get_prototype(ctx, NULL);
}

Obj ctx_get_slot(Context ctx, Obj obj, String *selector) {
  if (obj == NULL) {
    return NULL;
  }

  if (OBJ_ISA(obj, OT_SLOTS)) {
    ObjSlots *data = obj_get_data(obj);

    auto hash = hash_start(selector->length, selector->data);

    if (tbl_get(&data->slots, hash, (void **)&obj)) {
      return obj;
    }
  }

  return ctx_get_slot(ctx, ctx_get_prototype(ctx, obj), selector);
}

void ctx_send(Context ctx, Obj recv, String *selector) {
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

  auto invoked = ctx_get_slot(ctx, recv, selector);

  // TODO: doesNotUnderstand

  if (n_args != 0) {
    ASSERT(OBJ_IS_INVOCABLE(invoked), "tried to invoke a non-method");

    ASSERT(ctx_checkstack(ctx, n_args),
           "expected to have %lu arguments on stack", n_args);
  }

  auto tag = HEADER_GET_TAG(invoked->header);

  if (tag == OT_CMETHOD) {
    ctx_enter_activation(ctx, ctx->activation, invoked, recv);
    ObjCMethod *data = obj_get_data(invoked);

    if (!data->method(ctx)) {
      ctx_push(ctx, recv);
    }

    ctx_leave_activation(ctx);
  }

  else if (tag == OT_METHOD) {
    ctx_enter_activation(ctx, ctx->activation, invoked, recv);
    ObjMethod *data = obj_get_data(invoked);
    bc_run(ctx, data->bytecode.size, data->bytecode.data);
    ctx_leave_activation(ctx);
  } else {
    ctx_push(ctx, invoked);
  }
}
