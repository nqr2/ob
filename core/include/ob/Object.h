#ifndef OB_CORE_OBJECT_H_INCLUDED
#define OB_CORE_OBJECT_H_INCLUDED

/*
 * Copyright (C) 2025 nqr2
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
 * @brief Objects.
 */

#include "Array.h"
#include "ContextFwd.h"
#include "Number.h"
#include "String.h"
#include "Table.h"

#include <stdint.h>

typedef enum Tag : uint8_t {
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

typedef struct Object {
  union {
    struct {
      ob_ObjectTag tag : 4;
      bool mark : 1;
    };

    uint16_t word;
  } header;

  uint16_t size;
  uint32_t unused;
  struct Object *next;
} ob_Object;

typedef ob_Object *ob_Obj;

typedef struct {
  ob_Object *prototype;
  ob_Table slots;
} ob_ObjSlots;

typedef struct {
  ob_Obj env;
  ob_ArrayT(ob_Str) parameters;
  ob_ArrayT(uint8_t) bytecode;
  ob_ArrayT(ob_Obj) literals;
} ob_ObjMethod;

typedef bool (*ob_FnCMethod)(ob_Context ctx);

typedef struct {
  ob_Obj parent;   // the parent activation
  ob_Obj method;   // this method
  ob_Obj receiver; // this method's receiver
  ob_Obj env;      // this context's environment
} ob_ObjActivation;

typedef struct {
  ob_ArrayT(ob_Str) parameters;
  ob_FnCMethod method;
} ob_ObjCMethod;

typedef void (*ob_FnDestroy)(ob_Obj obj);
typedef void (*ob_FnVisit)(ob_Obj obj, void *userdata);

typedef struct {
  ob_Obj prototype;
  ob_FnVisit visit;
  ob_FnDestroy destroy;
  void *data;
} ob_ObjCData;

typedef bool (*ob_FnVisitPredicate)(ob_Obj obj, void *userdata);

typedef enum : uint8_t {
  OB_VISIT_NONE = 0,
  OB_VISIT_AFTER = 1,
  OB_VISIT_BEFORE = 2,
} ob_VisitFlags;

// NOTE: this also invokes visit on the obj in question
void ob_visit(ob_Obj obj, ob_VisitFlags flags, ob_FnVisit visit,
              ob_FnVisitPredicate predicate, void *userdata);

#define ob_visit_before(Obj, Visit, Userdata)                                  \
  ob_visit((Obj), OB_VISIT_BEFORE, (Visit), (Userdata))

#define ob_visit_after(Obj, Visit, Userdata)                                   \
  ob_visit((Obj), OB_VISIT_AFTER, (Visit), (Userdata))

ob_ObjectTag ob_get_tag(ob_Obj obj);
void *ob_get_payload(ob_Obj obj);
bool obobj_get_mark(ob_Obj obj);

void ob_mark(ob_Obj obj);

// NOTE: call before destroying an obj
void obobj_destroy(ob_Obj obj);

ob_Str *ob_cast_symbol(ob_Obj obj);
ob_Str *ob_cast_string(ob_Obj obj);
ob_ObjSlots *ob_cast_slots(ob_Obj obj);
ob_Number *ob_cast_number(ob_Obj obj);
ob_ArrayT(ob_Obj) * ob_cast_array(ob_Obj obj);
ob_ObjMethod *ob_cast_method(ob_Obj obj);
ob_FnCMethod *ob_cast_lightcmethod(ob_Obj obj);
ob_ObjCMethod *ob_cast_cmethod(ob_Obj obj);
void **ob_cast_lightcdata(ob_Obj obj);
ob_ObjCData *ob_cast_cdata(ob_Obj obj);
ob_ObjActivation *ob_cast_activation(ob_Obj obj);

#define OB_ISA(Obj, Tag) (ob_get_tag((Obj)) == (Tag))

#define OB_IS_INVOCABLE(Obj)                                                   \
  (OB_ISA((Obj), OB_METHOD) || OB_ISA((Obj), OB_LIGHTCMETHOD) ||               \
   OB_ISA((Obj), OB_CMETHOD))

#endif
