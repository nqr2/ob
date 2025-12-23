#include <ob/bits/AddMethods.h>

#include <ob/Assert.h>
#include <ob/Context.h>
#include <ob/Hash.h>

#include <string.h>

static void add_method(ob_Context ctx, ob_Obj target, const char *name,
                       ob_FnCMethod method) {
  ASSERT(obobj_get_tag(target) == OBOBJ_SLOTS, "expected a slots object");

  auto slots = ob_cast_slots(target);

  auto obj = obctx_alloc_lightcmethod(ctx, method);
  auto hash = obhash_start(strlen(name), name);

  obtbl_set(&slots->slots, hash, (void *)obj);
}

void ob_add_methods(ob_Context ctx, ob_Obj target,
                    const ob_MethodEntry *entries) {
  while (entries->name != NULL) {
    add_method(ctx, target, entries->name, entries->method);

    entries++;
  }
}
