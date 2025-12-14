#include <ob/Array.h>
#include <ob/Object.h>
#include <ob/Table.h>

ob_ObjectTag obobj_get_tag(ob_Obj obj) {
  if (obj == NULL) {
    return OBOBJ_NIL;
  }

  return obj->header.tag;
}

void *obobj_get_data(ob_Obj obj) {
  auto bytes = (uint8_t *)obj;
  return bytes + sizeof(ob_Object);
}

bool obobj_get_mark(ob_Obj obj) {
  if (obj == NULL) {
    return true;
  }

  return obj->header.mark;
}

static void mark(ob_Obj obj, void *unused) {
  (void)unused;
  obobj_mark(obj);
}

void obobj_mark(ob_Obj obj) {
  if (obj == NULL) {
    return;
  }

  if (obj->header.mark) {
    return;
  }

  switch (obj->header.tag) {
  case OBOBJ_STRING:
  case OBOBJ_SYMBOL: {
    ob_ObjString *str = obobj_get_data(obj);
    obstr_mark(str->inner);
  } break;

  default:
    break;
  }

  obj->header.mark = true;

  obobj_visit_before(obj, mark, NULL);
}

ob_Obj obobj_ref(ob_Obj obj) {
  if (obj->refcount < UINT32_MAX) {
    obj->refcount++;
  }

  return obj;
}

// true if rc=0
bool obobj_unref(ob_Obj obj) {
  if (obj == NULL) {
    return false;
  }

  if (obj->refcount < UINT32_MAX) {
    obj->refcount--;
  }

  return obj->refcount == 0;
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

  switch (obj->header.tag) {
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
