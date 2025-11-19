#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

#include "ContextFwd.h"
#include "Macros.h"

#include "Array.h"

#include <stdint.h>

// 4 bit tag, 1 bit mark, 11 rc?
typedef uint16_t Header;

typedef enum Tag {
  OT_NIL = 0,        // the nil object
  OT_SYMBOL = 1,     // #... / #a:b:...y:z: / #'...' / #+...-
  OT_STRING = 2,     // '...'
  OT_SLOTS = 3,      // slot objects
  OT_INTEGER = 4,    // integers
  OT_REAL = 5,       // floats
  OT_METHOD = 6,     // closures
  OT_CMETHOD = 7,    // functions from C
  OT_CDATA = 8,      // data from C
  OT_ACTIVATION = 9, // call stack entry
  OT_Ra = 10,
  OT_Rb = 11,
  OT_Rc = 12,
  OT_Rd = 13,
  OT_Re = 14,
  OT_Rf = 15,
} ObjectTag;

typedef struct Object {
  Header header;
  struct Object *next;
} Object;

typedef Object *Obj;

#define HEADER_GET_TAG(H) ((Header)((H) & 0xf))
#define HEADER_SET_TAG(H, T) ((Header)(((H) & 0xffff'fff0) | ((T) & 0xf)))

#define HEADER_GET_MARK(H) (((H) & 0x10) != 0)
#define HEADER_SET_MARK(H, M) ((Header)(((H) & ~0x10) | (((M) != 0) << 4)))

#define HEADER_GET_RC(H) ((H) >> 5)
#define HEADER_SET_RC(H, C) (((H) & 0x1f) | ((C) << 5))

#define RC_MAX 32

typedef void (*FnVisitor)(Obj obj);

// NOTE: this also invokes visit on the obj in question
void obj_visit(Obj obj, FnVisitor visit);

void *obj_payload(Obj obj);

void obj_mark(Obj obj) {
  if (HEADER_GET_MARK(obj->header)) {
    return;
  }

  switch (HEADER_GET_TAG(obj->header)) {
  case OT_STRING:
  case OT_SYMBOL: {
    struct {
      String *inner;
    } *str = obj_payload(obj);
    str_mark(str->inner);
  } break;

  default:
    break;
  }

  obj->header = HEADER_SET_MARK(obj->header, true);

  obj_visit(obj, obj_mark);
}

Obj obj_ref(Obj obj) {
  auto refcount = HEADER_GET_RC(obj->header);

  if (refcount < RC_MAX) {
    refcount++;
    obj->header = HEADER_SET_RC(obj->header, refcount);
  }

  return obj;
}

// true if rc=0
bool obj_unref(Obj obj) {
  auto refcount = HEADER_GET_RC(obj->header);

  if (refcount < RC_MAX) {
    refcount--;
    obj->header = HEADER_SET_RC(obj->header, refcount);
  }

  return refcount == 0;
}

[[deprecated("use Context.objects")]]
Obj live = NULL;

Obj obj_create(Context ctx, size_t payload_size);

static void obj__unref_(Obj obj) {
  IGNORE obj_unref(obj);
}

// NOTE: call before destroying an obj
void obj__destroy(Obj obj);

#define OBJ_CREATE_T(A, T) obj_create((A), sizeof(T))

[[deprecated("use Context.stack")]]
Array stack;

void obj_push(Context ctx, Obj obj);

Obj obj_pop(Context ctx);

#endif
