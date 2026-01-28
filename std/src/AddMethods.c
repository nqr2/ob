#include <ob/bits/AddMethods.h>

#include <ob/core/Object.h>
#include <ob/core/String.h>

#include <ql/Assert.h>
#include <ql/Hash.h>

#include <string.h>

static void add_method(ob_Ctx ctx, ob_Obj target, const char *name,
                       ob_FnCMethod method) {
  QL_ASSERT(ob_get_tag(target) == OB_SLOTS, "expected a slots object");

  auto slots = ob_cast_slots(target);

  auto obj = ob_create_lightcmethod(ctx, method);
  auto hash = ql_hash_start(strlen(name), name);

  ql_table_set(&slots->slots, hash, (void *)obj);
}

void ob_add_methods(ob_Ctx ctx, ob_Obj target, const ob_MethodEntry *entries) {
  while (entries->name != NULL) {
    add_method(ctx, target, entries->name, entries->method);

    entries++;
  }
}
