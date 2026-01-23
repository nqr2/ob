#include "ob/Array.h"
#include "ob/Object.h"
#include <ob/bits/AddMethods.h>
#include <ob/lib/Activation.h>

#include <ob/Context.h>

static bool act__var(ob_Context ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto value = ob_pop(ctx);
  auto key = ob_pop(ctx);

  auto sym = ob_cast_symbol(key);
  auto act = ob_cast_activation(receiver);
  auto env = ob_cast_slots(act->env);

  auto hash = obstr_get_hash(ctx, *sym);

  ql_table_set(&env->slots, hash, value);

  return false;
}

static bool act__self(ob_Context ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto act = ob_cast_activation(receiver);
  ob_push(ctx, act->receiver);
  return true;
}

static bool act__env(ob_Context ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto act = ob_cast_activation(receiver);
  ob_push(ctx, act->env);
  return true;
}

static void unpack(ob_Context ctx, ob_Obj args) {
  auto data = ob_cast_array(args);

  auto len = ql_array_length(data, sizeof(ob_Obj));

  for (size_t i = 0; i < len; i++) {
    auto obj = *(ob_Obj *)ql_array_at(data, sizeof(ob_Obj), i);
    ob_push(ctx, obj);
  }
}

static bool act__dnuw(ob_Context ctx) {
  auto act = ob_cast_activation(ctx->this_activation);

  auto selector = ob_pop(ctx);
  auto args = ob_pop(ctx);

  auto sel = *ob_cast_symbol(selector);

  if (ob_get_slot(ctx, NULL, act->env, sel)) {
    unpack(ctx, args);
    ob_send(ctx, act->env, sel);
  }

  if (ob_get_slot(ctx, NULL, act->receiver, sel)) {
    unpack(ctx, args);
    ob_send(ctx, act->receiver, sel);
  }

  return true;
}

void oblib_load_activation(ob_Context ctx) {
  ob_add_methods(ctx, ctx->proto.slots,
                 (ob_MethodEntry[]){{"var:is:", act__var},
                                    {"self", act__self},
                                    {"environment", act__env},
                                    {"doesNotUnderstand:with:", act__dnuw},
                                    OB_METHODS_END});
}
