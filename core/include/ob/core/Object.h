#ifndef OB_CORE_OBJECT_H_INCLUDED
#define OB_CORE_OBJECT_H_INCLUDED

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
 * @brief Objects.
 */

#include <ob/Core.h>

#include <ql/Array.h>
#include <ql/Number.h>
#include <ql/Table.h>

#include <stdint.h>

/** @details
 * This struct is actually a "header", so when the interpreter creates an Obj,
 * it allocates for both this header as well as any data it contains. Note that
 * said data is variably called the *contained data* or the *payload*.
 *
 * In all cases this should be used behind a pointer, using @ref ob_get_payload
 * for obtaining a pointer to the "contained data", with @c ob_cast_* as a safer
 * alternative.
 *
 * @todo
 * - Since this contains a pointer, the size must be a multiple of the pointer
 *   size, so there is a lot left unused.
 */
struct ob_Object {
  union {
    struct {
      ob_ObjectTag tag : 4; /// This object's tag.
      bool mark : 1;        /// This object's GC mark.
    };

    uint16_t word;
  } header;

  /// The size of the *allocated* data.
  uint16_t size;
  uint32_t unused;

  /// The next allocated object, or @c NULL.
  struct ob_Object *next;
};

struct ob_ObjSlots {
  ob_Obj prototype;
  ql_Table slots;
};

struct ob_ObjMethod {
  ob_Obj env;
  ql_ArrayT(ob_Str) parameters;
  ql_ArrayT(uint8_t) bytecode;
  ql_ArrayT(ob_Obj) literals;
};

typedef bool (*ob_FnCMethod)(ob_Ctx ctx);

struct ob_ObjActivation {
  const char *path;
  const char *this_line;
  size_t line;
  size_t column;

  ob_Obj parent;   // the parent activation
  ob_Obj method;   // this method
  ob_Obj receiver; // this method's receiver
  ob_Obj env;      // this context's environment
};

struct ob_ObjCMethod {
  ql_ArrayT(ob_Str) parameters;
  ob_FnCMethod method;
};

typedef void (*ob_FnDestroy)(ob_Obj obj);
typedef void (*ob_FnVisit)(ob_Obj obj, void *userdata);

typedef void (*ob_FnVisit2)(ob_Obj *obj, void *visit_data,
                            void (*push)(ql_Array *target, ob_Obj *obj));

typedef struct {
  ob_Obj prototype;
  ob_FnVisit2 visit;
  ob_FnDestroy finalizer;
} ob_DataMap;

struct ob_ObjCData {
  ob_DataMap *map;
  ob_Obj prototype;
  ob_FnVisit visit;
  ob_FnDestroy destroy;
  void *data;
};

typedef bool (*ob_FnVisitPredicate)(ob_Obj obj, void *userdata);

#define ob_visit_before(Obj, Visit, Userdata)                                  \
  ob_visit((Obj), OB_VISIT_BEFORE, (Visit), (Userdata))

#define ob_visit_after(Obj, Visit, Userdata)                                   \
  ob_visit((Obj), OB_VISIT_AFTER, (Visit), (Userdata))

void *ob_get_payload(ob_Obj obj);
bool obobj_get_mark(ob_Obj obj);

// NOTE: call before destroying an obj
void obobj_destroy(ob_Obj obj);

#define OB_ISA(Obj, Tag) (ob_get_tag((Obj)) == (Tag))

#define OB_IS_INVOCABLE(Obj)                                                   \
  (OB_ISA((Obj), OB_METHOD) || OB_ISA((Obj), OB_LIGHTCMETHOD) ||               \
   OB_ISA((Obj), OB_CMETHOD))

#endif
