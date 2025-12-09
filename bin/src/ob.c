#include "Array.h"
#include "Assert.h"
#include "Context.h"
#include "Hash.h"
#include "Number.h"
#include "Object.h"
#include "Parse.h"
#include "String.h"

#include <stdio.h>
#include <string.h>

void obj_print(Context ctx, Obj receiver) {

  auto tag = obj_get_tag(receiver);
  switch (tag) {
  case OT_NIL:
    printf("nil");
    break;

  case OT_SYMBOL:
    putchar('#');
    [[fallthrough]];

  case OT_STRING: {
    ObjString *str = obj_get_data(receiver);
    printf("'%.*s'", (int)str_get_length(str->inner),
           str_get_data(ctx, str->inner));
  } break;

  case OT_SLOTS:
    printf("#<slots:%p>", (void *)receiver);
    break;

  case OT_NUMBER: {
    ObjNumber *num = obj_get_data(receiver);

    if (num_is_int(num->number)) {
      printf("%ld", num_to_int(num->number));
    } else {
      printf("%f", num_to_float(num->number));
    }
  }; break;

    // NUMBER

  case OT_ARRAY: {
    printf("[");

    ObjArray *arr = obj_get_data(receiver);
    auto size = arr->items.size / sizeof(Obj);

    auto items = (Obj *)arr->items.data;

    for (size_t i = 0; i < size; i++) {
      obj_print(ctx, items[i]);

      if (i != size - 1) {
        printf(" . ");
      }
    }

    printf("]");
  }; break;

  case OT_METHOD:
    printf("#<method:%p>", (void *)receiver);
    break;

  case OT_CMETHOD: {
    ObjCMethod *method = obj_get_data(receiver);
    void *ptr = NULL;
    memcpy((void *)&ptr, (void *)&method->method, sizeof(void *));
    printf("#<cmethod:%p>", ptr);
  } break;

  case OT_CDATA: {
    ObjCData *data = obj_get_data(receiver);
    printf("#<cdata:%p>", data->cdata);
  } break;

  case OT_ACTIVATION:
    printf("#<activation:%p>", (void *)receiver);
    break;

  default:
    printf("#<r%d:%p>", tag, (void *)receiver);
    break;
  }
}

bool o__print(Context ctx) {
  ObjActivation *activation = obj_get_data(ctx->activation);

  auto receiver = activation->receiver;
  obj_print(ctx, receiver);

  putchar('\n');

  return false;
}

bool o__other(Context ctx) {
  (void)ctx;

  return true;
}

void dofile(Context ctx, const char *path) {
  auto file = fopen(path, "r");

  if (file == NULL) {
    return;
  }

  auto data = (Array){};
  auto length = 0L;

  fseek(file, 0, SEEK_END);

  length = ftell(file);

  rewind(file);

  arr_init(&data, ctx->allocator);
  arr_reserve(&data, length);
  data.size = length;

  fread(data.data, sizeof(char), data.size, file);

  run_file(ctx, data.size, data.data);

  // exit:
  arr_free(&data);
  fclose(file);
}

void repl(Context ctx) {
  auto line = (Array){};
  arr_init(&line, ctx->allocator);

  arr_reserve(&line, 256);

  while (true) {
    arr_clear(&line);

    if (feof(stdin)) {
      break;
    }

    printf("* ");
    fflush(stdout);

    int tmp = 0;
    while (tmp != '\n') {
      tmp = getchar();

      if (tmp == -1) {
        puts("bye");
        break;
      }

      auto tmp2 = (char)tmp;
      arr_push(&line, sizeof(char), &tmp2);
    }

    run_file(ctx, line.size, line.data);
  }

  arr_free(&line);
}

void add_method(Context ctx, Obj target, const char *name, FnCMethod method) {
  ASSERT(obj_get_tag(target) == OT_SLOTS, "expected a slots object");

  ObjSlots *slots = obj_get_data(target);

  auto obj = ctx_alloc_cmethod(ctx, method);
  auto hash = hash_start(strlen(name), name);

  tbl_set(&slots->slots, hash, (void *)obj);
}

typedef struct {
  const char *name;
  FnCMethod method;
} Entry;

void add_methods(Context ctx, Obj target, const Entry *entries) {
  while (entries->name != NULL) {
    add_method(ctx, target, entries->name, entries->method);

    entries++;
  }
}

int main(int argn, char *argv[]) {
  auto alloc = get_libc_allocator();

  auto ctx = ctx_create(&alloc);

  auto lib = (Entry[]){
      {"print", o__print},
      {"right:", o__other},
      {">:", o__other},
      {NULL, NULL},
  };

  add_methods(ctx, ctx->proto_object, lib);

  if (argn != 1) {
    for (int i = 1; i < argn; i++) {
      dofile(ctx, argv[i]);
    }
  } else {
    repl(ctx);
  }

  ctx_destroy(ctx);

  return 0;
}
