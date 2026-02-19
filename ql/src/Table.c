#include <ql/Table.h>

#include <stdbit.h>
#include <string.h>

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

void ql_table_init(ql_Table *tbl, ql_Allocator *alloc) {
  tbl->allocator = alloc;
  tbl->length = 0;
  tbl->capacity = 0;
  tbl->data = NULL;
}

void ql_table_free(ql_Table *tbl) {
  ql_deallocate(tbl->allocator, tbl->capacity * sizeof(TableEntry), tbl->data);

  ql_table_init(tbl, NULL);
}

static TableEntry *tbl__find(size_t capacity, TableEntry *entries,
                             uint64_t key) {
  // valid since capacity is always a power of 2
  auto index = key & (capacity - 1);

  while (true) {
    auto entry = &entries[index];

    if (entry->key == key || entry->status != TES_USED) {
      return entry;
    }

    index = (index + 1) % capacity;
  }

  return NULL;
}

void ql_table_reserve(ql_Table *tbl, size_t newcap) {
  newcap = stdc_bit_ceil(newcap);
  auto new_entries =
      (TableEntry *)ql_allocate(tbl->allocator, newcap * sizeof(TableEntry));

  tbl->length = 0;

  for (size_t i = 0; i < tbl->capacity; i++) {
    auto entry = &((TableEntry *)tbl->data)[i];
    TableEntry *dest = NULL;

    // shouldnt this be != TES_USED?
    // else it keeps all tombstones???
    if (entry->status == TES_EMPTY) {
      continue;
    }

    dest = tbl__find(newcap, new_entries, entry->key);
    *dest = *entry;

    tbl->length++;
  }

  ql_deallocate(tbl->allocator, tbl->capacity * sizeof(TableEntry), tbl->data);

  tbl->data = new_entries;
  tbl->capacity = newcap;
}

void ql_table_clear(ql_Table *tbl) {
  tbl->length = 0;
  memset(tbl->data, 0, sizeof(TableEntry) * tbl->capacity);
}

// return true if entry is new
bool ql_table_set(ql_Table *tbl, uint64_t key, void *value) {
  TableEntry *entry = NULL;
  auto is_new = false;

  if (2 * (tbl->length + 1) > tbl->capacity) {
    ql_table_reserve(tbl, tbl->length + 1);
  }

  entry = tbl__find(tbl->capacity, tbl->data, key);
  is_new = entry->status != TES_USED;

  if (is_new) {
    tbl->length++;
  }

  entry->key = key;
  entry->value = value;
  entry->status = TES_USED;
  return is_new;
}

void ql_table_merge(ql_Table *tbl, ql_Table *from) {
  for (size_t i = 0; i < from->capacity; i++) {
    auto entry = &((TableEntry *)from->data)[i];

    if (entry->status != TES_USED) {
      ql_table_set(tbl, entry->key, entry->value);
    }
  }
}

bool ql_table_get(ql_Table *tbl, uint64_t key, void **value) {
  if (tbl->length == 0) {
    return false;
  }

  auto index = key % tbl->capacity;
  auto start = index;

  while (true) {
    auto entry = &((TableEntry *)tbl->data)[index];

    if (entry->key == key && entry->status == TES_USED) {
      if (value != NULL) {
        *value = entry->value;
      }

      return true;
    }

    index += 1;
    index %= tbl->capacity;

    if (index == start) {
      return false;
    }
  }

  return false;
}

bool ql_table_remove(ql_Table *table, uint64_t key) {
  TableEntry *entry = NULL;

  if (table->length == 0) {
    return false;
  }

  entry = tbl__find(table->capacity, table->data, key);

  if (entry->status != TES_USED) {
    return false;
  }

  entry->status = TES_DEAD;
  return true;
}

bool ql_table_iterate(ql_Table *table, uint64_t *index, uint64_t *key,
                      void **value) {
  uint64_t current_key = *index;

  while (current_key < table->capacity) {
    auto entry = &((TableEntry *)table->data)[current_key];

    if (entry->status == TES_USED) {
      if (key != NULL) {
        *key = entry->key;
      }

      if (value != NULL) {
        *value = entry->value;
      }

      break;
    }

    current_key += 1;
  }

  if (current_key >= table->capacity) {
    return false;
  }

  current_key += 1;
  *index = current_key;
  return true;
}
