#include <ob/bits/AddMethods.h>
#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>
#include <ob/lib/Slots.h>

static bool slots_at_put(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto value = ob_pop(ctx);
  auto key = ob_pop(ctx);

  auto sym = *ob_cast_symbol(key);
  auto slots = ob_cast_slots(receiver);

  auto len = ql_array_length(&slots->slots, sizeof(ob_Slot));
  for (size_t i = 0; i < len; i++) {
    auto slot = (ob_Slot *)ql_array_at(&slots->slots, sizeof(ob_Slot), i);

    if (slot->key == sym) {
      slot->value = value;
    }
  }

  return false;
}

static bool slots_at(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto key = ob_pop(ctx);

  auto sym = *ob_cast_symbol(key);
  auto slots = ob_cast_slots(receiver);

  ob_Obj obj = nullptr;

  auto len = ql_array_length(&slots->slots, sizeof(ob_Slot));
  for (size_t i = 0; i < len; i++) {
    auto slot = (ob_Slot *)ql_array_at(&slots->slots, sizeof(ob_Slot), i);

    if (slot->key == sym) {
      ob_push(ctx, slot->value);
      return true;
    }
  }

  // TODO: raise an exn if we found nothing
  ob_push(ctx, obj);

  return true;
}

void oblib_load_slots(ob_Ctx ctx) {
  ob_add_methods(ctx, ctx->proto.slots,
                 (ob_MethodEntry[]){{"at:put:", slots_at_put},
                                    {"at:", slots_at},
                                    OB_METHODS_END});
}
