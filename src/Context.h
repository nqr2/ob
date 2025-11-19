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
  // Obj *_prototype...

  Obj activation;

  Array string_data;
  Array string_available;

  String *strings;
} *Context;

Context ctx_create(Allocator *alloc);
void ctx_destroy(Context ctx);

void ctx_mark(Context ctx);
void ctx_sweep(Context ctx);

void ctx_enter_activation(Context ctx, Obj caller, Obj method, Obj receiver);
void ctx_leave_activation(Context ctx);

#endif
