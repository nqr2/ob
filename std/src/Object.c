#include "ob/Object.h"
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
    auto str = ob_cast_symbol(receiver);
    printf("#'%.*s'", (int)obstr_get_length(*str), obstr_get_data(ctx, *str));
  } break;

  case OBOBJ_STRING: {
    auto str = ob_cast_string(receiver);
    printf("'%.*s'", (int)obstr_get_length(*str), obstr_get_data(ctx, *str));
  } break;

  case OBOBJ_SLOTS:
    printf("#<slots:%p>", (void *)receiver);
    break;

  case OBOBJ_NUMBER: {
    auto num = ob_cast_number(receiver);

    if (obnum_is_int(*num)) {
      printf("%ld", obnum_to_int(*num));
    } else {
      printf("%f", obnum_to_float(*num));
    }
  }; break;

    // NUMBER

  case OBOBJ_ARRAY: {
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

  case OBOBJ_METHOD:
    printf("#<method:%p>", (void *)receiver);
    break;

  case OBOBJ_LIGHTCMETHOD: {
    auto method = ob_cast_lightcmethod(receiver);
    void *ptr = NULL;
    memcpy((void *)&ptr, (void *)method, sizeof(void *));
    printf("#<cmethod:%p>", ptr);
  } break;

  case OBOBJ_LIGHTCDATA: {
    auto data = ob_cast_lightcdata(receiver);
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
  auto receiver = obctx_get_receiver(ctx);
  obj_print(ctx, receiver);

  putchar('\n');

  return false;
}

static bool o__share(ob_Context ctx) {
  auto receiver = obctx_get_receiver(ctx);
  auto operand = obctx_pop(ctx);

  auto result = OB_BOOL_CAST(ctx, (receiver == operand));
  obctx_push(ctx, result);

  return true;
}

void oblib_load_object(ob_Context ctx) {
  ob_add_methods(ctx, ctx->proto.object,
                 (ob_MethodEntry[]){{"print", o__print},
                                    {"sharesAddressWith:", o__share},
                                    OB_METHODS_END});
}
