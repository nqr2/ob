#include <ob/Argparse.h>
#include <ob/Array.h>
#include <ob/Assert.h>
#include <ob/Context.h>
#include <ob/Hash.h>
#include <ob/Number.h>
#include <ob/Object.h>
#include <ob/Parse.h>
#include <ob/String.h>

#include <ob/Std.h>
#include <ob/bits/AddMethods.h>

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
  ob_ObjActivation *activation = obobj_get_data(ctx->activation);

  auto receiver = activation->receiver;
  obj_print(ctx, receiver);

  putchar('\n');

  return false;
}

bool o__other(ob_Context ctx) {
  (void)ctx;

  return true;
}

void dofile(ob_Context ctx, const char *path) {
  auto file = fopen(path, "r");

  if (file == NULL) {
    return;
  }

  auto data = (ob_Array){};
  auto length = 0L;

  fseek(file, 0, SEEK_END);

  length = ftell(file);

  rewind(file);

  obarr_init(&data, ctx->allocator);
  obarr_reserve(&data, length);
  data.size = length;

  fread(data.data, sizeof(char), data.size, file);

  ob_run(ctx, data.size, data.data);

  // exit:
  obarr_free(&data);
  fclose(file);
}

void repl(ob_Context ctx) {
  auto line = (ob_Array){};
  obarr_init(&line, ctx->allocator);

  obarr_reserve(&line, 256);

  while (true) {
    obarr_clear(&line);
    obarr_clear(&ctx->stack);

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
      obarr_push(&line, sizeof(char), &tmp2);
    }

    ob_run(ctx, line.size, line.data);

    putchar('=');
    putchar('\t');

    auto obj = obctx_pop(ctx);
    obj_print(ctx, obj);

    putchar('\n');
  }

  obarr_free(&line);
}

int main(int argn, const char *argv[]) {
  bool is_interactive = argn == 1;

  auto f_interactive =
      obarg_create_flag('i', "interactive", OBARG_FLAG_SET, &is_interactive);
  f_interactive.description = "Open the interactive shell";

  auto parser = obarg_create_parser((ob_Flag[]){f_interactive, OB_FLAGS_END});

  obarg_parse(&parser, argn, argv);

  auto alloc = oballoc_create();

  auto ctx = obctx_create(&alloc);
  oblib_load_all(ctx);

  ob_add_methods(ctx, ctx->proto_object,
                 (ob_MethodEntry[]){{"print", o__print},
                                    {"right:", o__other},
                                    {">:", o__other},
                                    OB_METHODS_END});

  if (argn != 1) {
    for (int i = 1; i < argn; i++) {
      dofile(ctx, argv[i]);
    }
  }

  if (is_interactive) {
    repl(ctx);
  }

  obctx_destroy(ctx);

  return 0;
}
