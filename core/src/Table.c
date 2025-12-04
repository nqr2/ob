#include "Table.h"

#include <stdbit.h>
#include <string.h>

void tbl_init(Table *tbl, Allocator *alloc) {
  tbl->allocator = alloc;
  tbl->length = 0;
  tbl->capacity = 0;
  tbl->data = NULL;
}

void tbl_free(Table *tbl) {
  deallocate(tbl->allocator, tbl->data);

  tbl_init(tbl, NULL);
}

static TableEntry *tbl__find(size_t capacity, TableEntry *entries,
                             uint64_t key) {
  auto index = key % capacity;

  while (true) {
    auto entry = &entries[index];

    if (entry->key == key || entry->status != TES_USED) {
      return entry;
    }

    index = (index + 1) % capacity;
  }

  return NULL;
}

void tbl_reserve(Table *tbl, size_t newcap) {
  newcap = stdc_bit_ceil(newcap);
  auto new_entries =
      (TableEntry *)allocate(tbl->allocator, newcap * sizeof(TableEntry));

  memset(new_entries, 0, newcap * sizeof(TableEntry));

  tbl->length = 0;

  for (size_t i = 0; i < tbl->capacity; i++) {
    auto entry = &tbl->data[i];
    TableEntry *dest = NULL;

    // shouldnt this be != TES_USED?
    // else it keeps all tombstones???
    if (entry->status == TES_EMPTY) {
      continue;
    }

    dest = tbl__find(newcap, new_entries, entry->key);

    memcpy(dest, entry, sizeof(TableEntry));

    tbl->length++;
  }

  deallocate(tbl->allocator, tbl->data);

  tbl->data = new_entries;
  tbl->capacity = newcap;
}

void tbl_clear(Table *tbl) {
  tbl->length = 0;
  memset(tbl->data, 0, sizeof(TableEntry) * tbl->capacity);
}

// return true if entry is new
bool tbl_set(Table *tbl, uint64_t key, void *value) {
  TableEntry *entry = NULL;
  auto is_new = false;

  if (2 * (tbl->length + 1) > tbl->capacity) {
    tbl_reserve(tbl, tbl->length + 1);
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

void tbl_merge(Table *tbl, Table *from) {
  for (size_t i = 0; i < from->capacity; i++) {
    auto entry = &from->data[i];

    if (entry->status != TES_USED) {
      tbl_set(tbl, entry->key, entry->value);
    }
  }
}

bool tbl_get(Table *tbl, uint64_t key, void **value) {
  if (tbl->length == 0) {
    return false;
  }

  auto index = key % tbl->capacity;
  auto start = index;

  while (true) {
    auto entry = &tbl->data[index];

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

bool tbl_remove(Table *table, uint64_t key) {
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

bool tbl_iterate(Table *table, uint64_t *index, uint64_t *key, void **value) {
  uint64_t current_key = *index;

  while (current_key < table->capacity) {
    auto entry = &table->data[current_key];

    if (entry->status == TES_USED) {
      if (key != NULL) {
        *key = entry->key;
      }

      if (key != NULL) {
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
