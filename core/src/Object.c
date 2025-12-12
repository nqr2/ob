#include <ob/Array.h>
#include <ob/Object.h>
#include <ob/Table.h>

#define HEADER_GET_TAG(H) ((ob_ObjectHeader)((H) & 0xf))
#define HEADER_SET_TAG(H, T) ((ob_ObjectHeader)(((H) & 0xfff0) | ((T) & 0xf)))

#define HEADER_GET_MARK(H) (((H) & 0x10) != 0)
#define HEADER_SET_MARK(H, M)                                                  \
  ((ob_ObjectHeader)(((H) & 0xffef) | (((M) != 0) << 4)))

#define HEADER_GET_RC(H) ((H) >> 5)
#define HEADER_SET_RC(H, C) (((H) & 0x1f) | ((C) << 5))

#define RC_MAX 0x7ff

ob_ObjectTag obobj_get_tag(ob_Obj obj) {
  if (obj == NULL) {
    return OBOBJ_NIL;
  }

  return HEADER_GET_TAG(obj->header);
}

void *obobj_get_data(ob_Obj obj) {
  auto bytes = (uint8_t *)obj;
  return bytes + sizeof(ob_Object);
}

bool obobj_get_mark(ob_Obj obj) {
  if (obj == NULL) {
    return true;
  }

  return HEADER_GET_MARK(obj->header);
}

static void mark(ob_Obj obj, void *unused) {
  (void)unused;
  obobj_mark(obj);
}

void obobj_mark(ob_Obj obj) {
  if (HEADER_GET_MARK(obj->header)) {
    return;
  }

  switch (HEADER_GET_TAG(obj->header)) {
  case OBOBJ_STRING:
  case OBOBJ_SYMBOL: {
    ob_ObjString *str = obobj_get_data(obj);
    obstr_mark(str->inner);
  } break;

  default:
    break;
  }

  obj->header = HEADER_SET_MARK(obj->header, true);

  obobj_visit_before(obj, mark, NULL);
}

ob_Obj obobj_ref(ob_Obj obj) {
  auto refcount = HEADER_GET_RC(obj->header);

  if (refcount < RC_MAX) {
    refcount++;
    obj->header = HEADER_SET_RC(obj->header, refcount);
  }

  return obj;
}

// true if rc=0
bool obobj_unref(ob_Obj obj) {
  if (obj == NULL) {
    return false;
  }

  auto refcount = HEADER_GET_RC(obj->header);

  if (refcount < RC_MAX) {
    refcount--;
    obj->header = HEADER_SET_RC(obj->header, refcount);
  }

  return refcount == 0;
}

void obobj_destroy(ob_Obj obj) {
  switch (obobj_get_tag(obj)) {
  case OBOBJ_SLOTS: {
    ob_ObjSlots *data = obobj_get_data(obj);
    obtbl_free(&data->slots);
  } break;

  case OBOBJ_ARRAY: {
    ob_ObjArray *data = obobj_get_data(obj);
    obarr_free(&data->items);
  } break;

  case OBOBJ_METHOD: {
    ob_ObjMethod *data = obobj_get_data(obj);
    obarr_free(&data->parameters);
    obarr_free(&data->literals);
    obarr_free(&data->bytecode);
  } break;

    // TODO: implement uninterning afterwards

  default:
    break;
  }
}

void obobj_visit(ob_Object *obj, ob_VisitFlags flags, ob_FnVisit visit,
                 void *userdata) {
  // TODO: properly handle NULLs.
  if (obj == NULL) {
    return;
  }

  if (flags & VISIT_BEFORE) {
    visit(obj, userdata);
  }

  switch (HEADER_GET_TAG(obj->header)) {
  case OBOBJ_SLOTS: {
    ob_ObjSlots *data = obobj_get_data(obj);

    ob_Object *ref = NULL;
    uint64_t index = 0;

    while (obtbl_iterate(&data->slots, &index, NULL, (void **)&ref)) {
      obobj_visit(ref, flags, visit, userdata);
    }

    obobj_visit(data->prototype, flags, visit, userdata);
  } break;

  case OBOBJ_ARRAY: {
    ob_ObjArray *data = obobj_get_data(obj);

    auto length = data->items.size / sizeof(ob_Obj);
    for (size_t i = 0; i < length; i++) {
      obobj_visit(obarr_at(&data->items, sizeof(ob_Obj), i), flags, visit,
                  userdata);
    }
  } break;

  case OBOBJ_METHOD: {
    ob_ObjMethod *data = obobj_get_data(obj);

    for (size_t i = 0; i < data->literals.size / sizeof(ob_Object *); i++) {
      ob_Object *item = ((ob_Object **)data->literals.data)[i];

      obobj_visit(item, flags, visit, userdata);
    }

    obobj_visit(data->env, flags, visit, userdata);
  }; break;

  case OBOBJ_ACTIVATION: {
    ob_ObjActivation *data = obobj_get_data(obj);
    obobj_visit(data->parent, flags, visit, userdata);
    obobj_visit(data->caller, flags, visit, userdata);
    obobj_visit(data->method, flags, visit, userdata);
    obobj_visit(data->receiver, flags, visit, userdata);
    obobj_visit(data->env, flags, visit, userdata);
  }; break;

  default:
    break;
  }

  if (flags & VISIT_AFTER) {
    visit(obj, userdata);
  }
}
