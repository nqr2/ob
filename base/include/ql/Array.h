#ifndef QL_ARRAY_H_INCLUDED
#define QL_ARRAY_H_INCLUDED

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
 * @brief Dynamically allocated arrays.
 */

#include "Allocator.h"

typedef struct {
  ql_Allocator *allocator;
  size_t size, capacity;
  void *data;
} ql_Array;

void ql_array_init(ql_Array *arr, ql_Allocator *alloc);

void ql_array_free(ql_Array *arr);

void ql_array_reserve(ql_Array *arr, size_t newcap);

void ql_array_push(ql_Array *arr, size_t len, const void *data);

void ql_array_clear(ql_Array *arr);

bool ql_array_pop(ql_Array *arr, size_t len, void *data);

void ql_array_remove(ql_Array *arr, size_t size, size_t offset);

size_t ql_array_length(ql_Array *arr, size_t size);

void *ql_array_at(ql_Array *arr, size_t size, size_t index);

void *ql_array_last(ql_Array *arr, size_t size);

#define ql_ArrayT(...) ql_Array

#endif
