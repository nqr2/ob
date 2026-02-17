
#include <ob/bits/AddMethods.h>
#include <ob/lib/Activation.h>

#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>

static bool act__var(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto value = ob_pop(ctx);
  auto key = ob_pop(ctx);

  auto sym = ob_cast_symbol(key);
  auto env = ob_cast_slots(ob_cast_activation(receiver)->env);

  auto hash = obstr_get_hash(ctx, *sym);

  ql_table_set(&env->slots, hash, value);

  ob_push(ctx, value);
  return true;
}

static bool act__self(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto act = ob_cast_activation(receiver);
  ob_push(ctx, act->receiver);
  return true;
}

static bool act__env(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto act = ob_cast_activation(receiver);
  ob_push(ctx, act->env);
  return true;
}

static void unpack(ob_Ctx ctx, ob_Obj args) {
  auto data = ob_cast_array(args);

  auto len = ql_array_length(data, sizeof(ob_Obj));

  for (size_t i = 0; i < len; i++) {
    auto obj = *(ob_Obj *)ql_array_at(data, sizeof(ob_Obj), i);
    ob_push(ctx, obj);
  }
}

static bool act__cmw(ob_Ctx ctx) {
  auto act = ob_cast_activation(ob_get_receiver(ctx));

  auto selector = ob_pop(ctx);
  auto args = ob_pop(ctx);

  auto sel = *ob_cast_symbol(selector);

  if (ob_get_slot(ctx, NULL, act->env, sel)) {
    unpack(ctx, args);
    ob_send(ctx, act->env, sel);
    return true;
  }

  if (ob_get_slot(ctx, NULL, act->receiver, sel)) {
    unpack(ctx, args);
    ob_send(ctx, act->receiver, sel);
    return true;
  }

  return false;
}

static bool act__frame(ob_Ctx ctx) {
  ob_push(ctx, ctx->this_activation);
  return true;
}

void oblib_load_activation(ob_Ctx ctx) {
  ob_add_methods(ctx, ctx->proto.activation,
                 (ob_MethodEntry[]){{"var:is:", act__var},
                                    {"self", act__self},
                                    {"environment", act__env},
                                    {"callMissing:with:", act__cmw},
                                    {"thisFrame", act__frame},
                                    OB_METHODS_END});
}
