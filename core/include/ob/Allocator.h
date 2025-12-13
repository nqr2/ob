#ifndef OB_CORE_ALLOCATOR_H_INCLUDED
#define OB_CORE_ALLOCATOR_H_INCLUDED

/** @file
 *
 * @brief Allocation utilities.
 */

#include <stddef.h>

typedef void *(*ob_FnAllocate)(void *self, size_t size);
typedef void (*ob_FnDeallocate)(void *self, void *source);
typedef void *(*ob_FnReallocate)(void *self, void *source, size_t new);

/// A vtable for a memory allocator.
typedef struct {
  ob_FnAllocate allocate;
  ob_FnDeallocate deallocate;
  ob_FnReallocate reallocate;
  void *self;
} ob_Allocator;

/// Creates a default allocator.
ob_Allocator oballoc_create();

/// Allocates a pointer.
void *ob_allocate(ob_Allocator *alloc, size_t size);

/// Reallocates a pointer.
void *ob_reallocate(ob_Allocator *alloc, void *source, size_t new);

/// Deallocates a pointer.
void ob_deallocate(ob_Allocator *alloc, void *source);

#endif
