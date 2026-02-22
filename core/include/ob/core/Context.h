#ifndef OB_CORE_CONTEXT_H_INCLUDED
#define OB_CORE_CONTEXT_H_INCLUDED

/** @file
 *
 * @brief The interpreter state.
 */

#include <ob/Core.h>

#include <ob/base/Allocator.h>
#include <ob/base/Array.h>
#include <ob/base/Exn.h>
#include <ob/base/Number.h>
#include <ob/base/Table.h>

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
