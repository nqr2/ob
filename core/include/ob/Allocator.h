#ifndef QL_ALLOCATOR_H_INCLUDED
#define QL_ALLOCATOR_H_INCLUDED

/*
 * Copyright (C) 2025-2026 nqr2
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 */

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
