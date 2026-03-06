#include <ob/base/Array.h>
#include <ob/bits/AddMethods.h>
#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>
#include <ob/lib/Object.h>

#include <stdio.h>
#include <string.h>

void obj_print(ob_Ctx ctx, ob_Obj receiver) {
  auto tag = ob_get_tag(receiver);
  switch (tag) {
  case OB_NIL:
    printf("nil");
    break;

  case OB_SYMBOL: {
    auto str = ob_cast_symbol(receiver);
    printf("#'%.*s'", (int)obstr_get_length(*str), obstr_get_data(ctx, *str));
  } break;

  case OB_STRING: {
    auto str = ob_cast_string(receiver);
    printf("'%.*s'", (int)obstr_get_length(*str), obstr_get_data(ctx, *str));
  } break;

  case OB_SLOTS:
    printf("#<slots:%p>", (void *)receiver);
    break;

  case OB_NUMBER: {
    auto num = ob_cast_number(receiver);

    if (ql_number_is_int(*num)) {
      printf("%ld", ql_number_to_int(*num));
    } else {
      printf("%f", ql_number_to_float(*num));
    }
  }; break;

    // NUMBER

  case OB_ARRAY: {
    printf("[");

    auto arr = ob_cast_array(receiver);
    auto size = arr->size / sizeof(ob_Obj);

    auto items = (ob_Obj *)arr->data;

    for (size_t i = 0; i < size; i++) {
      obj_print(ctx, items[i]);

      if (i != size - 1) {
        printf(" . ");
      }
    }

    printf("]");
  }; break;

  case OB_METHOD:
    printf("#<method:%p>", (void *)receiver);
    break;

  case OB_LIGHTCMETHOD: {
    auto method = ob_cast_lightcmethod(receiver);
    void *ptr = NULL;
    memcpy((void *)&ptr, (void *)method, sizeof(void *));
    printf("#<cmethod:%p>", ptr);
  } break;

  case OB_CDATA: {
    auto data = ob_cast_cdata(receiver);
    printf("#<cdata:%p>", data);
  } break;

  case OB_ACTIVATION:
    printf("#<activation:%p>", (void *)receiver);
    break;

  default:
    printf("#<r%d:%p>", tag, (void *)receiver);
    break;
  }
}

bool o__print(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  obj_print(ctx, receiver);

  putchar('\n');

  return false;
}

static bool o__share(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto operand = ob_pop(ctx);

  auto result = OB_BOOL_CAST(ctx, (receiver == operand));
  ob_push(ctx, result);

  return true;
}

static bool o__prototype(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  ob_push(ctx, ob_get_prototype(ctx, receiver));
  return true;
}

static bool o__self(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  ob_push(ctx, receiver);
  return true;
}

static bool o__send(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto selector = ob_pop(ctx);
  auto args = ob_pop(ctx);

  auto sel = *ob_cast_string(selector);
  auto args_data = ob_cast_array(args);

  while (args_data->size > 0) {
    ob_Obj item = NULL;
    ql_array_pop(args_data, sizeof(ob_Obj), (void *)&item);
    ob_push(ctx, item);
  }

  ob_send(ctx, receiver, sel);

  return true;
}
static bool o__has_method(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto selector = ob_pop(ctx);

  auto sel = *ob_cast_symbol(selector);

  bool got = ob_get_slot(ctx, NULL, receiver, sel);

  ob_push(ctx, OB_BOOL_CAST(ctx, got));
  return true;
}

void oblib_load_object(ob_Ctx ctx) {
  ob_add_methods(ctx, ctx->proto.object,
                 (ob_MethodEntry[]){{"print", o__print},
                                    {"sharesAddressWith:", o__share},
                                    {"prototype", o__prototype},
                                    {"self", o__self},
                                    {"send:with:", o__send},
                                    {"hasMethod:", o__has_method},
                                    OB_METHODS_END});
}
