#include <ob/base/Array.h>
#include <ob/base/Assert.h>
#include <ob/base/Table.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>

void obslot_add(ob_Slots *slots, ob_Str key, ob_Obj value) {
  auto len = ql_array_length(&slots->data, sizeof(ob_Slot));
  size_t index = 0;

  for (; index < len; index++) {
    auto slot = (ob_Slot *)ql_array_at(&slots->data, sizeof(ob_Slot), index);

    if (slot->key == key) {
      slot->value = value;
      return;
    }
  }

  auto slot = (ob_Slot){.key = key, .value = value};
  ql_array_push(&slots->data, sizeof slot, &slot);
}

bool obslot_remove(ob_Slots *slots, ob_Str key) {
  auto len = ql_array_length(&slots->data, sizeof(ob_Slot));
  size_t index = 0;
  auto found = false;

  for (; index < len; index++) {
    auto slot = (ob_Slot *)ql_array_at(&slots->data, sizeof(ob_Slot), index);
    if (slot->key == key) {
      found = true;
      break;
    }
  }

  if (found) {
    ql_array_remove(&slots->data, sizeof(ob_Slot), index);
  }

  return found;
}

bool obslot_get(ob_Slots *slots, ob_Str key, ob_Obj *out) {
  auto len = ql_array_length(&slots->data, sizeof(ob_Slot));

  for (size_t index = 0; index < len; index++) {
    auto slot = (ob_Slot *)ql_array_at(&slots->data, sizeof(ob_Slot), index);

    if (slot->key == key) {
      if (out != nullptr) {
        *out = slot->value;
      }

      return true;
    }
  }

  return false;
}

ob_ObjectTag ob_get_tag(ob_Obj obj) {
  if (obj == NULL) {
    return OB_NIL;
  }

  return obj->header.tag;
}

void *ob_get_payload(ob_Obj obj) {
  auto bytes = (uint8_t *)obj;
  return bytes + sizeof(struct ob_Object);
}

bool obobj_get_mark(ob_Obj obj) {
  if (obj == NULL) {
    return true;
  }

  return obj->header.mark;
}

static void mark_inner(ob_Obj obj, void *unused) {
  (void)unused;

  obj->header.mark = true;

  ob_mark(obj);
}

static bool mark_pred(ob_Obj obj, void *unused) {
  (void)unused;

  return obj->header.mark;
}

void ob_mark(ob_Obj obj) {
  if (obj == NULL) {
    return;
  }

  switch (obj->header.tag) {
  case OB_STRING:
  case OB_SYMBOL: {
    auto str = (ob_Str *)ob_get_payload(obj);
    obstr_mark(*str);
  } break;

  case OB_METHOD: {
    ob_ObjMethod *method = ob_get_payload(obj);
    auto len = ql_array_length(&method->parameters, sizeof(ob_Str));

    for (size_t i = 0; i < len; i++) {
      auto str = (ob_Str *)ql_array_at(&method->parameters, sizeof(ob_Str), i);
      obstr_mark(*str);
    }
  }; break;

  case OB_CMETHOD: {
    ob_ObjCMethod *method = ob_get_payload(obj);
    auto len = ql_array_length(&method->parameters, sizeof(ob_Str));

    for (size_t i = 0; i < len; i++) {
      auto str = (ob_Str *)ql_array_at(&method->parameters, sizeof(ob_Str), i);
      obstr_mark(*str);
    }
  }; break;

  default:
    break;
  }

  ob_visit(obj, OB_VISIT_BEFORE, mark_inner, mark_pred, NULL);
}

void obobj_destroy(ob_Obj obj) {
  switch (ob_get_tag(obj)) {
  case OB_SLOTS: {
    ob_ObjSlots *data = ob_get_payload(obj);
    ql_array_free(&data->slots.data);
  } break;

  case OB_ARRAY: {
    ql_Array *data = ob_get_payload(obj);
    ql_array_free(data);
  } break;

  case OB_METHOD: {
    ob_ObjMethod *data = ob_get_payload(obj);
    ql_array_free(&data->parameters);
    ql_array_free(&data->literals);
    ql_array_free(&data->bytecode);
  } break;

  case OB_CDATA: {
    ob_ObjCData *data = ob_get_payload(obj);
    data->destroy(obj);
  };

    // TODO: implement uninterning afterwards

  default:
    break;
  }
}

void ob_visit(ob_Obj obj, ob_VisitFlags flags, ob_FnVisit visit,
              ob_FnVisitPredicate predicate, void *userdata) {
  if (obj == NULL) {
    return;
  }

  if ((predicate != NULL) && predicate(obj, userdata)) {
    return;
  }

  if (flags & OB_VISIT_BEFORE) {
    visit(obj, userdata);
  }

  switch (obj->header.tag) {
  case OB_SLOTS: {
    ob_ObjSlots *data = ob_get_payload(obj);

    auto len = ql_array_length(&data->slots.data, sizeof(ob_Slot));
    for (size_t i = 0; i < len; i++) {
      auto slot = (ob_Slot *)ql_array_at(&data->slots.data, sizeof(ob_Slot), i);
      obstr_mark(slot->key);
      ob_visit(slot->value, flags, visit, predicate, userdata);
    }

    ob_visit(data->prototype, flags, visit, predicate, userdata);
  } break;

  case OB_ARRAY: {
    ql_Array *data = ob_get_payload(obj);

    auto length = data->size / sizeof(ob_Obj);
    for (size_t i = 0; i < length; i++) {
      auto item = ql_array_at(data, sizeof(ob_Obj), i);
      ob_visit(item, flags, visit, predicate, userdata);
    }
  } break;

  case OB_METHOD: {
    ob_ObjMethod *data = ob_get_payload(obj);

    QL_ASSERT(obj != data->parent, "uhhhh");

    for (size_t i = 0; i < data->literals.size / sizeof(ob_Obj); i++) {
      ob_Obj item = ((ob_Obj *)data->literals.data)[i];

      ob_visit(item, flags, visit, predicate, userdata);
    }

    ob_visit(data->parent, flags, visit, predicate, userdata);
  }; break;

  case OB_ACTIVATION: {
    ob_ObjActivation *data = ob_get_payload(obj);
    ob_visit(data->parent, flags, visit, predicate, userdata);
    ob_visit(data->method, flags, visit, predicate, userdata);
    ob_visit(data->receiver, flags, visit, predicate, userdata);
    ob_visit(data->env, flags, visit, predicate, userdata);
  }; break;

  case OB_CDATA: {
    ob_ObjCData *data = ob_get_payload(obj);
    ob_visit(data->prototype, flags, visit, predicate, userdata);
    data->visit(obj, data->data);
  }; break;

  default:
    break;
  }

  if (flags & OB_VISIT_AFTER) {
    visit(obj, userdata);
  }
}

static void *cast(ob_Obj obj, ob_ObjectTag tag) {
  auto got = ob_get_tag(obj);
  QL_ASSERT(tag == got, "expected tag %d, got tag %d", tag, got);
  return ob_get_payload(obj);
}

ob_Str *ob_cast_symbol(ob_Obj obj) {
  return (ob_Str *)cast(obj, OB_SYMBOL);
}

ob_Str *ob_cast_string(ob_Obj obj) {
  return (ob_Str *)cast(obj, OB_STRING);
}

ob_ObjSlots *ob_cast_slots(ob_Obj obj) {
  return cast(obj, OB_SLOTS);
}

ql_Number *ob_cast_number(ob_Obj obj) {
  return cast(obj, OB_NUMBER);
}

ql_Array *ob_cast_array(ob_Obj obj) {
  return cast(obj, OB_ARRAY);
}

ob_ObjMethod *ob_cast_method(ob_Obj obj) {
  return cast(obj, OB_METHOD);
}

ob_FnCMethod *ob_cast_lightcmethod(ob_Obj obj) {
  return (ob_FnCMethod *)cast(obj, OB_LIGHTCMETHOD);
}

ob_ObjCMethod *ob_cast_cmethod(ob_Obj obj) {
  return cast(obj, OB_CMETHOD);
}

ob_ObjCData *ob_cast_cdata(ob_Obj obj) {
  return cast(obj, OB_CDATA);
}
ob_ObjActivation *ob_cast_activation(ob_Obj obj) {
  return cast(obj, OB_ACTIVATION);
}
