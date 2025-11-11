#include <stdint.h>
#include <stdlib.h>

#define IGNORE (void)

typedef void *Ptr;

typedef Ptr (*FnAllocate)(Ptr self, size_t size);
typedef void (*FnDeallocate)(Ptr self, Ptr source);
typedef Ptr (*FnReallocate)(Ptr self, Ptr source, size_t new);

typedef struct {
  FnAllocate allocate;
  FnDeallocate deallocate;
  FnReallocate reallocate;
  Ptr self;
} VtAllocator;

static Ptr a_malloc(Ptr _self, size_t size) {
  IGNORE _self;
  return malloc(size);
}

static void a_free(Ptr _self, Ptr source) {
  IGNORE _self;
  free(source);
}

static Ptr a_realloc(Ptr _self, Ptr source, size_t new) {
  IGNORE _self;
  return realloc(source, new);
}

VtAllocator get_libc_allocator() {
  VtAllocator result = {};

  result.self = NULL;
  result.allocate = a_malloc;
  result.deallocate = a_free;
  result.reallocate = a_realloc;

  return result;
}

Ptr allocate(VtAllocator *alloc, size_t size) {
  return alloc->allocate(alloc->self, size);
}

Ptr reallocate(VtAllocator *alloc, Ptr source, size_t new) {
  return alloc->reallocate(alloc->self, source, new);
}

void deallocate(VtAllocator *alloc, Ptr source) {
  alloc->deallocate(alloc->self, source);
}

int main() { return 0; }
