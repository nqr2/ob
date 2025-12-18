#include "ob/Table.h"
#include <ob/Allocator.h>
#include <ob/Array.h>
#include <ob/Assert.h>
#include <ob/Bytecode.h>
#include <ob/Context.h>
#include <ob/Exn.h>
#include <ob/Exncodes.h>
#include <ob/Hash.h>
#include <ob/Number.h>
#include <ob/Object.h>
#include <ob/String.h>

#include <ctype.h>
#include <string.h>

#define DEFAULT_GC_FACTOR 1.5f

static void gc_sweep(ob_Context ctx);

ob_Context obctx_create(ob_Allocator *alloc) {
  ob_Context ctx = ob_allocate(alloc, sizeof(struct Context));

  ctx->gc_state.factor = DEFAULT_GC_FACTOR;
  ctx->gc_state.enabled = true;

  obarr_init(&ctx->stack, alloc);
  obarr_init(&ctx->string_data, alloc);
  obarr_init(&ctx->string_available, alloc);

  ctx->allocator = alloc;

  ctx->proto_object = obctx_alloc_slots(ctx, NULL);

  ctx->proto_nil = obctx_alloc_slots(ctx, NULL);
  ctx->proto_symbol = obctx_alloc_slots(ctx, NULL);
  ctx->proto_string = obctx_alloc_slots(ctx, NULL);
  ctx->proto_slots = obctx_alloc_slots(ctx, NULL);
  ctx->proto_number = obctx_alloc_slots(ctx, NULL);
  ctx->proto_array = obctx_alloc_slots(ctx, NULL);
  ctx->proto_method = obctx_alloc_slots(ctx, NULL);
  ctx->proto_lightcmethod = obctx_alloc_slots(ctx, NULL);
  ctx->proto_lightcdata = obctx_alloc_slots(ctx, NULL);
  ctx->proto_activation = obctx_alloc_slots(ctx, NULL);

  ctx->shell = obctx_alloc_slots(ctx, NULL);

  obtbl_init(&ctx->interned, ctx->allocator);

  obexn_init(&ctx->exnbuf, ctx->allocator);

  ctx->gc_state.previous_hs = ctx->allocator->used;

  return ctx;
}

void obctx_destroy(ob_Context ctx) {
  auto alloc = ctx->allocator;

  gc_sweep(ctx);
  gc_sweep(ctx);

  obarr_free(&ctx->stack);
  obarr_free(&ctx->string_data);
  obarr_free(&ctx->string_available);

  obtbl_free(&ctx->interned);

  obexn_free(&ctx->exnbuf);

  ob_deallocate(alloc, sizeof(struct Context), ctx);
}

ob_Obj obctx_allocate(ob_Context ctx, ob_ObjectTag tag, size_t payload_size) {
  auto obj =
      (ob_Obj)ob_allocate(ctx->allocator, sizeof(ob_Object) + payload_size);

  obj->next = ctx->objects;
  obj->size = payload_size;
  obj->header.tag = tag;

  ctx->objects = obj;

  return obj;
}

ob_Obj obctx_alloc_symbol(ob_Context ctx, ob_Str symbol) {
  auto data = obstr_get_data(ctx, symbol);
  auto len = obstr_get_length(symbol);

  auto hash = obhash_start(len, data);

  ob_Obj obj = NULL;

  if (obtbl_get(&ctx->interned, hash, (void **)&obj)) {
    return obj;
  }

  obj = obctx_allocate(ctx, OBOBJ_SYMBOL, sizeof(ob_Str));

  auto sym = (ob_Str *)obobj_get_data(obj);
  *sym = symbol;

  obtbl_set(&ctx->interned, hash, (void *)obj);

  return obj;
}

ob_Obj obctx_alloc_string(ob_Context ctx, ob_Str string) {
  auto obj = obctx_allocate(ctx, OBOBJ_STRING, sizeof(ob_Str));

  auto str = (ob_Str *)obobj_get_data(obj);
  *str = string;

  return obj;
}

ob_Obj obctx_alloc_slots(ob_Context ctx, ob_Obj prototype) {
  auto obj = obctx_allocate(ctx, OBOBJ_SLOTS, sizeof(ob_ObjSlots));

  ob_ObjSlots *slots = obobj_get_data(obj);

  obtbl_init(&slots->slots, ctx->allocator);
  slots->prototype = prototype;

  return obj;
}

ob_Obj obctx_alloc_number(ob_Context ctx, ob_Number number) {
  auto obj = obctx_allocate(ctx, OBOBJ_NUMBER, sizeof(ob_Number));

  ob_Number *num = obobj_get_data(obj);
  *num = number;

  return obj;
}

ob_Obj obctx_alloc_integer(ob_Context ctx, int64_t number) {
  return obctx_alloc_number(ctx, obnum_of_int(number));
}

ob_Obj obctx_alloc_real(ob_Context ctx, double number) {
  return obctx_alloc_number(ctx, obnum_of_float(number));
}

ob_Obj obctx_alloc_array(ob_Context ctx) {
  auto obj = obctx_allocate(ctx, OBOBJ_ARRAY, sizeof(ob_Array));

  ob_Array *arr = obobj_get_data(obj);
  obarr_init(arr, ctx->allocator);

  return obj;
}

ob_Obj obctx_alloc_method(ob_Context ctx) {
  auto obj = obctx_allocate(ctx, OBOBJ_METHOD, sizeof(ob_ObjMethod));

  ob_ObjMethod *method = obobj_get_data(obj);

  // TODO: bind any later activation environments to this env
  method->env = ctx->activation;

  obarr_init(&method->bytecode, ctx->allocator);
  obarr_init(&method->literals, ctx->allocator);
  obarr_init(&method->parameters, ctx->allocator);

  return obj;
}

ob_Obj obctx_alloc_lightcmethod(ob_Context ctx, ob_FnCMethod method) {
  auto obj = obctx_allocate(ctx, OBOBJ_LIGHTCMETHOD, sizeof(ob_FnCMethod));

  auto data = (ob_FnCMethod *)obobj_get_data(obj);
  *data = method;

  return obj;
}

ob_Obj obctx_alloc_lightcdata(ob_Context ctx, void *cdata) {
  auto obj = obctx_allocate(ctx, OBOBJ_LIGHTCDATA, sizeof(void *));

  auto data = (void **)obobj_get_data(obj);
  *data = cdata;

  return obj;
}

static void deallocate(ob_Context ctx, ob_Obj object) {
  auto size = sizeof(ob_Object) + object->size;

  obobj_destroy(object);
  ob_deallocate(ctx->allocator, size, object);
}

static void gc_mark(ob_Context ctx) {
  obobj_mark(ctx->activation);

  obobj_mark(ctx->proto_object);

  obobj_mark(ctx->proto_nil);
  obobj_mark(ctx->proto_symbol);
  obobj_mark(ctx->proto_string);
  obobj_mark(ctx->proto_slots);
  obobj_mark(ctx->proto_number);
  obobj_mark(ctx->proto_array);
  obobj_mark(ctx->proto_method);
  obobj_mark(ctx->proto_lightcmethod);
  obobj_mark(ctx->proto_lightcdata);
  obobj_mark(ctx->proto_activation);

  obobj_mark(ctx->shell);

  auto data = (ob_Obj *)ctx->stack.data;

  for (size_t i = 0; i < ctx->stack.size / sizeof(ob_Obj); i++) {
    obobj_mark(data[i]);
  }

  uint64_t index = 0;
  ob_Obj obj = NULL;

  while (obtbl_iterate(&ctx->interned, &index, NULL, (void **)&obj)) {
    obobj_mark(obj);
  }
}

// TODO:undebug
static void gc_sweep(ob_Context ctx) {
  obstr_sweep(ctx);

  ob_Obj newlive = NULL;
  auto live = ctx->objects;

  while (live != NULL) {
    ob_Obj next = live->next;

    if (live->header.mark) {
      live->header.mark = false;
      live->next = newlive;
      newlive = live;
    } else {
      deallocate(ctx, live);
    }

    live = next;
  }

  ctx->objects = newlive;
}

void obctx_gc(ob_Context ctx) {
  if (!ctx->gc_state.enabled) {
    return;
  }

  auto max_hs =
      (size_t)((float)ctx->gc_state.previous_hs * ctx->gc_state.factor);

  if (ctx->allocator->used > max_hs) {
    gc_mark(ctx);
    gc_sweep(ctx);

    ctx->gc_state.previous_hs = ctx->allocator->used;
  }
}

void obctx_enter_activation(ob_Context ctx, ob_Obj caller, ob_Obj method,
                            ob_Obj receiver) {
  auto act = obctx_allocate(ctx, OBOBJ_ACTIVATION, sizeof(ob_ObjActivation));

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

  if (obj == ctx->proto_slots) {
    return ctx->proto_object;
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
  case OBOBJ_LIGHTCMETHOD:
    return ctx->proto_lightcmethod;
  case OBOBJ_LIGHTCDATA:
    return ctx->proto_lightcdata;
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

ob_Obj obctx_get_slot(ob_Context ctx, ob_Obj obj, ob_Str selector) {
  while (obj != NULL) {
    if (OBOBJ_ISA(obj, OBOBJ_SLOTS)) {
      ob_ObjSlots *data = obobj_get_data(obj);

      auto str = obstr_get_data(ctx, selector);
      auto hash = obhash_start(selector->length, str);

      if (obtbl_get(&data->slots, hash, (void **)&obj)) {
        return obj;
      }
    }

    if (obj != ctx->proto_object) {
      obj = obctx_get_prototype(ctx, obj);
    } else {
      break;
    }
  }

  return NULL;
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

  if (tag == OBOBJ_LIGHTCMETHOD) {
    ctx->gc_state.enabled = false;

    obctx_enter_activation(ctx, ctx->activation, invoked, recv);
    auto data = (ob_FnCMethod *)obobj_get_data(invoked);

    if (!(*data)(ctx)) {
      obctx_push(ctx, recv);
    }

    obctx_leave_activation(ctx);

    // TODO: check that we actually popped n args

    ctx->gc_state.enabled = true;
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

ob_Exncode obctx_pcall(ob_Context ctx,
                       void (*inner)(ob_Context ctx, void *userdata),
                       void *userdata) {
  OB_EXN_BEGIN(&ctx->exnbuf, {
    auto code = obexn_code(&ctx->exnbuf);
    obexn__end(&ctx->exnbuf);
    return code;
  });

  inner(ctx, userdata);

  OB_EXN_END(&ctx->exnbuf);

  return OB_OK;
}
