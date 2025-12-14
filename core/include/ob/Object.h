#ifndef OB_CORE_OBJECT_H_INCLUDED
#define OB_CORE_OBJECT_H_INCLUDED

/** @file
 *
 * @brief Objects.
 */

#include "Array.h"
#include "ContextFwd.h"
#include "Table.h"

#include <stdint.h>

typedef enum Tag : uint8_t {
  OBOBJ_NIL = 0,           // the nil object
  OBOBJ_SYMBOL = 1,        // #... / #a:b:...y:z: / #'...' / #+...-
  OBOBJ_STRING = 2,        // '...'
  OBOBJ_SLOTS = 3,         // slot objects
  OBOBJ_NUMBER = 4,        // numbers
  OBOBJ_ARRAY = 5,         // [ ... ]
  OBOBJ_METHOD = 6,        // closures
  OBOBJ_LIGHT_CMETHOD = 7, // functions from C
  OBOBJ_LIGHT_CDATA = 8,   // data from C
  OBOBJ_ACTIVATION = 9,    // call stack entry
  OT_Ra = 10,
  OT_Rb = 11,
  OT_Rc = 12,
  OT_Rd = 13,
  OT_Re = 14,
  OT_Rf = 15,
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
  uint32_t refcount;
  struct Object *next;
} ob_Object;

typedef ob_Object *ob_Obj;

typedef struct {
  ob_Object *prototype;
  ob_Table slots;
} ob_ObjSlots;

typedef struct {
  ob_Obj env;
  ob_Array parameters;
  ob_Array bytecode;
  ob_Array literals;
} ob_ObjMethod;

typedef bool (*ob_FnCMethod)(ob_Context ctx);

typedef struct {
  ob_Obj parent;   // the parent activation
  ob_Obj caller;   // the method's caller
  ob_Obj method;   // this method
  ob_Obj receiver; // this method's receiver
  ob_Obj env;      // this context's environment
} ob_ObjActivation;

typedef void (*ob_FnVisit)(ob_Obj obj, void *userdata);

typedef enum : uint8_t {
  VISIT_NONE = 0,
  VISIT_AFTER = 1,
  VISIT_BEFORE = 2,
} ob_VisitFlags;

// NOTE: this also invokes visit on the obj in question
void obobj_visit(ob_Obj obj, ob_VisitFlags flags, ob_FnVisit visit,
                 void *userdata);

#define obobj_visit_before(Obj, Visit, Userdata)                               \
  obobj_visit((Obj), VISIT_BEFORE, (Visit), (Userdata))

#define obobj_visit_after(Obj, Visit, Userdata)                                \
  obobj_visit((Obj), VISIT_AFTER, (Visit), (Userdata))

ob_ObjectTag obobj_get_tag(ob_Obj obj);
void *obobj_get_data(ob_Obj obj);
bool obobj_get_mark(ob_Obj obj);

void obobj_mark(ob_Obj obj);

ob_Obj obobj_ref(ob_Obj obj);

// true if rc=0
bool obobj_unref(ob_Obj obj);

// NOTE: call before destroying an obj
void obobj_destroy(ob_Obj obj);

#define OBOBJ_ISA(Obj, Tag) (obobj_get_tag((Obj)) == (Tag))

#define OBOBJ_IS_INVOCABLE(Obj)                                                \
  (OBOBJ_ISA((Obj), OBOBJ_METHOD) || OBOBJ_ISA((Obj), OBOBJ_LIGHT_CMETHOD))

#endif
