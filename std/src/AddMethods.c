#include "ob/Core.h"
#include <ob/base/Assert.h>
#include <ob/base/Hash.h>
#include <ob/bits/AddMethods.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>

#include <string.h>

static void add_method(ob_Ctx ctx, ob_Obj target, char const *name,
                       ob_FnCMethod method) {
  QL_ASSERT(ob_get_tag(target) == OB_SLOTS, "expected a slots object");

  auto slots = ob_cast_slots(target);

  auto key = ob_create_symbol(ctx, strlen(name), name);
  auto value = ob_create_lightcmethod(ctx, method);

  auto slot = (ob_Slot){.key = *ob_cast_symbol(key), .value = value};
  ql_array_push(&slots->slots, sizeof(ob_Slot), &slot);
}

void ob_add_methods(ob_Ctx ctx, ob_Obj target, ob_MethodEntry const *entries) {
  while (entries->name != NULL) {
    add_method(ctx, target, entries->name, entries->method);

    entries++;
  }
}
