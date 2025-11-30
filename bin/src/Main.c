#include <stdbit.h>

#include "Context.h"
#include "Hash.h"
#include "Parse.h"

#include <stdio.h>

bool o__print(Context ctx) {
  printf("activation:%p\n", (void *)ctx->activation);

  return false;
}

int main() {
  auto alloc = get_libc_allocator();

  auto ctx = ctx_create(&alloc);

  run_literal(ctx, "");

  auto obj = ctx_alloc_slots(ctx, NULL);

  ObjSlots *data = obj_get_data(obj);

  auto sel = str_create_literal(ctx, "print");

  auto print = ctx_alloc_cmethod(ctx, o__print);

  tbl_set(&data->slots, hash_start(sel->length, sel->data), (void *)print);

  ctx_send(ctx, obj, sel);

  ctx_sweep(ctx);

  ctx_destroy(ctx);

  return 0;
}
