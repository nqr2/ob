#ifndef CONTEXT_H_INCLUDED
#define CONTEXT_H_INCLUDED

#include "Allocator.h"
// #include "Object.h"

typedef struct Context {
  Allocator *allocator;

  // Obj objects;
  // Obj *_prototype...
} *Context;

Context ctx_create(Allocator *alloc);
void ctx_destroy(Context ctx);

#endif
