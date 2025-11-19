#include "Object.h"

#include "Context.h"
#include "Table.h"

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
