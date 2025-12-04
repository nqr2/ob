#include "Array.h"
#include "Context.h"
#include "Object.h"
#include "Parse.h"
#include "String.h"

#include <stdio.h>

bool o__print(Context ctx) {
  printf("activation:%p\n", (void *)ctx->activation);

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
        putchar('\n');
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
    // run a repl
  }

  ctx_sweep(ctx);

  ctx_destroy(ctx);

  return 0;
}
