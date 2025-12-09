#include <ob/Context.h>
#include <ob/Hash.h>
#include <ob/Interner.h>
#include <ob/Table.h>

#include <string.h>

void intr_init(Interner *intr, Context ctx, Allocator *alloc) {
  memset(intr, 0, sizeof(Interner));

  arr_init(&intr->data, alloc);
  tbl_init(&intr->interned, alloc);

  intr->context = ctx;
  intr->allocator = alloc;
}

void intr_free(Interner *intr) {
  arr_free(&intr->data);
  tbl_free(&intr->interned);

  intr_init(intr, NULL, NULL);
}

// TODO: uninterning, etc
String *intr_intern(Interner *intr, size_t length, const char *data) {
  uint64_t hash = hash_start(length, data);

  String *str = NULL;

  if (!tbl_get(&intr->interned, hash, (void **)&str)) {
    str = (String *)allocate(intr->allocator, sizeof(String));

    arr_push(&intr->data, length, data);

    str->next = intr->context->strings;
    str->offset = intr->data.size - length;

    intr->context->strings = str;

    tbl_set(&intr->interned, hash, str);
  }

  return str;
}

String *intr_find(Interner *intr, uint64_t hash) {
  String *str = NULL;

  tbl_get(&intr->interned, hash, (void **)&str);

  return str;
}

void intr_mark(Interner *intr) {
  String *str = NULL;
  uint64_t index = 0;

  while (tbl_iterate(&intr->interned, &index, NULL, (void **)&str)) {
    str_mark(str);
  }
}
