#include <ob/bits/AddMethods.h>
#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>
#include <ob/lib/Slots.h>

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

static bool slots_at(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto key = ob_pop(ctx);

  auto sym = ob_cast_symbol(key);
  auto slots = ob_cast_slots(receiver);

  auto hash = obstr_get_hash(ctx, *sym);

  ob_Obj obj = nullptr;
  ql_table_get(&slots->slots, hash, (void **)&obj);

  ob_push(ctx, obj);

  return true;
}

void oblib_load_slots(ob_Ctx ctx) {
  ob_add_methods(ctx, ctx->proto.slots,
                 (ob_MethodEntry[]){{"at:put:", slots_at_put},
                                    {"at:", slots_at},
                                    OB_METHODS_END});
}
