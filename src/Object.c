#include "Object.h"

#include "Context.h"

void *obj_payload(Obj obj) {
  auto bytes = (uint8_t *)obj;
  return bytes + sizeof(Object);
}

Obj obj_create(Context ctx, size_t payload_size) {
  auto obj = (Obj)allocate(ctx->allocator, sizeof(Object) + payload_size);

  obj->header = 0;
  obj->next = ctx->objects;

  ctx->objects = obj;

  return obj;
}

void obj_push(Context ctx, Obj obj) {
  arr_push(&ctx->stack, sizeof(Obj), (const void *)&obj);
}

Obj obj_pop(Context ctx) {
  Obj obj;

  if (!arr_pop(&ctx->stack, sizeof(Obj), (void *)&obj)) {
    // TODO: fail? cannot pop empty stack.
  }

  return obj;
}
