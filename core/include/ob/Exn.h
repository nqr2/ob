#ifndef OB_CORE_EXN_H_INCLUDED
#define OB_CORE_EXN_H_INCLUDED

/** @file
 *
 * @brief "Exception" handling.
 */

#include "Array.h"

#include <setjmp.h>

typedef int ob_Exncode;

typedef union {
  void *pointer;
  size_t integer;
} ob_Exndata;

typedef struct {
  ob_Array entries;
} ob_Exnbuf;

#define OB_EXN_BEGIN(Buf, OnFailure)                                           \
  do {                                                                         \
    jmp_buf jmp;                                                               \
    if (setjmp(jmp)) {                                                         \
      OnFailure                                                                \
    }                                                                          \
    obexn__begin((Buf), jmp);                                                  \
  } while (false)

#define OB_EXN_END(Buf) obexn__end((Buf))

void obexn__begin(ob_Exnbuf *buf, jmp_buf jmp);
void obexn__end(ob_Exnbuf *buf);

void obexn_init(ob_Exnbuf *buf, ob_Allocator *alloc);
void obexn_free(ob_Exnbuf *buf);

const ob_Exndata *obexn_data(ob_Exnbuf *buf);
ob_Exncode obexn_code(ob_Exnbuf *buf);

void obexn_throw(ob_Exnbuf *buf, ob_Exncode code, ob_Exndata data);
void obexn_rethrow(ob_Exnbuf *buf);

#endif
