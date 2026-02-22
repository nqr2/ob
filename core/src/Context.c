#include <ob/base/Allocator.h>
#include <ob/base/Array.h>
#include <ob/base/Assert.h>
#include <ob/base/Exn.h>
#include <ob/base/Hash.h>
#include <ob/base/Log.h>
#include <ob/base/Number.h>
#include <ob/base/Table.h>
#include <ob/core/Bytecode.h>
#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>

#include <ctype.h>
#include <string.h>

#define DEFAULT_GC_FACTOR 1.5f

static void gc_sweep(ob_Ctx ctx);

ob_Ctx ob_create(ql_Allocator *alloc) {
  ob_Ctx ctx = ql_allocate(alloc, sizeof(struct ob_Context));

  ctx->gc_state.factor = DEFAULT_GC_FACTOR;
  ctx->gc_state.enabled = true;

  ql_array_init(&ctx->stack, alloc);
  ql_array_init(&ctx->string_data, alloc);
  ql_array_init(&ctx->string_available, alloc);

  ctx->allocator = alloc;

  ctx->proto.object = ob_create_slots(ctx, NULL);

  ctx->proto.nil = ob_create_slots(ctx, NULL);
  ctx->proto.symbol = ob_create_slots(ctx, NULL);
  ctx->proto.string = ob_create_slots(ctx, NULL);
  ctx->proto.slots = ob_create_slots(ctx, NULL);
  ctx->proto.number = ob_create_slots(ctx, NULL);
  ctx->proto.array = ob_create_slots(ctx, NULL);
  ctx->proto.method = ob_create_slots(ctx, NULL);
  ctx->proto.lightcmethod = ob_create_slots(ctx, ctx->proto.method);
  ctx->proto.cmethod = ob_create_slots(ctx, ctx->proto.method);
  ctx->proto.lightcdata = ob_create_slots(ctx, NULL);
  ctx->proto.cdata = ob_create_slots(ctx, NULL);
  ctx->proto.activation = ob_create_slots(ctx, NULL);

  ctx->known.shell = ob_create_slots(ctx, NULL);
  ctx->known.o_true = ob_create_slots(ctx, NULL);
  ctx->known.o_false = ob_create_slots(ctx, NULL);

  ql_table_init(&ctx->interned, ctx->allocator);

  ql_exn_init(&ctx->exnbuf, ctx->allocator);

  ctx->gc_state.previous_hs = ctx->allocator->used;

  return ctx;
}

void ob_destroy(ob_Ctx ctx) {
  auto alloc = ctx->allocator;

  gc_sweep(ctx);
  gc_sweep(ctx);

  ql_array_free(&ctx->stack);
  ql_array_free(&ctx->string_data);
  ql_array_free(&ctx->string_available);

  ql_table_free(&ctx->interned);

  ql_exn_free(&ctx->exnbuf);

  ql_deallocate(alloc, sizeof(struct ob_Context), ctx);
}

ob_Obj obctx_allocate(ob_Ctx ctx, ob_ObjectTag tag, size_t payload_size) {
  auto obj = (ob_Obj)ql_allocate(ctx->allocator,
                                 sizeof(struct ob_Object) + payload_size);

  obj->next = ctx->objects;
  obj->size = payload_size;
  obj->header.tag = tag;

  ctx->objects = obj;

  return obj;
}

ob_Obj ob_create_symbol(ob_Ctx ctx, ob_Str symbol) {
  auto data = obstr_get_data(ctx, symbol);
  auto len = obstr_get_length(symbol);

  auto hash = ql_hash_start(len, data);

  ob_Obj obj = NULL;

  if (ql_table_get(&ctx->interned, hash, (void **)&obj)) {
    return obj;
  }

  obj = obctx_allocate(ctx, OB_SYMBOL, sizeof(ob_Str));

  *ob_cast_symbol(obj) = symbol;
  ql_table_set(&ctx->interned, hash, (void *)obj);

  return obj;
}

ob_Obj ob_create_string(ob_Ctx ctx, ob_Str string) {
  auto obj = obctx_allocate(ctx, OB_STRING, sizeof(ob_Str));

  auto str = (ob_Str *)ob_get_payload(obj);
  *str = string;

  return obj;
}

ob_Obj ob_create_slots(ob_Ctx ctx, ob_Obj prototype) {
  auto obj = obctx_allocate(ctx, OB_SLOTS, sizeof(ob_ObjSlots));

  ob_ObjSlots *slots = ob_get_payload(obj);

  ql_table_init(&slots->slots, ctx->allocator);
  slots->prototype = prototype;

  return obj;
}

ob_Obj ob_create_number(ob_Ctx ctx, ql_Number number) {
  auto obj = obctx_allocate(ctx, OB_NUMBER, sizeof(ql_Number));

  ql_Number *num = ob_get_payload(obj);
  *num = number;

  return obj;
}

ob_Obj ob_create_integer(ob_Ctx ctx, int64_t number) {
  return ob_create_number(ctx, ql_number_of_int(number));
}

ob_Obj ob_create_real(ob_Ctx ctx, double number) {
  return ob_create_number(ctx, ql_number_of_float(number));
}

ob_Obj ob_create_array(ob_Ctx ctx) {
  auto obj = obctx_allocate(ctx, OB_ARRAY, sizeof(ql_Array));

  ql_Array *arr = ob_get_payload(obj);
  ql_array_init(arr, ctx->allocator);

  return obj;
}

ob_Obj ob_create_method(ob_Ctx ctx) {
  auto obj = obctx_allocate(ctx, OB_METHOD, sizeof(ob_ObjMethod));

  ob_ObjMethod *method = ob_get_payload(obj);

  method->parent = nullptr;

  ql_array_init(&method->bytecode, ctx->allocator);
  ql_array_init(&method->literals, ctx->allocator);
  ql_array_init(&method->parameters, ctx->allocator);

  return obj;
}

ob_Obj ob_create_lightcmethod(ob_Ctx ctx, ob_FnCMethod method) {
  auto obj = obctx_allocate(ctx, OB_LIGHTCMETHOD, sizeof(ob_FnCMethod));

  auto data = (ob_FnCMethod *)ob_get_payload(obj);
  *data = method;

  return obj;
}

ob_Obj ob_create_lightcdata(ob_Ctx ctx, void *cdata) {
  auto obj = obctx_allocate(ctx, OB_LIGHTCDATA, sizeof(void *));

  auto data = (void **)ob_get_payload(obj);
  *data = cdata;

  return obj;
}

ob_Obj ob_create_cdata(ob_Ctx ctx, ob_Obj prototype, ob_FnVisit visit,
                       ob_FnDestroy destructor, void *data) {
  auto obj = obctx_allocate(ctx, OB_CDATA, sizeof(ob_ObjCData));

  ob_ObjCData *cdata = ob_get_payload(obj);
  cdata->prototype = prototype;
  cdata->visit = visit;
  cdata->destroy = destructor;
  cdata->data = data;

  return obj;
}

static void deallocate(ob_Ctx ctx, ob_Obj object) {
  auto size = sizeof(struct ob_Object) + object->size;

  obobj_destroy(object);
  ql_deallocate(ctx->allocator, size, object);
}

static void gc_mark(ob_Ctx ctx) {
  ob_mark(ctx->proto.object);

  ob_mark(ctx->proto.nil);
  ob_mark(ctx->proto.symbol);
  ob_mark(ctx->proto.string);
  ob_mark(ctx->proto.slots);
  ob_mark(ctx->proto.number);
  ob_mark(ctx->proto.array);
  ob_mark(ctx->proto.method);
  ob_mark(ctx->proto.lightcmethod);
  ob_mark(ctx->proto.cmethod);
  ob_mark(ctx->proto.lightcdata);
  ob_mark(ctx->proto.cdata);
  ob_mark(ctx->proto.activation);

  ob_mark(ctx->known.shell);
  ob_mark(ctx->known.o_false);
  ob_mark(ctx->known.o_true);

  ob_mark(ctx->this_activation);

  auto data = (ob_Obj *)ctx->stack.data;

  for (size_t i = 0; i < ctx->stack.size / sizeof(ob_Obj); i++) {
    ob_mark(data[i]);
  }

  uint64_t index = 0;
  ob_Obj obj = NULL;

  while (ql_table_iterate(&ctx->interned, &index, NULL, (void **)&obj)) {
    ob_mark(obj);
  }
}

// TODO:undebug
static void gc_sweep(ob_Ctx ctx) {
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

void ob_gc(ob_Ctx ctx, bool force) {
  if (force || ob_should_gc(ctx)) {
    gc_mark(ctx);
    gc_sweep(ctx);

    ctx->gc_state.previous_hs = ctx->allocator->used;
  }
}

bool ob_should_gc(ob_Ctx ctx) {
  if (!ctx->gc_state.enabled) {
    return false;
  }

  auto max_hs =
      (size_t)((float)ctx->gc_state.previous_hs * ctx->gc_state.factor);

  return ctx->allocator->used > max_hs;
}

void obctx_enter_activation(ob_Ctx ctx, ob_Obj method, ob_Obj receiver) {
  auto act = obctx_allocate(ctx, OB_ACTIVATION, sizeof(ob_ObjActivation));

  ob_ObjActivation *data = ob_get_payload(act);
  data->path = "*unknown*";
  data->line = 0;
  data->column = 0;

  data->parent = ctx->this_activation;
  data->method = method;
  data->receiver = receiver;
  data->env = ob_create_slots(ctx, nullptr);

  ctx->this_activation = act;
}

void obctx_leave_activation(ob_Ctx ctx) {
  ob_ObjActivation *data = ob_get_payload(ctx->this_activation);

  // we know an activation is "live" if it's env is allocated, else it belongs
  // to a method that already returned.
  data->env = NULL;

  ctx->this_activation = data->parent;
}

void ob_push(ob_Ctx ctx, ob_Obj obj) {
  ql_array_push(&ctx->stack, sizeof(ob_Obj), (void const *)&obj);
}

ob_Obj ob_pop(ob_Ctx ctx) {
  ob_Obj obj;

  if (!ql_array_pop(&ctx->stack, sizeof(ob_Obj), (void *)&obj)) {
    QL_ASSERT(false, "cannot pop from empty stack");
  }

  return obj;
}

bool ob_checkstack(ob_Ctx ctx, size_t narg) {
  return (ctx->stack.size / sizeof(ob_Obj)) >= narg;
}

ob_Obj ob_get_prototype(ob_Ctx ctx, ob_Obj obj) {
  if (obj == ctx->proto.object) {
    return NULL;
  }

  if (obj == ctx->proto.slots) {
    return ctx->proto.object;
  }

  switch (ob_get_tag(obj)) {
  case OB_NIL:
    return ctx->proto.nil;
  case OB_SYMBOL:
    return ctx->proto.symbol;
  case OB_STRING:
    return ctx->proto.string;
  case OB_SLOTS: {
    ob_ObjSlots *slots = ob_get_payload(obj);

    if (slots->prototype != NULL) {
      return slots->prototype;
    }

    return ctx->proto.slots;
  }
  case OB_NUMBER:
    return ctx->proto.number;
  case OB_ARRAY:
    return ctx->proto.array;
  case OB_METHOD:
    return ctx->proto.method;
  case OB_LIGHTCMETHOD:
    return ctx->proto.lightcmethod;
  case OB_CMETHOD:
    return ctx->proto.cmethod;
  case OB_LIGHTCDATA:
    return ctx->proto.lightcdata;
  case OB_CDATA: {
    ob_ObjCData *data = ob_get_payload(obj);

    if (data->prototype != NULL) {
      return data->prototype;
    }

    return ctx->proto.cdata;
  }
  case OB_ACTIVATION:
    return ctx->proto.activation;
  case OB_RESERVED_c:
  case OB_RESERVED_d:
  case OB_RESERVED_e:
  case OB_RESERVED_f:
    break;
  }

  return ob_get_prototype(ctx, NULL);
}

bool ob_get_slot(ob_Ctx ctx, ob_Obj *slot, ob_Obj obj, ob_Str selector) {
  if (obj == nullptr) {
    obj = ctx->proto.nil;
  }

  while ((obj != nullptr) && (obj != ctx->proto.object)) {
    if (OB_ISA(obj, OB_SLOTS)) {
      auto data = ob_cast_slots(obj);

      auto str = obstr_get_data(ctx, selector);
      auto hash = ql_hash_start(selector->length, str);

      if (ql_table_get(&data->slots, hash, (void **)slot)) {
        return true;
      }
    }

    if (obj != ctx->proto.object) {
      obj = ob_get_prototype(ctx, obj);
    }
  }

  if (obj == ctx->proto.object) {
    auto data = ob_cast_slots(obj);

    auto str = obstr_get_data(ctx, selector);
    auto hash = ql_hash_start(selector->length, str);

    return ql_table_get(&data->slots, hash, (void **)slot);
  }

  return false;
}

void invoke(ob_Ctx ctx, ob_Obj invoked, ob_Obj recv, size_t n_args_passed) {
  auto tag = ob_get_tag(invoked);

  auto activation = ob_cast_activation(ctx->this_activation);
  auto previous = activation->receiver;

  activation->receiver = recv;

  switch (tag) {
  case OB_LIGHTCMETHOD: {
    ctx->gc_state.enabled = false;

    auto data = *(ob_FnCMethod *)ob_get_payload(invoked);

    if (!data(ctx)) {
      ob_push(ctx, recv);
    }

    // TODO: check that we actually popped n args
    ctx->gc_state.enabled = true;
  }; break;

  case OB_CMETHOD: {
    ob_ObjCMethod *data = ob_get_payload(invoked);

    QL_ASSERT(n_args_passed ==
                  ql_array_length(&data->parameters, sizeof(ob_Str)),
              "not enough arguments to invoke C method");

    ctx->gc_state.enabled = false;

    if (!data->method(ctx)) {
      ob_push(ctx, recv);
    }

    ctx->gc_state.enabled = true;
  }; break;

  case OB_METHOD: {
    auto data = ob_cast_method(invoked);
    auto env = ob_cast_slots(activation->env);

    size_t n_parameters = ql_array_length(&data->parameters, sizeof(ob_Str));
    size_t n_remaining = 0;

    QL_INFO("n_parameters is %zu, n_arguments is %zu", n_parameters,
            n_args_passed);

    QL_ASSERT(n_args_passed >= n_parameters,
              "did not provide enough arguments to method");

    if (n_args_passed > n_parameters) {
      n_remaining = n_args_passed - n_parameters;
      n_args_passed = n_parameters;
    }

    for (size_t i = 0; i < n_args_passed; i++) {
      auto param = *(ob_Str *)ql_array_at(&data->parameters, sizeof(ob_Str),
                                          n_parameters - i - 1);
      auto item = ob_pop(ctx);

      QL_DEBUG("set env [%.*s] = %p", obstr_get_length(param),
               obstr_get_data(ctx, param), item);

      ql_table_set(&env->slots, obstr_get_hash(ctx, param), item);
    }

    while (n_remaining-- > 0) {
      (void)ob_pop(ctx);
    }

    obbc_run(ctx, data->bytecode.size, data->bytecode.data);
  }; break;
  default:
    QL_ASSERT(false, "should not be able to invoke this object");
  }

  activation->receiver = previous;
}

static size_t args_for_sel(size_t len, char const *data) {
  size_t n_args = 0;

  if (ispunct(data[0])) {
    n_args = 1;
  } else {
    for (size_t i = 0; i < len; i++) {
      if (data[i] == ':') {
        n_args++;
      }
    }
  }

  return n_args;
}

void ob_send_ext(ob_Ctx ctx, ob_Obj recv, ob_Str selector, ob_SendFlags flags) {
  auto sel = obstr_get_data(ctx, selector);
  auto len = obstr_get_length(selector);

  auto n_args = args_for_sel(len, sel);

  QL_ASSERT(ob_checkstack(ctx, n_args),
            "expected to have %lu arguments on stack", n_args);

  QL_DEBUG("in: #<%p> send %zu args to: #'%.*s'", recv, n_args, len, sel);

  ob_Obj invoked = NULL;

  if (!ob_get_slot(ctx, &invoked, recv, selector)) {
    QL_DEBUG("missing: %.*s", len, sel);

    if (flags & OB_SEND_CMW) {
      auto cmw = obstr_create_literal(ctx, "callMissing:with:");

      if (!ob_get_slot(ctx, &invoked, recv, cmw)) {
        QL_ASSERT(
            false,
            "#<%p> (%d) missing method: #'%.*s' and could not call missing",
            recv, ob_get_tag(recv), len, sel);
      }

      QL_DEBUG("cmw returned %p", invoked);

      auto args = ob_create_array(ctx);
      auto args_data = ob_cast_array(args);

      auto len = ql_array_length(&ctx->stack, sizeof(ob_Obj));

      if (n_args > 0) {
        ql_array_push(
            args_data, n_args * sizeof(ob_Obj),
            ql_array_at(&ctx->stack, sizeof(ob_Obj), len - n_args - 1));
      }

      ob_push(ctx, args);
      ob_push(ctx, ob_create_symbol(ctx, selector));

      n_args = 2;
    } else {
      QL_ASSERT(false, "#<%p> (%d) missing method: #'%.*s'", recv,
                ob_get_tag(recv), len, sel);
    }
  }

  bool is_invocable = OB_IS_INVOCABLE(invoked);

  if (n_args != 0) {
    if (ob_get_tag(invoked) == OB_STRING) {
      auto is = *ob_cast_string(invoked);
      QL_ERROR("tried to invoke: '%.*s'", obstr_get_length(is),
               obstr_get_data(ctx, is));
    }

    QL_ASSERT(is_invocable, "tried to invoke a non-method object %p (%d)",
              invoked, ob_get_tag(invoked));
  }

  if (is_invocable) {
    obctx_enter_activation(ctx, invoked, recv);
    invoke(ctx, invoked, recv, n_args);
    obctx_leave_activation(ctx);
  }

  else {
    ob_push(ctx, invoked);
  }
}

void ob_send(ob_Ctx ctx, ob_Obj recv, ob_Str selector) {
  ob_send_ext(ctx, recv, selector, OB_SEND_CMW);
}

ql_Exncode ob_pcall(ob_Ctx ctx, void (*inner)(ob_Ctx ctx, void *userdata),
                    void *userdata) {
  QL_EXN_BEGIN(&ctx->exnbuf, {
    auto code = ql_exn_get_code(&ctx->exnbuf);
    ql_exn__end(&ctx->exnbuf);
    return code;
  });

  inner(ctx, userdata);

  QL_EXN_END(&ctx->exnbuf);

  return OB_OK;
}

ob_Obj ob_get_receiver(ob_Ctx ctx) {
  if (ctx->this_activation == NULL) {
    return NULL;
  }

  ob_ObjActivation *act = ob_get_payload(ctx->this_activation);
  return act->receiver;
}
