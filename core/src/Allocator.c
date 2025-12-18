#include <ob/Allocator.h>
#include <ob/Assert.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void *a_alloc(void *userdata, size_t ptr_size, void *ptr,
                     size_t new_size) {
  (void)userdata;
  (void)ptr_size;

  if (new_size == 0) {
    free(ptr);
    ptr = NULL;
  } else if (ptr == NULL) {
    ptr = malloc(new_size);
  } else {
    ptr = realloc(ptr, new_size);
  }

  return ptr;
}

ob_Allocator oballoc_create() {
  auto result = (ob_Allocator){};

  result.userdata = NULL;
  result.used = 0;
  result.allocate = a_alloc;

  return result;
}

void *ob_allocate(ob_Allocator *alloc, size_t size) {
  return ob_reallocate(alloc, 0, NULL, size);
}

void *ob_reallocate(ob_Allocator *alloc, size_t old, void *source, size_t new) {
  ASSERT_NONNULL(alloc);

  ASSERT_NONNULL(alloc->allocate);
  void *ptr = alloc->allocate(alloc->userdata, old, source, new);

  if (new != 0) {
    ASSERT_NONNULL(ptr);
  }

  if (old < new) {
    alloc->used += new - old;
    memset(((uint8_t *)ptr) + old, 0, new - old);
  } else if (old > new) {
    alloc->used -= old - new;
  }

  return ptr;
}

void ob_deallocate(ob_Allocator *alloc, size_t size, void *source) {
  ob_reallocate(alloc, size, source, 0);
}
