#ifndef OB_CORE_TABLE_H_INCLUDED
#define OB_CORE_TABLE_H_INCLUDED

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
 * @brief Associative arrays between "hashes" and pointers.
 */

#include "Allocator.h"

#include <stdint.h>

typedef struct {
  ob_Allocator *allocator;
  size_t length, capacity;
  void *data;
} ob_Table;

void obtbl_init(ob_Table *tbl, ob_Allocator *alloc);
void obtbl_free(ob_Table *tbl);

void obtbl_reserve(ob_Table *tbl, size_t newcap);
void obtbl_clear(ob_Table *tbl);

// return true if entry is new
bool obtbl_set(ob_Table *tbl, uint64_t key, void *value);

void obtbl_merge(ob_Table *tbl, ob_Table *from);

bool obtbl_get(ob_Table *tbl, uint64_t key, void **value);

bool obtbl_remove(ob_Table *table, uint64_t key);

bool obtbl_iterate(ob_Table *table, uint64_t *index, uint64_t *key,
                   void **value);

#endif
