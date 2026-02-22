#ifndef OB_BASE_ALLOCATOR_H_INCLUDED
#define OB_BASE_ALLOCATOR_H_INCLUDED

/** @file
 *
 * @brief Memory allocators.
 */

#include <stddef.h>

/** @brief An allocation function.
 *
 * @returns @c NULL if @p new_size is 0.
 * @returns An allocated pointer if @ptr is NULL and @p new_size is not 0.
 * @returns A resized pointer otherwise.
 */
typedef void *(*ql_FnAllocate)(void *userdata, size_t ptr_size, void *ptr,
                               size_t new_size);

/// A vtable for a memory allocator.
typedef struct {
  ql_FnAllocate allocate;
  size_t used;
  void *userdata;
} ql_Allocator;

/// Creates a default allocator.
ql_Allocator ql_alloc_create();

void *ql_allocate(ql_Allocator *alloc, size_t size);
void *ql_reallocate(ql_Allocator *alloc, size_t old, void *source, size_t new);
void ql_deallocate(ql_Allocator *alloc, size_t size, void *source);

#endif
