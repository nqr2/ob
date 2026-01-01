#ifndef OB_CORE_ARRAY_H_INCLUDED
#define OB_CORE_ARRAY_H_INCLUDED

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
  ob_Allocator *allocator;
  size_t size, capacity;
  void *data;
} ob_Array;

void obarr_init(ob_Array *arr, ob_Allocator *alloc);

void obarr_free(ob_Array *arr);

void obarr_reserve(ob_Array *arr, size_t newcap);

void obarr_push(ob_Array *arr, size_t len, const void *data);

void obarr_clear(ob_Array *arr);

bool obarr_pop(ob_Array *arr, size_t len, void *data);

void obarr_remove(ob_Array *arr, size_t size, size_t offset);

size_t obarr_length(ob_Array *arr, size_t size);

void *obarr_at(ob_Array *arr, size_t size, size_t index);

void *obarr_last(ob_Array *arr, size_t size);

#define ob_ArrayT(...) ob_Array

#endif
