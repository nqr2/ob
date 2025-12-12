#include <ob/Allocator.h>
#include <ob/Assert.h>
#include <ob/Macros.h>

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

ob_Allocator oballoc_create() {
  ob_Allocator result = {};

  result.self = NULL;
  result.allocate = a_malloc;
  result.deallocate = a_free;
  result.reallocate = a_realloc;

  return result;
}

void *ob_allocate(ob_Allocator *alloc, size_t size) {
  if (size == 0) {
    return NULL;
  }

  ASSERT_NONNULL(alloc->allocate);
  void *ptr = alloc->allocate(alloc->self, size);
  ASSERT_NONNULL(ptr);

  memset(ptr, 0, size);

  return ptr;
}

void *ob_reallocate(ob_Allocator *alloc, void *source, size_t new) {
  ASSERT_NONNULL(alloc);

  if (new == 0) {
    ob_deallocate(alloc, source);
    return NULL;
  }

  if (source == 0) {
    return ob_allocate(alloc, new);
  }

  ASSERT_NONNULL(alloc->reallocate);
  void *ptr = alloc->reallocate(alloc->self, source, new);
  ASSERT_NONNULL(ptr);

  return ptr;
}

void ob_deallocate(ob_Allocator *alloc, void *source) {
  ASSERT_NONNULL(alloc);

  if (source == 0) {
    return;
  }

  ASSERT_NONNULL(alloc->deallocate);
  alloc->deallocate(alloc->self, source);
}
