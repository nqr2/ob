#ifndef OB_CORE_H_INCLUDED
#define OB_CORE_H_INCLUDED

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

#include <ql/Allocator.h>
#include <ql/Array.h>
#include <ql/Exn.h>
#include <ql/Number.h>

#include <stdint.h>

typedef struct ob_Context *ob_Ctx;
typedef struct ob_String *ob_Str;
typedef struct ob_Object *ob_Obj;

typedef struct ob_ObjSlots ob_ObjSlots;
typedef struct ob_ObjMethod ob_ObjMethod;
typedef struct ob_ObjCMethod ob_ObjCMethod;
typedef struct ob_ObjCData ob_ObjCData;
typedef struct ob_ObjActivation ob_ObjActivation;

typedef enum : ql_Exncode {
  OB_OK = 0,

  OB_DOES_NOT_UNDERSTAND,
} ob_Exncode;

typedef enum : uint8_t {
  OB_NIL = 0,          // the nil object
  OB_SYMBOL = 1,       // #... / #a:b:...y:z: / #'...' / #+...-
  OB_STRING = 2,       // '...'
  OB_SLOTS = 3,        // slot objects
  OB_NUMBER = 4,       // numbers
  OB_ARRAY = 5,        // [ ... ]
  OB_METHOD = 6,       // closures
  OB_LIGHTCMETHOD = 7, // functions from C
  OB_CMETHOD = 8,      // annotated cmethod
  OB_LIGHTCDATA = 9,   // data from C
  OB_CDATA = 10,       // annotated cdata
  OB_ACTIVATION = 11,  // call stack entry
  OB_RESERVED_c = 12,
  OB_RESERVED_d = 13,
  OB_RESERVED_e = 14,
  OB_RESERVED_f = 15,
} ob_ObjectTag;

typedef enum : uint8_t {
  OB_VISIT_NONE = 0,
  OB_VISIT_AFTER = 1,
  OB_VISIT_BEFORE = 2,
} ob_VisitFlags;

typedef void (*ob_FnDestroy)(ob_Obj obj);
typedef void (*ob_FnVisit)(ob_Obj obj, void *userdata);
typedef bool (*ob_FnVisitPredicate)(ob_Obj obj, void *userdata);
typedef bool (*ob_FnCMethod)(ob_Ctx ctx);

ob_Ctx ob_create(ql_Allocator *alloc);
void ob_destroy(ob_Ctx ctx);

ob_Obj ob_create_symbol(ob_Ctx ctx, ob_Str symbol);
ob_Obj ob_create_string(ob_Ctx ctx, ob_Str string);
ob_Obj ob_create_slots(ob_Ctx ctx, ob_Obj prototype);
ob_Obj ob_create_number(ob_Ctx ctx, ql_Number number);
ob_Obj ob_create_integer(ob_Ctx ctx, int64_t number);
ob_Obj ob_create_real(ob_Ctx ctx, double number);
ob_Obj ob_create_array(ob_Ctx ctx);
ob_Obj ob_create_method(ob_Ctx ctx);
ob_Obj ob_create_lightcmethod(ob_Ctx ctx, ob_FnCMethod method);
ob_Obj ob_create_cmethod(ob_Ctx ctx, ob_FnCMethod method, ql_Array parameters);
ob_Obj ob_create_lightcdata(ob_Ctx ctx, void *cdata);
ob_Obj ob_create_cdata(ob_Ctx ctx, ob_Obj prototype, ob_FnVisit visit,
                       ob_FnDestroy destructor, void *data);

ob_Str *ob_cast_symbol(ob_Obj obj);
ob_Str *ob_cast_string(ob_Obj obj);
ob_ObjSlots *ob_cast_slots(ob_Obj obj);
ql_Number *ob_cast_number(ob_Obj obj);
ql_ArrayT(ob_Obj) * ob_cast_array(ob_Obj obj);
ob_ObjMethod *ob_cast_method(ob_Obj obj);
ob_FnCMethod *ob_cast_lightcmethod(ob_Obj obj);
ob_ObjCMethod *ob_cast_cmethod(ob_Obj obj);
void **ob_cast_lightcdata(ob_Obj obj);
ob_ObjCData *ob_cast_cdata(ob_Obj obj);
ob_ObjActivation *ob_cast_activation(ob_Obj obj);

// NOTE: this also invokes visit on the obj in question
void ob_visit(ob_Obj obj, ob_VisitFlags flags, ob_FnVisit visit,
              ob_FnVisitPredicate predicate, void *userdata);

void ob_mark(ob_Obj obj);
bool ob_should_gc(ob_Ctx ctx);
void ob_gc2(ob_Ctx ctx, bool force); // NOTE: rename to just "gc" later

void ob_push(ob_Ctx ctx, ob_Obj obj);
ob_Obj ob_pop(ob_Ctx ctx);
bool ob_checkstack(ob_Ctx ctx, size_t narg);

ob_ObjectTag ob_get_tag(ob_Obj obj);
ob_Obj ob_get_prototype(ob_Ctx ctx, ob_Obj obj);
bool ob_get_slot(ob_Ctx ctx, ob_Obj *slot, ob_Obj obj, ob_Str selector);
ob_Obj ob_get_receiver(ob_Ctx ctx);

typedef enum {
  OB_SEND_DNUW = 0x1, // Dispatch #doesNotUnderstand:with:
} ob_SendFlags;

void ob_send_ext(ob_Ctx ctx, ob_Obj recv, ob_Str selector, ob_SendFlags flags);
void ob_send(ob_Ctx ctx, ob_Obj recv, ob_Str selector);

ob_Exncode ob_pcall(ob_Ctx ctx, void (*inner)(ob_Ctx ctx, void *userdata),
                    void *userdata);

/* Parse some text, and return a unary closure */
ob_Obj ob_load(ob_Ctx ctx, size_t length, const char *text);

/* Parse some text, and run it. */
void ob_run(ob_Ctx ctx, size_t length, const char *text);

#define OB_ISA(Obj, Tag) (ob_get_tag((Obj)) == (Tag))

#define OB_IS_INVOCABLE(Obj)                                                   \
  (OB_ISA((Obj), OB_METHOD) || OB_ISA((Obj), OB_LIGHTCMETHOD) ||               \
   OB_ISA((Obj), OB_CMETHOD))

#endif
