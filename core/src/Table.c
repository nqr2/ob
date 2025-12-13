#include <ob/Table.h>

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

void obtbl_init(ob_Table *tbl, ob_Allocator *alloc) {
  tbl->allocator = alloc;
  tbl->length = 0;
  tbl->capacity = 0;
  tbl->data = NULL;
}

void obtbl_free(ob_Table *tbl) {
  ob_deallocate(tbl->allocator, tbl->data);

  obtbl_init(tbl, NULL);
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

void obtbl_reserve(ob_Table *tbl, size_t newcap) {
  newcap = stdc_bit_ceil(newcap);
  auto new_entries =
      (TableEntry *)ob_allocate(tbl->allocator, newcap * sizeof(TableEntry));

  memset(new_entries, 0, newcap * sizeof(TableEntry));

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

    memcpy(dest, entry, sizeof(TableEntry));

    tbl->length++;
  }

  ob_deallocate(tbl->allocator, tbl->data);

  tbl->data = new_entries;
  tbl->capacity = newcap;
}

void obtbl_clear(ob_Table *tbl) {
  tbl->length = 0;
  memset(tbl->data, 0, sizeof(TableEntry) * tbl->capacity);
}

// return true if entry is new
bool obtbl_set(ob_Table *tbl, uint64_t key, void *value) {
  TableEntry *entry = NULL;
  auto is_new = false;

  if (2 * (tbl->length + 1) > tbl->capacity) {
    obtbl_reserve(tbl, tbl->length + 1);
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

void obtbl_merge(ob_Table *tbl, ob_Table *from) {
  for (size_t i = 0; i < from->capacity; i++) {
    auto entry = &((TableEntry *)from->data)[i];

    if (entry->status != TES_USED) {
      obtbl_set(tbl, entry->key, entry->value);
    }
  }
}

bool obtbl_get(ob_Table *tbl, uint64_t key, void **value) {
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

bool obtbl_remove(ob_Table *table, uint64_t key) {
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

bool obtbl_iterate(ob_Table *table, uint64_t *index, uint64_t *key,
                   void **value) {
  uint64_t current_key = *index;

  while (current_key < table->capacity) {
    auto entry = &((TableEntry *)table->data)[current_key];

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
