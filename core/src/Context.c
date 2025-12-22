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

  ctx->proto.object = obctx_alloc_slots(ctx, NULL);

  ctx->proto.nil = obctx_alloc_slots(ctx, NULL);
  ctx->proto.symbol = obctx_alloc_slots(ctx, NULL);
  ctx->proto.string = obctx_alloc_slots(ctx, NULL);
  ctx->proto.slots = obctx_alloc_slots(ctx, NULL);
  ctx->proto.number = obctx_alloc_slots(ctx, NULL);
  ctx->proto.array = obctx_alloc_slots(ctx, NULL);
  ctx->proto.method = obctx_alloc_slots(ctx, NULL);
  ctx->proto.lightcmethod = obctx_alloc_slots(ctx, NULL);
  ctx->proto.cmethod = obctx_alloc_slots(ctx, NULL);
  ctx->proto.lightcdata = obctx_alloc_slots(ctx, NULL);
  ctx->proto.cdata = obctx_alloc_slots(ctx, NULL);
  ctx->proto.activation = obctx_alloc_slots(ctx, NULL);

  ctx->known.shell = obctx_alloc_slots(ctx, NULL);
  ctx->known.o_true = obctx_alloc_slots(ctx, NULL);
  ctx->known.o_false = obctx_alloc_slots(ctx, NULL);

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

  method->env = ctx->this_activation;

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

ob_Obj obctx_alloc_cdata(ob_Context ctx, ob_Obj prototype, ob_FnVisit visit,
                         ob_FnDestroy destructor, void *data) {
  auto obj = obctx_allocate(ctx, OBOBJ_CDATA, sizeof(ob_ObjCData));

  ob_ObjCData *cdata = obobj_get_data(obj);
  cdata->prototype = prototype;
  cdata->visit = visit;
  cdata->destroy = destructor;
  cdata->data = data;

  return obj;
}

static void deallocate(ob_Context ctx, ob_Obj object) {
  auto size = sizeof(ob_Object) + object->size;

  obobj_destroy(object);
  ob_deallocate(ctx->allocator, size, object);
}

static void gc_mark(ob_Context ctx) {
  obobj_mark(ctx->this_activation);

  obobj_mark(ctx->proto.object);

  obobj_mark(ctx->proto.nil);
  obobj_mark(ctx->proto.symbol);
  obobj_mark(ctx->proto.string);
  obobj_mark(ctx->proto.slots);
  obobj_mark(ctx->proto.number);
  obobj_mark(ctx->proto.array);
  obobj_mark(ctx->proto.method);
  obobj_mark(ctx->proto.lightcmethod);
  obobj_mark(ctx->proto.cmethod);
  obobj_mark(ctx->proto.lightcdata);
  obobj_mark(ctx->proto.cdata);
  obobj_mark(ctx->proto.activation);

  obobj_mark(ctx->known.shell);
  obobj_mark(ctx->known.o_false);
  obobj_mark(ctx->known.o_true);

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

void obctx_enter_activation(ob_Context ctx, ob_Obj method, ob_Obj receiver) {
  auto act = obctx_allocate(ctx, OBOBJ_ACTIVATION, sizeof(ob_ObjActivation));

  ob_ObjActivation *data = obobj_get_data(act);
  data->parent = ctx->this_activation;
  data->method = method;
  data->receiver = receiver;
  data->env = obctx_alloc_slots(ctx, NULL);

  ctx->this_activation = act;
}

void obctx_leave_activation(ob_Context ctx) {
  ob_ObjActivation *data = obobj_get_data(ctx->this_activation);

  // we know an activation is "live" if it's env is allocated, else it belongs
  // to a method that already returned.
  data->env = NULL;

  ctx->this_activation = data->parent;
}

void obctx_push(ob_Context ctx, ob_Obj obj) {
  obarr_push(&ctx->stack, sizeof(ob_Obj), (const void *)&obj);
}

ob_Obj obctx_pop(ob_Context ctx) {
  ob_Obj obj;

  if (!obarr_pop(&ctx->stack, sizeof(ob_Obj), (void *)&obj)) {
    ASSERT(false, "cannot pop from empty stack");
  }

  return obj;
}

bool obctx_checkstack(ob_Context ctx, size_t narg) {
  return (ctx->stack.size / sizeof(ob_Object *)) >= narg;
}

ob_Obj obctx_get_prototype(ob_Context ctx, ob_Obj obj) {
  if (obj == ctx->proto.object) {
    return NULL;
  }

  if (obj == ctx->proto.slots) {
    return ctx->proto.object;
  }

  switch (obobj_get_tag(obj)) {
  case OBOBJ_NIL:
    return ctx->proto.nil;
  case OBOBJ_SYMBOL:
    return ctx->proto.symbol;
  case OBOBJ_STRING:
    return ctx->proto.string;
  case OBOBJ_SLOTS: {
    ob_ObjSlots *slots = obobj_get_data(obj);

    if (slots->prototype != NULL) {
      return slots->prototype;
    }

    return ctx->proto.slots;
  }
  case OBOBJ_NUMBER:
    return ctx->proto.number;
  case OBOBJ_ARRAY:
    return ctx->proto.array;
  case OBOBJ_METHOD:
    return ctx->proto.method;
  case OBOBJ_LIGHTCMETHOD:
    return ctx->proto.lightcmethod;
  case OBOBJ_CMETHOD:
    return ctx->proto.cmethod;
  case OBOBJ_LIGHTCDATA:
    return ctx->proto.lightcdata;
  case OBOBJ_CDATA: {
    ob_ObjCData *data = obobj_get_data(obj);

    if (data->prototype != NULL) {
      return data->prototype;
    }

    return ctx->proto.cdata;
  }
  case OBOBJ_ACTIVATION:
    return ctx->proto.activation;
  case OT_Rc:
  case OT_Rd:
  case OT_Re:
  case OT_Rf:
    break;
  }

  return obctx_get_prototype(ctx, NULL);
}

bool obctx_get_slot(ob_Context ctx, ob_Obj *slot, ob_Obj obj, ob_Str selector) {
  while (obj != NULL) {
    if (OBOBJ_ISA(obj, OBOBJ_SLOTS)) {
      ob_ObjSlots *data = obobj_get_data(obj);

      auto str = obstr_get_data(ctx, selector);
      auto hash = obhash_start(selector->length, str);

      if (obtbl_get(&data->slots, hash, (void **)&obj)) {
        if (slot != NULL) {
          *slot = obj;
        }

        return true;
      }
    }

    if (obj != ctx->proto.object) {
      obj = obctx_get_prototype(ctx, obj);
    } else {
      break;
    }
  }

  return false;
}

static void invoke(ob_Context ctx, ob_Obj invoked, ob_Obj recv, size_t n_args) {
  auto tag = obobj_get_tag(invoked);

  switch (tag) {
  case OBOBJ_LIGHTCMETHOD: {
    ctx->gc_state.enabled = false;

    auto data = *(ob_FnCMethod *)obobj_get_data(invoked);

    if (!data(ctx)) {
      obctx_push(ctx, recv);
    }

    // TODO: check that we actually popped n args
    ctx->gc_state.enabled = true;
  }; break;

  case OBOBJ_CMETHOD: {
    ob_ObjCMethod *data = obobj_get_data(invoked);

    ASSERT(n_args == obarr_length(&data->parameters, sizeof(ob_Str)),
           "not enough arguments to invoke C method");

    ctx->gc_state.enabled = false;

    if (!data->method(ctx)) {
      obctx_push(ctx, recv);
    }

    ctx->gc_state.enabled = true;
  }; break;

  case OBOBJ_METHOD: {
    ob_ObjMethod *data = obobj_get_data(invoked);
    ob_ObjActivation *act = obobj_get_data(ctx->this_activation);
    ob_ObjSlots *env = obobj_get_data(act->env);

    size_t length = obarr_length(&data->parameters, sizeof(ob_Str));

    for (size_t i = 0; i < length; i++) {
      auto param = (ob_Str *)obarr_at(&data->parameters, sizeof(ob_Str), i);
      auto item = obctx_pop(ctx);

      obtbl_set(&env->slots, obstr_get_hash(ctx, *param), item);
    }

    obbc_run(ctx, data->bytecode.size, data->bytecode.data);
  }; break;
  default:
    ASSERT(false, "should not be able to invoke this object");
  }
}

void obctx_send(ob_Context ctx, ob_Obj recv, ob_Str selector) {
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

  ASSERT(obctx_checkstack(ctx, n_args),
         "expected to have %lu arguments on stack", n_args);

  ob_Obj invoked = NULL;

  if (!obctx_get_slot(ctx, &invoked, recv, selector)) {
    ASSERT(false, "todo doesNotUnderstand");
    // TODO: doesNotUnderstand
  }

  bool is_invocable = OBOBJ_IS_INVOCABLE(invoked);

  if (n_args != 0) {
    ASSERT(is_invocable, "tried to invoke a non-method object %p", invoked);
  }

  if (is_invocable) {
    obctx_enter_activation(ctx, invoked, recv);
    invoke(ctx, invoked, recv, n_args);
    obctx_leave_activation(ctx);
  }

  else {
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

ob_Obj obctx_get_receiver(ob_Context ctx) {
  if (ctx->this_activation == NULL) {
    return NULL;
  }

  ob_ObjActivation *act = obobj_get_data(ctx->this_activation);
  return act->receiver;
}
