#include <ob/Exn.h>

#include <string.h>

struct Entry {
  jmp_buf jmp;
  ob_Exncode code;
  ob_Exndata data;
};

void obexn__begin(ob_Exnbuf *buf, jmp_buf jmp) {
  struct Entry entry = {};
  memcpy(entry.jmp, jmp, sizeof(jmp_buf));
  obarr_push(&buf->entries, sizeof(entry), &entry);
}

void obexn__end(ob_Exnbuf *buf) {
  obarr_pop(&buf->entries, sizeof(struct Entry), NULL);
}

void obexn_init(ob_Exnbuf *buf, ob_Allocator *alloc) {
  obarr_init(&buf->entries, alloc);
}

void obexn_free(ob_Exnbuf *buf) {
  obarr_free(&buf->entries);
}

const ob_Exndata *obexn_data(ob_Exnbuf *buf) {
  struct Entry *entry = obarr_last(&buf->entries, sizeof(struct Entry));

  if (entry) {
    return &entry->data;
  }

  return NULL;
}

void obexn_throw(ob_Exnbuf *buf, ob_Exncode code, ob_Exndata data) {
  struct Entry *ent = obarr_last(&buf->entries, sizeof(struct Entry));

  if (ent) {
    ent->code = code;
    ent->data = data;
    longjmp(ent->jmp, code);
  }
}

void obexn_rethrow(ob_Exnbuf *buf) {
  struct Entry *ent = obarr_last(&buf->entries, sizeof(struct Entry));

  if (ent) {
    longjmp(ent->jmp, ent->code);
  }
}
