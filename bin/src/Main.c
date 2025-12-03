#include <stdbit.h>

#include "Context.h"
#include "Object.h"
#include "Parse.h"

#include <stdio.h>

bool o__print(Context ctx) {
  printf("activation:%p\n", (void *)ctx->activation);

  return false;
}

bool o__other(Context ctx) {
  (void)ctx;

  return true;
}

int main() {
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

  run_literal(
      ctx, "   \"this should only return 3:      "
           "     { { 1 print } >: 2 } right: 3 \""
           "  (1 print >: 2 right: 3)   \"ignore this comment!\"   print  .\n"
           "  { 4 print . 5 print } print . #selector print.   ");

  ctx_sweep(ctx);

  ctx_destroy(ctx);

  return 0;
}
