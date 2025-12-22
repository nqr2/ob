#include <ob/bits/AddMethods.h>
#include <ob/lib/Object.h>

#include <ob/Context.h>

#include <stdio.h>
#include <string.h>

void obj_print(ob_Context ctx, ob_Obj receiver) {
  auto tag = obobj_get_tag(receiver);
  switch (tag) {
  case OBOBJ_NIL:
    printf("nil");
    break;

  case OBOBJ_SYMBOL: {
    auto str = (ob_Str *)obobj_get_data(receiver);
    printf("#'%.*s'", (int)obstr_get_length(*str), obstr_get_data(ctx, *str));
  } break;

  case OBOBJ_STRING: {
    auto str = (ob_Str *)obobj_get_data(receiver);
    printf("'%.*s'", (int)obstr_get_length(*str), obstr_get_data(ctx, *str));
  } break;

  case OBOBJ_SLOTS:
    printf("#<slots:%p>", (void *)receiver);
    break;

  case OBOBJ_NUMBER: {
    ob_Number *num = obobj_get_data(receiver);

    if (obnum_is_int(*num)) {
      printf("%ld", obnum_to_int(*num));
    } else {
      printf("%f", obnum_to_float(*num));
    }
  }; break;

    // NUMBER

  case OBOBJ_ARRAY: {
    printf("[");

    ob_Array *arr = obobj_get_data(receiver);
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

  case OBOBJ_METHOD:
    printf("#<method:%p>", (void *)receiver);
    break;

  case OBOBJ_LIGHTCMETHOD: {
    auto method = (ob_FnCMethod *)obobj_get_data(receiver);
    void *ptr = NULL;
    memcpy((void *)&ptr, (void *)method, sizeof(void *));
    printf("#<cmethod:%p>", ptr);
  } break;

  case OBOBJ_LIGHTCDATA: {
    auto data = (void **)obobj_get_data(receiver);
    printf("#<cdata:%p>", *data);
  } break;

  case OBOBJ_ACTIVATION:
    printf("#<activation:%p>", (void *)receiver);
    break;

  default:
    printf("#<r%d:%p>", tag, (void *)receiver);
    break;
  }
}

bool o__print(ob_Context ctx) {
  ob_ObjActivation *activation = obobj_get_data(ctx->this_activation);

  auto receiver = activation->receiver;
  obj_print(ctx, receiver);

  putchar('\n');

  return false;
}

void oblib_load_object(ob_Context ctx) {
  ob_add_methods(ctx, ctx->proto.object,
                 (ob_MethodEntry[]){{"print", o__print}, OB_METHODS_END});
}
