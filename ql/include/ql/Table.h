#ifndef QL_TABLE_H_INCLUDED
#define QL_TABLE_H_INCLUDED

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
  ql_Allocator *allocator;
  size_t length, capacity;
  void *data;
} ql_Table;

void ql_table_init(ql_Table *tbl, ql_Allocator *alloc);
void ql_table_free(ql_Table *tbl);

void ql_table_reserve(ql_Table *tbl, size_t newcap);
void ql_table_clear(ql_Table *tbl);

// return true if entry is new
bool ql_table_set(ql_Table *tbl, uint64_t key, void *value);

void ql_table_merge(ql_Table *tbl, ql_Table *from);

bool ql_table_get(ql_Table *tbl, uint64_t key, void **value);

bool ql_table_remove(ql_Table *table, uint64_t key);

bool ql_table_iterate(ql_Table *table, uint64_t *index, uint64_t *key,
                      void **value);

#endif
