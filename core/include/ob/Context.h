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

#include "Allocator.h"
#include "Array.h"
#include "Exn.h"
#include "Number.h"
#include "Object.h"
#include "String.h"

typedef struct Context {
  struct {
    bool enabled;
    float factor;
    size_t previous_hs;
  } gc_state;

  ob_Allocator *allocator;

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

  ob_String *strings;
  ob_Table interned;

  ob_Exnbuf exnbuf;
} *ob_Context;

ob_Context obctx_create(ob_Allocator *alloc);
void obctx_destroy(ob_Context ctx);

ob_Obj obctx_allocate(ob_Context ctx, ob_ObjectTag tag, size_t payload_size);
ob_Obj ob_create_symbol(ob_Context ctx, ob_Str symbol);
ob_Obj ob_create_string(ob_Context ctx, ob_Str string);
ob_Obj ob_create_slots(ob_Context ctx, ob_Obj prototype);
ob_Obj ob_create_number(ob_Context ctx, ob_Number number);
ob_Obj ob_create_integer(ob_Context ctx, int64_t number);
ob_Obj ob_create_real(ob_Context ctx, double number);
ob_Obj ob_create_array(ob_Context ctx);
ob_Obj ob_create_method(ob_Context ctx);
ob_Obj ob_create_lightcmethod(ob_Context ctx, ob_FnCMethod method);
ob_Obj ob_create_cmethod(ob_Context ctx, ob_FnCMethod method,
                         ql_Array parameters);
ob_Obj ob_create_lightcdata(ob_Context ctx, void *cdata);
ob_Obj ob_create_cdata(ob_Context ctx, ob_Obj prototype, ob_FnVisit visit,
                       ob_FnDestroy destructor, void *data);

void ob_gc(ob_Context ctx);

void obctx_enter_activation(ob_Context ctx, ob_Obj method, ob_Obj receiver);
void obctx_leave_activation(ob_Context ctx);

void ob_push(ob_Context ctx, ob_Obj obj);
ob_Obj ob_pop(ob_Context ctx);
bool ob_checkstack(ob_Context ctx, size_t narg);

ob_Obj ob_get_prototype(ob_Context ctx, ob_Obj obj);

bool ob_get_slot(ob_Context ctx, ob_Obj *slot, ob_Obj obj, ob_Str selector);

typedef enum {
  OB_SEND_DNUW = 0x1, // Dispatch #doesNotUnderstand:with:
} ob_SendFlags;

void ob_send_ext(ob_Context ctx, ob_Obj recv, ob_Str selector,
                 ob_SendFlags flags);

void ob_send(ob_Context ctx, ob_Obj recv, ob_Str selector);

ob_Exncode obctx_pcall(ob_Context ctx,
                       void (*inner)(ob_Context ctx, void *userdata),
                       void *userdata);

ob_Obj ob_get_receiver(ob_Context ctx);

#define OB_BOOL_CAST(Ctx, Bool)                                                \
  (Bool) ? (ctx->known.o_true) : (ctx->known.o_false)

#endif
