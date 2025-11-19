#include "Allocator.h"
#include "Macros.h"

#include <stdlib.h>
#include <string.h>

static void *a_malloc(void *_self, size_t size) {
  IGNORE _self;
  return calloc(1, size);
}

static void a_free(void *_self, void *source) {
  IGNORE _self;
  free(source);
}

static void *a_realloc(void *_self, void *source, size_t new) {
  IGNORE _self;
  return realloc(source, new);
}

Allocator get_libc_allocator() {
  Allocator result = {};

  result.self = NULL;
  result.allocate = a_malloc;
  result.deallocate = a_free;
  result.reallocate = a_realloc;

  return result;
}

void *allocate(Allocator *alloc, size_t size) {
  void *ptr = alloc->allocate(alloc->self, size);

  memset(ptr, 0, size);

  return ptr;
}

void *reallocate(Allocator *alloc, void *source, size_t new) {
  void *ptr = alloc->reallocate(alloc->self, source, new);

  return ptr;
}

void deallocate(Allocator *alloc, void *source) {
  alloc->deallocate(alloc->self, source);
}
