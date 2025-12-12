#include <ob/Context.h>
#include <ob/Hash.h>
#include <ob/Interner.h>
#include <ob/Table.h>

#include <string.h>

void obintr_init(ob_Interner *intr, ob_Context ctx, ob_Allocator *alloc) {
  memset(intr, 0, sizeof(ob_Interner));

  obarr_init(&intr->data, alloc);
  obtbl_init(&intr->interned, alloc);

  intr->context = ctx;
  intr->allocator = alloc;
}

void obintr_free(ob_Interner *intr) {
  obarr_free(&intr->data);
  obtbl_free(&intr->interned);

  obintr_init(intr, NULL, NULL);
}

// TODO: uninterning, etc
ob_String *obintr_intern(ob_Interner *intr, size_t length, const char *data) {
  uint64_t hash = obhash_start(length, data);

  ob_String *str = NULL;

  if (!obtbl_get(&intr->interned, hash, (void **)&str)) {
    str = (ob_String *)ob_allocate(intr->allocator, sizeof(ob_String));

    obarr_push(&intr->data, length, data);

    str->next = intr->context->strings;
    str->offset = intr->data.size - length;

    intr->context->strings = str;

    obtbl_set(&intr->interned, hash, str);
  }

  return str;
}

ob_String *obintr_find(ob_Interner *intr, uint64_t hash) {
  ob_String *str = NULL;

  obtbl_get(&intr->interned, hash, (void **)&str);

  return str;
}

void obintr_mark(ob_Interner *intr) {
  ob_String *str = NULL;
  uint64_t index = 0;

  while (obtbl_iterate(&intr->interned, &index, NULL, (void **)&str)) {
    obstr_mark(str);
  }
}
