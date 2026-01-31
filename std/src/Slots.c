#include <ob/bits/AddMethods.h>
#include <ob/lib/Slots.h>

#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>

static bool slots_at_put(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto value = ob_pop(ctx);
  auto key = ob_pop(ctx);

  auto sym = ob_cast_symbol(key);
  auto slots = ob_cast_slots(receiver);

  auto hash = obstr_get_hash(ctx, *sym);

  ql_table_set(&slots->slots, hash, value);

  return false;
}

void oblib_load_slots(ob_Ctx ctx) {
  ob_add_methods(ctx, ctx->proto.slots,
                 (ob_MethodEntry[]){{"at:put:", slots_at_put}, OB_METHODS_END});
}
