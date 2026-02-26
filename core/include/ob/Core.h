#ifndef OB_CORE_H_INCLUDED
#define OB_CORE_H_INCLUDED

/** @file
 *
 * @brief
 * `ob`'s "core" definitions and functionality.
 *
 * For most cases you should just @c #include this header, and only use the ones
 * in the @c ob/core directory for "lower-level" implementation details.
 */

#include <ob/bits/Begin.h>

#include <ob/base/Allocator.h>
#include <ob/base/Array.h>
#include <ob/base/Exn.h>
#include <ob/base/Number.h>

#include <stdint.h>

/// The interpreter state.
typedef struct ob_Context *ob_Ctx;

typedef struct ob_String *ob_Str;
typedef struct ob_Object *ob_Obj;

typedef struct ob_ObjSlots ob_ObjSlots;
typedef struct ob_ObjMethod ob_ObjMethod;
typedef struct ob_ObjCMethod ob_ObjCMethod;
typedef struct ob_ObjCData ob_ObjCData;
typedef struct ob_ObjActivation ob_ObjActivation;

typedef enum : ql_Exncode {
  /// No errors happened.
  OB_OK = 0,

  /// An object did not understand a message and could not dispatch
  /// @c #call-missing:with:.
  OB_CALLED_MISSING,
} ob_Exncode;

/// Object tags. @sa ob_Object::proto for details on "containing" data.
typedef enum : uint8_t {
  /// The @c NULL object.
  /// Cannot contain anything.
  OB_NIL = 0,

  /// Interned strings.
  /// Contains a @ref ob_Str.
  OB_SYMBOL = 1,

  /// Uninterned strings.
  /// Contains a @ref ob_Str.
  OB_STRING = 2,

  /// Slot objects.
  /// Contains a @ref ob_ObjSlots.
  OB_SLOTS = 3,

  /// Number objects.
  /// Contains a @c ql_Number.
  OB_NUMBER = 4,

  /// Array objects.
  /// Contains a @c ql_Array
  OB_ARRAY = 5,

  /// Closure objects.
  /// Contains a @ref ob_ObjMethod.
  OB_METHOD = 6,

  /// "Raw" functions from C.
  /// Contains a @ref ob_FnCMethod.
  OB_LIGHTCMETHOD = 7,

  /// Annotated functions from C.
  /// Contains a @ref ob_ObjCMethod.
  OB_CMETHOD = 8,

  /// "Raw" pointers from C.
  /// Contains a @c void*.
  OB_LIGHTCDATA = 9,

  /// Annotated data from C.
  /// Contains a @ref ob_ObjCData.
  OB_CDATA = 10,

  /// Call stack entries.
  /// Contains a @ref ob_ObjActivation.
  OB_ACTIVATION = 11,

  OB_RESERVED_c = 12,
  OB_RESERVED_d = 13,
  OB_RESERVED_e = 14,
  OB_RESERVED_f = 15,
} ob_ObjectTag;

typedef enum : uint8_t {
  /// Do not call the @p visit callback on the object.
  OB_VISIT_NONE = 0,

  /// Call the @p visit callback before visiting it's children.
  OB_VISIT_AFTER = 1,

  /// Call the @p visit callback after visiting it's children.
  OB_VISIT_BEFORE = 2,
} ob_VisitFlags;

/// A destructor callback.
typedef void (*ob_FnDestroy)(ob_Obj obj);

/// A visit callback.
typedef void (*ob_FnVisit)(ob_Obj obj, void *userdata);

typedef bool (*ob_FnVisitPredicate)(ob_Obj obj, void *userdata);

typedef bool (*ob_FnCMethod)(ob_Ctx ctx);

/// Allocate and initialize an interpreter context.
ob_Ctx ob_create(ql_Allocator *alloc);

/// Destroy an interpreter context.
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
ql_Array *ob_cast_array(ob_Obj obj);
ob_ObjMethod *ob_cast_method(ob_Obj obj);
ob_FnCMethod *ob_cast_lightcmethod(ob_Obj obj);
ob_ObjCMethod *ob_cast_cmethod(ob_Obj obj);
void **ob_cast_lightcdata(ob_Obj obj);
ob_ObjCData *ob_cast_cdata(ob_Obj obj);
ob_ObjActivation *ob_cast_activation(ob_Obj obj);

/**
 * @brief Call a function on an object and each of it's children.
 *
 * @param obj       The object to be visited
 * @param flags     Flags controlling when to call a callback.
 * @param visit     The callback to call on every object.
 * @param predicate If this is not @c NULL, only recurse if it returns @c true.
 * @param userdata  Extra data to pass to @p visit.
 */
void ob_visit(ob_Obj obj, ob_VisitFlags flags, ob_FnVisit visit,
              ob_FnVisitPredicate predicate, void *userdata);

/// Set an object's GC mark.
void ob_mark(ob_Obj obj);

/// @returns @c true if the interpreter should call the GC at this point.
bool ob_should_gc(ob_Ctx ctx);

/** @brief Perform a garbage collection cycle.
 *
 * @param force Force a cycle to be run even if not needed.
 */
void ob_gc(ob_Ctx ctx, bool force);

/// Push an object into the interpreter stack.
void ob_push(ob_Ctx ctx, ob_Obj obj);

/// Pop an object from the interpreter stack.
ob_Obj ob_pop(ob_Ctx ctx);

/// @returns @c true if there are (at least) @p narg items in the stack.
bool ob_checkstack(ob_Ctx ctx, size_t narg);

/// @returns The object's tag.
ob_ObjectTag ob_get_tag(ob_Obj obj);

/** @returns The object's prototype.
 *
 * @important
 * Because of how inheritance works, be careful when using this in a loop,
 * since calling this when @p obj is @c NULL will *not* return @c NULL.
 * @sa ob_Context for details.
 */
ob_Obj ob_get_prototype(ob_Ctx ctx, ob_Obj obj);
bool ob_get_slot(ob_Ctx ctx, ob_Obj *slot, ob_Obj obj, ob_Str selector);

/// @returns The receiver in this stack frame.
ob_Obj ob_get_receiver(ob_Ctx ctx);

typedef enum {
  /// Dispatch #callMissing:with: if the method is not present.
  OB_SEND_CMW = 0x1,
} ob_SendFlags;

void ob_send_ext(ob_Ctx ctx, ob_Obj recv, ob_Str selector, ob_SendFlags flags);

/// Send a message to a receiver.
void ob_send(ob_Ctx ctx, ob_Obj recv, ob_Str selector);

/// Call a function, and catch any exceptions thrown.
ob_Exncode ob_pcall(ob_Ctx ctx, void (*inner)(ob_Ctx ctx, void *userdata),
                    void *userdata);

/// Parse some text with a path, and return a unary closure
ob_Obj ob_load_ext(ob_Ctx ctx, char const *file, size_t length,
                   char const *text);

/// Parse some text, and return a unary closure
ob_Obj ob_load(ob_Ctx ctx, size_t length, char const *text);

/// Call @ref ob_load_ext, and invoke the returned closure.
void ob_run_ext(ob_Ctx ctx, char const *file, size_t length, char const *text);

/// Call @ref ob_load, and invoke the returned closure.
void ob_run(ob_Ctx ctx, size_t length, char const *text);

#define OB_ISA(Obj, Tag) (ob_get_tag((Obj)) == (Tag))

#define OB_IS_INVOCABLE(Obj)                                                   \
  (OB_ISA((Obj), OB_METHOD) || OB_ISA((Obj), OB_LIGHTCMETHOD) ||               \
   OB_ISA((Obj), OB_CMETHOD))

#include <ob/bits/End.h>

#endif
