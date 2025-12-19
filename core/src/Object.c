#include <ob/Array.h>
#include <ob/Object.h>
#include <ob/String.h>
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

static void mark_inner(ob_Obj obj, void *unused) {
  (void)unused;

  obj->header.mark = true;

  obobj_mark(obj);
}

static bool mark_pred(ob_Obj obj, void *unused) {
  (void)unused;

  return obj->header.mark;
}

void obobj_mark(ob_Obj obj) {
  if (obj == NULL) {
    return;
  }

  switch (obj->header.tag) {
  case OBOBJ_STRING:
  case OBOBJ_SYMBOL: {
    auto str = (ob_Str *)obobj_get_data(obj);
    obstr_mark(*str);
  } break;

  case OBOBJ_METHOD: {
    ob_ObjMethod *method = obobj_get_data(obj);
    auto len = obarr_length(&method->parameters, sizeof(ob_Str));

    for (size_t i = 0; i < len; i++) {
      auto str = (ob_Str *)obarr_at(&method->parameters, sizeof(ob_Str), i);
      obstr_mark(*str);
    }
  };

  case OBOBJ_CMETHOD: {
    ob_ObjCMethod *method = obobj_get_data(obj);
    auto len = obarr_length(&method->parameters, sizeof(ob_Str));

    for (size_t i = 0; i < len; i++) {
      auto str = (ob_Str *)obarr_at(&method->parameters, sizeof(ob_Str), i);
      obstr_mark(*str);
    }
  };

  default:
    break;
  }

  obobj_visit(obj, VISIT_AFTER, mark_inner, mark_pred, NULL);
}

void obobj_destroy(ob_Obj obj) {
  switch (obobj_get_tag(obj)) {
  case OBOBJ_SLOTS: {
    ob_ObjSlots *data = obobj_get_data(obj);
    obtbl_free(&data->slots);
  } break;

  case OBOBJ_ARRAY: {
    ob_Array *data = obobj_get_data(obj);
    obarr_free(data);
  } break;

  case OBOBJ_METHOD: {
    ob_ObjMethod *data = obobj_get_data(obj);
    obarr_free(&data->parameters);
    obarr_free(&data->literals);
    obarr_free(&data->bytecode);
  } break;

  case OBOBJ_CDATA: {
    ob_ObjCData *data = obobj_get_data(obj);
    data->destroy(obj);
  };

    // TODO: implement uninterning afterwards

  default:
    break;
  }
}

void obobj_visit(ob_Object *obj, ob_VisitFlags flags, ob_FnVisit visit,
                 ob_FnVisitPredicate predicate, void *userdata) {
  if (obj == NULL) {
    return;
  }

  if ((predicate != NULL) && predicate(obj, userdata)) {
    return;
  }

  if (flags & VISIT_BEFORE) {
    visit(obj, userdata);
  }

  switch (obj->header.tag) {
  case OBOBJ_SLOTS: {
    ob_ObjSlots *data = obobj_get_data(obj);

    ob_Obj ref = NULL;
    uint64_t index = 0;

    while (obtbl_iterate(&data->slots, &index, NULL, (void **)&ref)) {
      obobj_visit(ref, flags, visit, predicate, userdata);
    }

    obobj_visit(data->prototype, flags, visit, predicate, userdata);
  } break;

  case OBOBJ_ARRAY: {
    ob_Array *data = obobj_get_data(obj);

    auto length = data->size / sizeof(ob_Obj);
    for (size_t i = 0; i < length; i++) {
      auto item = obarr_at(data, sizeof(ob_Obj), i);
      obobj_visit(item, flags, visit, predicate, userdata);
    }
  } break;

  case OBOBJ_METHOD: {
    ob_ObjMethod *data = obobj_get_data(obj);

    for (size_t i = 0; i < data->literals.size / sizeof(ob_Obj); i++) {
      ob_Obj item = ((ob_Obj *)data->literals.data)[i];

      obobj_visit(item, flags, visit, predicate, userdata);
    }

    obobj_visit(data->env, flags, visit, predicate, userdata);
  }; break;

  case OBOBJ_ACTIVATION: {
    ob_ObjActivation *data = obobj_get_data(obj);
    obobj_visit(data->parent, flags, visit, predicate, userdata);
    obobj_visit(data->method, flags, visit, predicate, userdata);
    obobj_visit(data->receiver, flags, visit, predicate, userdata);
    obobj_visit(data->env, flags, visit, predicate, userdata);
  }; break;

  case OBOBJ_CDATA: {
    ob_ObjCData *data = obobj_get_data(obj);
    obobj_visit(data->prototype, flags, visit, predicate, userdata);
    data->visit(obj, data->data);
  }; break;

  default:
    break;
  }

  if (flags & VISIT_AFTER) {
    visit(obj, userdata);
  }
}
