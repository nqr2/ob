#include "Context.h"
#include "Allocator.h"

Context ctx_create(Allocator *alloc) {
  Context ctx = allocate(alloc, sizeof(struct Context));

  ctx->allocator = alloc;

  return ctx;
}

void ctx_destroy(Context ctx) {
  auto alloc = ctx->allocator;

  deallocate(alloc, ctx);
}
