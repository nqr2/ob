#include <ql/Exn.h>

#include <string.h>

struct Entry {
  jmp_buf jmp;
  ql_Exncode code;
  ql_Exndata data;
};

void ql_exn__begin(ql_Exnbuf *buf, jmp_buf jmp) {
  struct Entry entry = {};
  memcpy(entry.jmp, jmp, sizeof(jmp_buf));
  ql_array_push(&buf->entries, sizeof(entry), &entry);
}

void ql_exn__end(ql_Exnbuf *buf) {
  ql_array_pop(&buf->entries, sizeof(struct Entry), NULL);
}

void ql_exn_init(ql_Exnbuf *buf, ql_Allocator *alloc) {
  ql_array_init(&buf->entries, alloc);
}

void ql_exn_free(ql_Exnbuf *buf) {
  ql_array_free(&buf->entries);
}

const ql_Exndata *ql_exn_get_data(ql_Exnbuf *buf) {
  struct Entry *entry = ql_array_last(&buf->entries, sizeof(struct Entry));

  if (entry) {
    return &entry->data;
  }

  return NULL;
}

ql_Exncode ql_exn_get_code(ql_Exnbuf *buf) {
  struct Entry *entry = ql_array_last(&buf->entries, sizeof(struct Entry));

  if (entry) {
    return entry->code;
  }

  return 0;
}

void ql_exn_throw(ql_Exnbuf *buf, ql_Exncode code, ql_Exndata data) {
  struct Entry *ent = ql_array_last(&buf->entries, sizeof(struct Entry));

  if (ent) {
    ent->code = code;
    ent->data = data;
    longjmp(ent->jmp, code);
  }
}

void ql_exn_rethrow(ql_Exnbuf *buf) {
  struct Entry *ent = ql_array_last(&buf->entries, sizeof(struct Entry));

  if (ent) {
    longjmp(ent->jmp, ent->code);
  }
}
