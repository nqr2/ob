#include "Context.h"
#include "Allocator.h"
#include "Array.h"

#include <string.h>

Context ctx_create(Allocator *alloc) {
  Context ctx = allocate(alloc, sizeof(struct Context));

  memset(ctx, 0, sizeof(struct Context));

  arr_init(&ctx->stack, alloc);
  arr_init(&ctx->string_data, alloc);
  arr_init(&ctx->string_available, alloc);

  ctx->allocator = alloc;

  ctx->proto_nil = obj_create_slots(ctx, NULL);
  ctx->proto_symbol = obj_create_slots(ctx, NULL);
  ctx->proto_string = obj_create_slots(ctx, NULL);
  ctx->proto_slots = obj_create_slots(ctx, NULL);
  ctx->proto_integer = obj_create_slots(ctx, NULL);
  ctx->proto_real = obj_create_slots(ctx, NULL);
  ctx->proto_method = obj_create_slots(ctx, NULL);
  ctx->proto_cmethod = obj_create_slots(ctx, NULL);
  ctx->proto_cdata = obj_create_slots(ctx, NULL);
  ctx->proto_activation = obj_create_slots(ctx, NULL);

  return ctx;
}

void ctx_destroy(Context ctx) {
  auto alloc = ctx->allocator;

  // TODO: get rid of objects and strings here

  ctx_sweep(ctx);
  ctx_sweep(ctx);

  arr_free(&ctx->stack);
  arr_free(&ctx->string_data);
  arr_free(&ctx->string_available);

  deallocate(alloc, ctx);
}

void ctx_mark(Context ctx) {
  obj_mark(ctx->activation);

  obj_mark(ctx->proto_nil);
  obj_mark(ctx->proto_symbol);
  obj_mark(ctx->proto_string);
  obj_mark(ctx->proto_slots);
  obj_mark(ctx->proto_integer);
  obj_mark(ctx->proto_real);
  obj_mark(ctx->proto_method);
  obj_mark(ctx->proto_cmethod);
  obj_mark(ctx->proto_cdata);
  obj_mark(ctx->proto_activation);

  auto data = (Object **)ctx->stack.data;

  for (size_t i = 0; i < ctx->stack.size / sizeof(Object *); i++) {
    obj_mark(data[i]);
  }
}

void ctx_sweep(Context ctx) {
  str_sweep(ctx);

  Object *newlive = NULL;
  auto live = ctx->objects;

  while (live != NULL) {
    Object *next = live->next;

    if (HEADER_GET_MARK(live->header)) {
      live->header = HEADER_SET_MARK(live->header, false);
    } else {
      obj_destroy(live);
      deallocate(ctx->allocator, live);
      live = NULL;
    }

    if (live != NULL) {
      live->next = newlive;
      newlive = live;
    }

    live = next;
  }

  ctx->objects = newlive;
}

void ctx_enter_activation(Context ctx, Obj caller, Obj method, Obj receiver) {
  Object *act = obj_create(ctx, sizeof(ObjActivation));

  ObjActivation *data = obj_payload(act);
  data->parent = ctx->activation;
  data->caller = caller;
  data->method = method;
  data->receiver = receiver;
  data->env = obj_create_slots(ctx, NULL);

  ctx->activation = act;
}

void ctx_leave_activation(Context ctx) {
  ObjActivation *data = obj_payload(ctx->activation);
  ctx->activation = data->parent;
}

bool ctx_checkstack(Context ctx, size_t narg) {
  return (ctx->stack.size / sizeof(Object *)) >= narg;
}
