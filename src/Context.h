#ifndef CONTEXT_H_INCLUDED
#define CONTEXT_H_INCLUDED

#include "Allocator.h"
#include "Object.h"

#include "Array.h"

typedef struct Context {
  Allocator *allocator;

  Array stack;

  Obj objects;
  // Obj *_prototype...
} *Context;

Context ctx_create(Allocator *alloc);
void ctx_destroy(Context ctx);

#endif
