#ifndef OB_CORE_CONTEXT_H_INCLUDED
#define OB_CORE_CONTEXT_H_INCLUDED

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

  ob_Array stack;

  ob_Obj objects;

  ob_Obj proto_object, proto_nil, proto_symbol, proto_string, proto_slots,
      proto_number, proto_array, proto_method, proto_lightcmethod,
      proto_lightcdata, proto_activation;

  ob_Obj shell;

  ob_Obj activation;

  ob_Array string_data;
  ob_Array string_available;

  ob_String *strings;
  ob_Table interned;

  ob_Exnbuf exnbuf;
} *ob_Context;

ob_Context obctx_create(ob_Allocator *alloc);
void obctx_destroy(ob_Context ctx);

ob_Obj obctx_allocate(ob_Context ctx, ob_ObjectTag tag, size_t payload_size);
ob_Obj obctx_alloc_symbol(ob_Context ctx, ob_Str symbol);
ob_Obj obctx_alloc_string(ob_Context ctx, ob_Str string);
ob_Obj obctx_alloc_slots(ob_Context ctx, ob_Obj prototype);
ob_Obj obctx_alloc_number(ob_Context ctx, ob_Number number);
ob_Obj obctx_alloc_integer(ob_Context ctx, int64_t number);
ob_Obj obctx_alloc_real(ob_Context ctx, double number);
ob_Obj obctx_alloc_array(ob_Context ctx);
ob_Obj obctx_alloc_method(ob_Context ctx);
ob_Obj obctx_alloc_lightcmethod(ob_Context ctx, ob_FnCMethod method);
ob_Obj obctx_alloc_lightcdata(ob_Context ctx, void *cdata);

void obctx_gc(ob_Context ctx);

void obctx_enter_activation(ob_Context ctx, ob_Obj method, ob_Obj receiver);
void obctx_leave_activation(ob_Context ctx);

void obctx_push(ob_Context ctx, ob_Obj obj);
ob_Obj obctx_pop(ob_Context ctx);
bool obctx_checkstack(ob_Context ctx, size_t narg);

ob_Obj obctx_get_prototype(ob_Context ctx, ob_Obj obj);
ob_Obj obctx_get_slot(ob_Context ctx, ob_Obj obj, ob_Str selector);
void obctx_send(ob_Context ctx, ob_Obj recv, ob_Str selector);

ob_Exncode obctx_pcall(ob_Context ctx,
                       void (*inner)(ob_Context ctx, void *userdata),
                       void *userdata);

#endif
