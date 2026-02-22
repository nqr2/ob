#ifndef OB_BASE_TABLE_H_INCLUDED
#define OB_BASE_TABLE_H_INCLUDED

/** @file
 *
 * @brief Associative arrays between "hashes" and pointers.
 */

#include <ob/base/Allocator.h>

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
