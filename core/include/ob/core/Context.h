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

/** @details
 *
 */
struct ob_Context {
  /** @details
   * This data controls the results of @ref ob_should_gc, and thus of when the
   * garbage collection is called. For now that is as simple as collecting when
   * the current heap size exceeds the last one by a given *factor*.
   */
  struct {
    bool enabled;
    float factor;       /// The heap growth factor.
    size_t previous_hs; /// The heap size before this cycle.
  } gc_state;

  ql_Allocator *allocator;

  ql_Array stack;

  /// A list of every allocated object.
  ob_Obj objects;

  /** @brief The prototypes for all object tags.
   *
   * @details
   * The prototype of every object is @c proto.object, which includes
   * @c proto.object itself.
   */
  struct {
    ob_Obj object, nil, symbol, string, slots, number, array, method,
        lightcmethod, cmethod, lightcdata, cdata, activation;
  } proto;

  /// Objects that are guaranteed to exist.
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

void obctx_enter_activation(ob_Ctx ctx, ob_Obj method, ob_Obj receiver);
void obctx_leave_activation(ob_Ctx ctx);

#define OB_BOOL_CAST(Ctx, Bool)                                                \
  (Bool) ? (ctx->known.o_true) : (ctx->known.o_false)

#endif
