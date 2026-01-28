#ifndef OB_CORE_CONTEXT_H_INCLUDED
#define OB_CORE_CONTEXT_H_INCLUDED

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
 * @brief The interpreter state.
 */

#include <ob/Core.h>

#include <ql/Allocator.h>
#include <ql/Array.h>
#include <ql/Exn.h>
#include <ql/Number.h>
#include <ql/Table.h>

struct ob_Context {
  struct {
    bool enabled;
    float factor;
    size_t previous_hs;
  } gc_state;

  ql_Allocator *allocator;

  ql_Array stack;

  ob_Obj objects;

  struct {
    ob_Obj object, nil, symbol, string, slots, number, array, method,
        lightcmethod, cmethod, lightcdata, cdata, activation;
  } proto;

  struct {
    ob_Obj shell, o_true, o_false;
  } known;

  ob_Obj this_activation;

  ql_Array string_data;
  ql_Array string_available;

  ob_Str strings;
  ql_Table interned;

  ql_Exnbuf exnbuf;
};

ob_Obj obctx_allocate(ob_Ctx ctx, ob_ObjectTag tag, size_t payload_size);

void ob_gc(ob_Ctx ctx);

void obctx_enter_activation(ob_Ctx ctx, ob_Obj method, ob_Obj receiver);
void obctx_leave_activation(ob_Ctx ctx);

#define OB_BOOL_CAST(Ctx, Bool)                                                \
  (Bool) ? (ctx->known.o_true) : (ctx->known.o_false)

#endif
