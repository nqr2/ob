#ifndef OB_CORE_ALLOCATOR_H_INCLUDED
#define OB_CORE_ALLOCATOR_H_INCLUDED

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
typedef void *(*ob_FnAllocate)(void *userdata, size_t ptr_size, void *ptr,
                               size_t new_size);

/// A vtable for a memory allocator.
typedef struct {
  ob_FnAllocate allocate;
  void *userdata;
} ob_Allocator;

/// Creates a default allocator.
ob_Allocator oballoc_create();

void *ob_allocate(ob_Allocator *alloc, size_t size);
void *ob_reallocate(ob_Allocator *alloc, size_t old, void *source, size_t new);
void ob_deallocate(ob_Allocator *alloc, size_t size, void *source);

#endif
