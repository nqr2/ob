#include <ob/base/Assert.h>
#include <ob/base/Log.h>
#include <ob/bits/AddMethods.h>
#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>
#include <ob/lib/Activation.h>

static bool act_var_is(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto value = ob_pop(ctx);
  auto key = ob_pop(ctx);

  auto sym = *ob_cast_symbol(key);
  auto env = ob_cast_slots(ob_cast_activation(receiver)->env);

  auto slot = (ob_Slot){.key = sym, .value = value};
  ql_array_push(&env->slots, sizeof(slot), &slot);

  ob_push(ctx, value);
  return true;
}

static bool act_self(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto act = ob_cast_activation(receiver);
  ob_push(ctx, act->receiver);
  return true;
}

static bool act_env(ob_Ctx ctx) {
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

static bool act_cmw(ob_Ctx ctx) {
  auto act = ob_cast_activation(ob_get_receiver(ctx));
  auto selector = ob_pop(ctx);
  auto args = ob_pop(ctx);

  auto sel = *ob_cast_symbol(selector);

  while (act != NULL) {
    QL_INFO("try on env");
    if (ob_get_slot(ctx, NULL, act->env, sel)) {
      QL_INFO("found on env");
      unpack(ctx, args);
      ob_send(ctx, act->env, sel);
      return true;
    }

    QL_INFO("try on slot");
    if (ob_get_slot(ctx, NULL, act->receiver, sel)) {
      QL_INFO("found on recv");
      unpack(ctx, args);
      ob_send(ctx, act->receiver, sel);
      return true;
    }

    QL_INFO("try on parent");
    if (act->parent != NULL) {
      act = ob_cast_activation(act->parent);
      continue;
    }

    break;
  }

  QL_ASSERT(false, "method missing: #'%.*s'", sel->length,
            obstr_get_data(ctx, sel));

  return false;
}

static bool act_thisframe(ob_Ctx ctx) {
  ob_push(ctx, ctx->this_activation);
  return true;
}

static bool act_parentframe(ob_Ctx ctx) {
  auto activation = ob_cast_activation(ctx->this_activation);
  ob_push(ctx, activation->parent);
  return true;
}

void oblib_load_activation(ob_Ctx ctx) {
  ob_add_methods(ctx, ctx->proto.activation,
                 (ob_MethodEntry[]){{"var:is:", act_var_is},
                                    {"self", act_self},
                                    {"environment", act_env},
                                    {"call-missing:with:", act_cmw},
                                    {"this-frame", act_thisframe},
                                    {"parent-frame", act_parentframe},
                                    OB_METHODS_END});
}
