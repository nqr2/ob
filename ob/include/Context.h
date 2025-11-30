#ifndef CONTEXT_H_INCLUDED
#define CONTEXT_H_INCLUDED

#include "Allocator.h"
#include "Object.h"
#include "String.h"

#include "Array.h"

typedef struct Context {
  Allocator *allocator;

  Array stack;

  Obj objects;

  Obj proto_nil, proto_symbol, proto_string, proto_slots, proto_integer,
      proto_real, proto_method, proto_cmethod, proto_cdata, proto_activation;

  Obj activation;

  Array string_data;
  Array string_available;

  String *strings;
} *Context;

Context ctx_create(Allocator *alloc);
void ctx_destroy(Context ctx);

Obj ctx_allocate(Context ctx, size_t payload_size);
Obj ctx_alloc_symbol(Context ctx, String *symbol);
Obj ctx_alloc_string(Context ctx, String *string);
Obj ctx_alloc_slots(Context ctx, Obj prototype);
Obj ctx_alloc_integer(Context ctx, int64_t number);
Obj ctx_alloc_real(Context ctx, double number);
Obj ctx_alloc_method(Context ctx);
Obj ctx_alloc_cmethod(Context ctx, FnCMethod method);
Obj ctx_alloc_cdata(Context ctx, void *cdata);

void ctx_mark(Context ctx);
void ctx_sweep(Context ctx);

void ctx_enter_activation(Context ctx, Obj caller, Obj method, Obj receiver);
void ctx_leave_activation(Context ctx);

void ctx_push(Context ctx, Obj obj);
Obj ctx_pop(Context ctx);
bool ctx_checkstack(Context ctx, size_t narg);

Obj ctx_get_prototype(Context ctx, Obj obj);
Obj ctx_get_slot(Context ctx, Obj obj, String *selector);
void ctx_send(Context ctx, Obj recv, String *selector);

#endif
