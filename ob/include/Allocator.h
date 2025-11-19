#ifndef ALLOCATOR_H_INCLUDED
#define ALLOCATOR_H_INCLUDED

#include <stddef.h>

typedef void *(*FnAllocate)(void *self, size_t size);
typedef void (*FnDeallocate)(void *self, void *source);
typedef void *(*FnReallocate)(void *self, void *source, size_t new);

typedef struct {
  FnAllocate allocate;
  FnDeallocate deallocate;
  FnReallocate reallocate;
  void *self;
} Allocator;

Allocator get_libc_allocator();

void *allocate(Allocator *alloc, size_t size);
void *reallocate(Allocator *alloc, void *source, size_t new);
void deallocate(Allocator *alloc, void *source);

#endif
