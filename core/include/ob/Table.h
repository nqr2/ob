#ifndef OB_CORE_TABLE_H_INCLUDED
#define OB_CORE_TABLE_H_INCLUDED

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
