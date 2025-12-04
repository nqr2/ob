#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include "Allocator.h"

#include <stdint.h>

typedef enum {
  TES_EMPTY = 0,
  TES_USED = 1,
  TES_DEAD = 2,
} TableEntryStatus;

typedef struct {
  uint64_t key;

  void *value;
  TableEntryStatus status;
} TableEntry;

typedef struct {
  Allocator *allocator;
  size_t length, capacity;
  TableEntry *data;
} Table;

void tbl_init(Table *tbl, Allocator *alloc);
void tbl_free(Table *tbl);

void tbl_reserve(Table *tbl, size_t newcap);
void tbl_clear(Table*tbl);

// return true if entry is new
bool tbl_set(Table *tbl, uint64_t key, void *value);

void tbl_merge(Table *tbl, Table *from);

bool tbl_get(Table *tbl, uint64_t key, void **value);

bool tbl_remove(Table *table, uint64_t key);

bool tbl_iterate(Table *table, uint64_t *index, uint64_t *key, void **value);

#endif
