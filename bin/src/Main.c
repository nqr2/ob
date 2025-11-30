#include <stdbit.h>

#include "Context.h"
#include "Hash.h"
#include "Object.h"
#include "Parse.h"

#include <stdio.h>

bool o__print(Context ctx) {
  printf("activation:%p\n", (void *)ctx->activation);

  return false;
}

int main() {
  auto alloc = get_libc_allocator();

  auto ctx = ctx_create(&alloc);

  ObjSlots *p_int = obj_get_data(ctx->proto_integer);
  auto sel = str_create_literal(ctx, "print");
  auto print = ctx_alloc_cmethod(ctx, o__print);
  tbl_set(&p_int->slots, hash_start(sel->length, sel->data), (void *)print);

  run_literal(ctx, "  1   \"ignore this comment!\"   print  ");

  ctx_sweep(ctx);

  ctx_destroy(ctx);

  return 0;
}
