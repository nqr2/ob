#include <ob/Exn.h>

#include <string.h>

struct Entry {
  jmp_buf jmp;
  Exncode code;
  Exndata data;
};

void exn__begin(Exnbuf *buf, jmp_buf jmp) {
  struct Entry entry = {};
  memcpy(entry.jmp, jmp, sizeof(jmp_buf));
  arr_push(&buf->entries, sizeof(entry), &entry);
}

void exn__end(Exnbuf *buf) {
  arr_pop(&buf->entries, sizeof(struct Entry), NULL);
}

void exn_init(Exnbuf *buf, Allocator *alloc) {
  arr_init(&buf->entries, alloc);
}

void exn_free(Exnbuf *buf) {
  arr_free(&buf->entries);
}

const Exndata *exn_data(Exnbuf *buf) {
  struct Entry *entry = arr_last(&buf->entries, sizeof(struct Entry));

  if (entry) {
    return &entry->data;
  }

  return NULL;
}

void exn_throw(Exnbuf *buf, Exncode code, Exndata data) {
  struct Entry *ent = arr_last(&buf->entries, sizeof(struct Entry));

  if (ent) {
    ent->code = code;
    ent->data = data;
    longjmp(ent->jmp, code);
  }
}

void exn_rethrow(Exnbuf *buf) {
  struct Entry *ent = arr_last(&buf->entries, sizeof(struct Entry));

  if (ent) {
    longjmp(ent->jmp, ent->code);
  }
}
