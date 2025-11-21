#ifndef EXN_H_INCLUDED
#define EXN_H_INCLUDED

#include "Array.h"

#include <setjmp.h>

typedef union {
  void *pointer;
  size_t integer;
} Exndata;

typedef struct {
  Array entries;
} Exnbuf;

#define EXN_BEGIN(Buf, OnFailure)                                              \
  do {                                                                         \
    jmp_buf jmp;                                                               \
    if (setjmp(jmp)) {                                                         \
      OnFailure                                                                \
    }                                                                          \
    exn__begin((Buf), jmp);                                                    \
  } while (false)

#define EXN_END(Buf) exn__end((Buf))

void exn__begin(Exnbuf *buf, jmp_buf jmp);
void exn__end(Exnbuf *buf);

void exn_init(Exnbuf *buf, Allocator *alloc);
void exn_free(Exnbuf *buf);

const Exndata *exn_data(Exnbuf *buf);
void exn_throw(Exnbuf *buf, Exndata data);
void exn_rethrow(Exnbuf *buf);

#endif
