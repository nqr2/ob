#include "Object.h"

#include "Bytecode.h"
#include "Context.h"
#include "Hash.h"
#include "Table.h"

#include <ctype.h>

void *obj_payload(Obj obj) {
  auto bytes = (uint8_t *)obj;
  return bytes + sizeof(Object);
}

void obj_mark(Obj obj) {
  if (HEADER_GET_MARK(obj->header)) {
    return;
  }

  switch (HEADER_GET_TAG(obj->header)) {
  case OT_STRING:
  case OT_SYMBOL: {
    struct {
      String *inner;
    } *str = obj_payload(obj);
    str_mark(str->inner);
  } break;

  default:
    break;
  }

  obj->header = HEADER_SET_MARK(obj->header, true);

  obj_visit(obj, obj_mark);
}

Obj obj_ref(Obj obj) {
  auto refcount = HEADER_GET_RC(obj->header);

  if (refcount < RC_MAX) {
    refcount++;
    obj->header = HEADER_SET_RC(obj->header, refcount);
  }

  return obj;
}

// true if rc=0
bool obj_unref(Obj obj) {
  auto refcount = HEADER_GET_RC(obj->header);

  if (refcount < RC_MAX) {
    refcount--;
    obj->header = HEADER_SET_RC(obj->header, refcount);
  }

  return refcount == 0;
}

Obj obj_create(Context ctx, size_t payload_size) {
  auto obj = (Obj)allocate(ctx->allocator, sizeof(Object) + payload_size);

  obj->header = 0;
  obj->next = ctx->objects;

  ctx->objects = obj;

  return obj;
}

void obj_push(Context ctx, Obj obj) {
  arr_push(&ctx->stack, sizeof(Obj), (const void *)&obj);
}

Obj obj_pop(Context ctx) {
  Obj obj;

  if (!arr_pop(&ctx->stack, sizeof(Obj), (void *)&obj)) {
    // TODO: fail? cannot pop empty stack.
  }

  return obj;
}

static void obj__unref_(Obj obj) {
  (void)obj_unref(obj);
}

void obj_destroy(Object *obj) {
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

Obj obj_create_slots(Context ctx, Obj prototype) {
  Obj obj = obj_create(ctx, sizeof(ObjSlots));
  obj->header = HEADER_SET_TAG(0, OT_SLOTS);

  ObjSlots *slots = obj_payload(obj);

  tbl_init(&slots->slots, ctx->allocator);
  slots->prototype = prototype;

  return obj;
}

Object *obj_create_cmethod(Context ctx, FnCMethod method) {
  Object *obj = obj_create(ctx, sizeof(ObjCMethod));
  obj->header = HEADER_SET_TAG(0, OT_CMETHOD);

  ObjCMethod *data = obj_payload(obj);
  data->method = method;

  return obj;
}

ObjectTag obj_tag(Obj obj) {
  if (obj == NULL) {
    return OT_NIL;
  }

  return HEADER_GET_TAG(obj->header);
}

bool obj_isa(Obj obj, ObjectTag tag) {
  return obj_tag(obj) == tag;
}

Obj obj_getproto(Context ctx, Obj obj) {
  if (obj == ctx->proto_nil) {
    return NULL;
  }

  switch (obj_tag(obj)) {
  case OT_NIL:
    return ctx->proto_nil;
  case OT_SYMBOL:
    return ctx->proto_symbol;
  case OT_STRING:
    return ctx->proto_string;
  case OT_SLOTS: {
    ObjSlots *slots = obj_payload(obj);

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
  return obj_getproto(ctx, NULL);
}

Object *obj_get(Context ctx, Object *obj, String *selector) {
  if (obj == NULL) {
    return NULL;
  }

  if (obj_isa(obj, OT_SLOTS)) {
    ObjSlots *data = obj_payload(obj);
    (void)data;

    auto hash = hash_start(selector->length, selector->data);

    if (tbl_get(&data->slots, hash, (void **)&obj)) {
      return obj;
    }
  }

  return obj_get(ctx, obj_getproto(ctx, obj), selector);
}

void obj_send(Context ctx, Object *recv, String *selector) {
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

  (void)/*TODO: assert its true*/ ctx_checkstack(ctx, n_args);

  auto invoked = obj_get(ctx, recv, selector);

  (void)/*TODO: also this assert*/ obj_is_invocable(invoked);

  ctx_enter_activation(ctx, ctx->activation, invoked, recv);

  auto tag = HEADER_GET_TAG(invoked->header);

  if (tag == OT_CMETHOD) {
    ObjCMethod *data = obj_payload(invoked);
    data->method(ctx);
  }

  if (tag == OT_METHOD) {
    ObjMethod *data = obj_payload(invoked);
    bc_run(ctx, data->bytecode.size, data->bytecode.data);
  }

  // TODO: call closure

  ctx_leave_activation(ctx);
}

bool obj_is_invocable(Object *obj) {
  auto tag = HEADER_GET_TAG(obj->header);
  return ((tag == OT_METHOD) || (tag == OT_CMETHOD)) != 0;
}
