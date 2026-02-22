#ifndef OB_BASE_EXN_H_INCLUDED
#define OB_BASE_EXN_H_INCLUDED

/** @file
 *
 * @brief "Exception" handling.
 */

#include <ob/base/Array.h>

#include <setjmp.h>

typedef int ql_Exncode;

typedef union {
  void *pointer;
  size_t integer;
} ql_Exndata;

typedef struct {
  ql_Array entries;
} ql_Exnbuf;

#define QL_EXN_BEGIN(Buf, OnFailure)                                           \
  do {                                                                         \
    jmp_buf jmp;                                                               \
    if (setjmp(jmp)) {                                                         \
      OnFailure                                                                \
    }                                                                          \
    ql_exn__begin((Buf), jmp);                                                 \
  } while (false)

#define QL_EXN_END(Buf) ql_exn__end((Buf))

void ql_exn__begin(ql_Exnbuf *buf, jmp_buf jmp);
void ql_exn__end(ql_Exnbuf *buf);

void ql_exn_init(ql_Exnbuf *buf, ql_Allocator *alloc);
void ql_exn_free(ql_Exnbuf *buf);

ql_Exndata const *ql_exn_get_data(ql_Exnbuf *buf);
ql_Exncode ql_exn_get_code(ql_Exnbuf *buf);

void ql_exn_throw(ql_Exnbuf *buf, ql_Exncode code, ql_Exndata data);
void ql_exn_rethrow(ql_Exnbuf *buf);

#endif
