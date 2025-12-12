#include <ob/Allocator.h>
#include <ob/Array.h>
#include <ob/Assert.h>
#include <ob/Bytecode.h>
#include <ob/Context.h>
#include <ob/Hash.h>
#include <ob/Number.h>
#include <ob/Object.h>
#include <ob/String.h>

#include <ctype.h>
#include <string.h>

ob_Context obctx_create(ob_Allocator *alloc) {
  ob_Context ctx = ob_allocate(alloc, sizeof(struct Context));

  memset(ctx, 0, sizeof(struct Context));

  obarr_init(&ctx->stack, alloc);
  obarr_init(&ctx->string_data, alloc);
  obarr_init(&ctx->string_available, alloc);

  ctx->allocator = alloc;

  ctx->proto_object = obctx_alloc_slots(ctx, NULL);

  ctx->proto_nil = obctx_alloc_slots(ctx, ctx->proto_object);
  ctx->proto_symbol = obctx_alloc_slots(ctx, ctx->proto_object);
  ctx->proto_string = obctx_alloc_slots(ctx, ctx->proto_object);
  ctx->proto_slots = obctx_alloc_slots(ctx, ctx->proto_object);
  ctx->proto_number = obctx_alloc_slots(ctx, ctx->proto_object);
  ctx->proto_array = obctx_alloc_slots(ctx, ctx->proto_object);
  ctx->proto_method = obctx_alloc_slots(ctx, ctx->proto_object);
  ctx->proto_cmethod = obctx_alloc_slots(ctx, ctx->proto_object);
  ctx->proto_cdata = obctx_alloc_slots(ctx, ctx->proto_object);
  ctx->proto_activation = obctx_alloc_slots(ctx, ctx->proto_object);

  ctx->shell = obctx_alloc_slots(ctx, ctx->proto_slots);

  return ctx;
}

void obctx_destroy(ob_Context ctx) {
  auto alloc = ctx->allocator;

  // TODO: get rid of objects and strings here

  obctx_sweep(ctx);
  obctx_sweep(ctx);

  obarr_free(&ctx->stack);
  obarr_free(&ctx->string_data);
  obarr_free(&ctx->string_available);

  ob_deallocate(alloc, ctx);
}

ob_Obj obctx_allocate(ob_Context ctx, size_t payload_size) {
  auto obj =
      (ob_Obj)ob_allocate(ctx->allocator, sizeof(ob_Object) + payload_size);

  obj->header = 0;
  obj->next = ctx->objects;

  ctx->objects = obj;

  return obj;
}

ob_Obj obctx_alloc_symbol(ob_Context ctx, ob_String *symbol) {
  auto obj = obctx_allocate(ctx, sizeof(ob_ObjSymbol));
  obj->header = OBOBJ_SYMBOL;

  ob_ObjSymbol *sym = obobj_get_data(obj);
  sym->inner = symbol;

  return obj;
}

ob_Obj obctx_alloc_string(ob_Context ctx, ob_String *string) {
  auto obj = obctx_allocate(ctx, sizeof(ob_ObjString));
  obj->header = OBOBJ_STRING;

  ob_ObjString *str = obobj_get_data(obj);
  str->inner = string;

  return obj;
}

ob_Obj obctx_alloc_slots(ob_Context ctx, ob_Obj prototype) {
  auto obj = obctx_allocate(ctx, sizeof(ob_ObjSlots));
  obj->header = OBOBJ_SLOTS;

  ob_ObjSlots *slots = obobj_get_data(obj);

  obtbl_init(&slots->slots, ctx->allocator);
  slots->prototype = prototype;

  return obj;
}

ob_Obj obctx_alloc_number(ob_Context ctx, ob_Number number) {
  auto obj = obctx_allocate(ctx, sizeof(ob_ObjNumber));
  obj->header = OBOBJ_NUMBER;

  ob_ObjNumber *num = obobj_get_data(obj);
  num->number = number;

  return obj;
}

ob_Obj obctx_alloc_integer(ob_Context ctx, int64_t number) {
  return obctx_alloc_number(ctx, obnum_of_int(number));
}

ob_Obj obctx_alloc_real(ob_Context ctx, double number) {
  return obctx_alloc_number(ctx, obnum_of_float(number));
}

ob_Obj obctx_alloc_array(ob_Context ctx) {
  auto obj = obctx_allocate(ctx, sizeof(ob_ObjArray));
  obj->header = OBOBJ_ARRAY;

  ob_ObjArray *arr = obobj_get_data(obj);
  obarr_init(&arr->items, ctx->allocator);

  return obj;
}

ob_Obj obctx_alloc_method(ob_Context ctx) {
  auto obj = obctx_allocate(ctx, sizeof(ob_ObjMethod));
  obj->header = OBOBJ_METHOD;

  ob_ObjMethod *method = obobj_get_data(obj);

  // TODO: bind any later activation environments to this env
  method->env = ctx->activation;

  obarr_init(&method->bytecode, ctx->allocator);
  obarr_init(&method->literals, ctx->allocator);
  obarr_init(&method->parameters, ctx->allocator);

  return obj;
}

ob_Obj obctx_alloc_cmethod(ob_Context ctx, ob_FnCMethod method) {
  auto obj = obctx_allocate(ctx, sizeof(ob_ObjCMethod));
  obj->header = OBOBJ_CMETHOD;

  ob_ObjCMethod *data = obobj_get_data(obj);
  data->method = method;

  return obj;
}

ob_Obj obctx_alloc_cdata(ob_Context ctx, void *cdata) {
  auto obj = obctx_allocate(ctx, sizeof(ob_ObjCData));
  obj->header = OBOBJ_CMETHOD;

  ob_ObjCData *data = obobj_get_data(obj);
  data->cdata = cdata;

  return obj;
}

void obctx_mark(ob_Context ctx) {
  obobj_mark(ctx->activation);

  obobj_mark(ctx->proto_nil);
  obobj_mark(ctx->proto_symbol);
  obobj_mark(ctx->proto_string);
  obobj_mark(ctx->proto_slots);
  obobj_mark(ctx->proto_number);
  obobj_mark(ctx->proto_array);
  obobj_mark(ctx->proto_method);
  obobj_mark(ctx->proto_cmethod);
  obobj_mark(ctx->proto_cdata);
  obobj_mark(ctx->proto_activation);

  auto data = (ob_Object **)ctx->stack.data;

  for (size_t i = 0; i < ctx->stack.size / sizeof(ob_Object *); i++) {
    obobj_mark(data[i]);
  }
}

void obctx_sweep(ob_Context ctx) {
  obstr_sweep(ctx);

  ob_Object *newlive = NULL;
  auto live = ctx->objects;

  while (live != NULL) {
    ob_Object *next = live->next;

    if (obobj_get_mark(live)) {
      live->header &= 0xffff'ffef;
      live->next = newlive;
      newlive = live;
    } else {
      obobj_destroy(live);
      ob_deallocate(ctx->allocator, live);
    }

    live = next;
  }

  ctx->objects = newlive;
}

void obctx_enter_activation(ob_Context ctx, ob_Obj caller, ob_Obj method,
                            ob_Obj receiver) {
  ob_Object *act = obctx_allocate(ctx, sizeof(ob_ObjActivation));

  ob_ObjActivation *data = obobj_get_data(act);
  data->parent = ctx->activation;
  data->caller = caller;
  data->method = method;
  data->receiver = receiver;
  data->env = obctx_alloc_slots(ctx, NULL);

  ctx->activation = act;
}

void obctx_leave_activation(ob_Context ctx) {
  ob_ObjActivation *data = obobj_get_data(ctx->activation);
  ctx->activation = data->parent;
}

void obctx_push(ob_Context ctx, ob_Obj obj) {
  obarr_push(&ctx->stack, sizeof(ob_Obj), (const void *)&obj);
}

ob_Obj obctx_pop(ob_Context ctx) {
  ob_Obj obj;

  if (!obarr_pop(&ctx->stack, sizeof(ob_Obj), (void *)&obj)) {
    // TODO: fail? cannot pop empty stack.
  }

  return obj;
}

bool obctx_checkstack(ob_Context ctx, size_t narg) {
  return (ctx->stack.size / sizeof(ob_Object *)) >= narg;
}

ob_Obj obctx_get_prototype(ob_Context ctx, ob_Obj obj) {
  if (obj == ctx->proto_object) {
    return NULL;
  }

  switch (obobj_get_tag(obj)) {
  case OBOBJ_NIL:
    return ctx->proto_nil;
  case OBOBJ_SYMBOL:
    return ctx->proto_symbol;
  case OBOBJ_STRING:
    return ctx->proto_string;
  case OBOBJ_SLOTS: {
    ob_ObjSlots *slots = obobj_get_data(obj);

    if (slots->prototype != NULL) {
      return slots->prototype;
    }

    return ctx->proto_slots;
  }
  case OBOBJ_NUMBER:
    return ctx->proto_number;
  case OBOBJ_ARRAY:
    return ctx->proto_array;
  case OBOBJ_METHOD:
    return ctx->proto_method;
  case OBOBJ_CMETHOD:
    return ctx->proto_cmethod;
  case OBOBJ_CDATA:
    return ctx->proto_cdata;
  case OBOBJ_ACTIVATION:
    return ctx->proto_activation;
  case OT_Ra:
  case OT_Rb:
  case OT_Rc:
  case OT_Rd:
  case OT_Re:
  case OT_Rf:
    break;
  }

  return obctx_get_prototype(ctx, NULL);
}

ob_Obj obctx_get_slot(ob_Context ctx, ob_Obj obj, ob_String *selector) {
  if (obj == NULL) {
    return NULL;
  }

  if (OBOBJ_ISA(obj, OBOBJ_SLOTS)) {
    ob_ObjSlots *data = obobj_get_data(obj);

    auto str = obstr_get_data(ctx, selector);
    auto hash = obhash_start(selector->length, str);

    if (obtbl_get(&data->slots, hash, (void **)&obj)) {
      return obj;
    }
  }

  return obctx_get_slot(ctx, obctx_get_prototype(ctx, obj), selector);
}

void obctx_send(ob_Context ctx, ob_Obj recv, ob_String *selector) {
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

  auto invoked = obctx_get_slot(ctx, recv, selector);

  // TODO: doesNotUnderstand

  if (n_args != 0) {
    ASSERT(OBOBJ_IS_INVOCABLE(invoked),
           "tried to invoke a non-method object %p", invoked);

    ASSERT(obctx_checkstack(ctx, n_args),
           "expected to have %lu arguments on stack", n_args);
  }

  auto tag = obobj_get_tag(invoked);

  if (tag == OBOBJ_CMETHOD) {
    obctx_enter_activation(ctx, ctx->activation, invoked, recv);
    ob_ObjCMethod *data = obobj_get_data(invoked);

    if (!data->method(ctx)) {
      obctx_push(ctx, recv);
    }

    obctx_leave_activation(ctx);

    // TODO: check that we actually popped n args
  }

  else if (tag == OBOBJ_METHOD) {
    obctx_enter_activation(ctx, ctx->activation, invoked, recv);

    // TODO: bind every argument to the implicit recv

    ob_ObjMethod *data = obobj_get_data(invoked);
    obbc_run(ctx, data->bytecode.size, data->bytecode.data);
    obctx_leave_activation(ctx);

  } else {
    obctx_push(ctx, invoked);
  }
}
