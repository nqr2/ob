#include "Array.h"
#include "Context.h"
#include "Object.h"
#include "Parse.h"
#include "String.h"

#include <stdio.h>

bool o__print(Context ctx) {
  ObjActivation *activation = obj_get_data(ctx->activation);

  auto receiver = activation->receiver;
  auto tag = obj_get_tag(receiver);
  switch (tag) {
  case OT_NIL:
    printf("nil");
    break;

  case OT_SYMBOL:
    putchar('#');

  case OT_STRING: {
    ObjString *str = obj_get_data(receiver);
    printf("'%.*s'", (int)str_get_length(str->inner),
           str_get_data(ctx, str->inner));
  } break;

  case OT_SLOTS:
    printf("#<slots:%p>", (void *)receiver);
    break;

    // INTEGER
    // REAL

  case OT_METHOD:
    printf("#<method:%p>", (void *)receiver);
    break;

  case OT_CMETHOD: {
    ObjCMethod *method = obj_get_data(receiver);
    printf("#<cmethod:%p>", (void *)method->method);
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

int main(int argn, char *argv[]) {
  auto alloc = get_libc_allocator();

  auto ctx = ctx_create(&alloc);

  ObjSlots *p_obj = obj_get_data(ctx->proto_object);

  auto sel = str_create_literal(ctx, "print");
  auto print = ctx_alloc_cmethod(ctx, o__print);
  tbl_set(&p_obj->slots, str_get_hash(ctx, sel), (void *)print);

  sel = str_create_literal(ctx, "right:");
  auto right = ctx_alloc_cmethod(ctx, o__other);
  tbl_set(&p_obj->slots, str_get_hash(ctx, sel), (void *)right);

  sel = str_create_literal(ctx, ">:");
  tbl_set(&p_obj->slots, str_get_hash(ctx, sel), (void *)right);

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
