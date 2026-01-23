#ifndef QL_EXN_H_INCLUDED
#define QL_EXN_H_INCLUDED

/*
 * Copyright (C) 2025-2026 nqr2
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 */

/** @file
 *
 * @brief "Exception" handling.
 */

#include "Array.h"

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

const ql_Exndata *ql_exn_get_data(ql_Exnbuf *buf);
ql_Exncode ql_exn_get_code(ql_Exnbuf *buf);

void ql_exn_throw(ql_Exnbuf *buf, ql_Exncode code, ql_Exndata data);
void ql_exn_rethrow(ql_Exnbuf *buf);

#endif
