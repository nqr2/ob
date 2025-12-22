#include "ob/Array.h"
#include "ob/Bytecode.h"
#include "ob/Object.h"
#include <ob/Context.h>
#include <ob/bits/AddMethods.h>
#include <ob/lib/Method.h>

static bool method_call(ob_Context ctx) {
  ob_ObjActivation *activation = obobj_get_data(ctx->activation);
  auto receiver = activation->receiver;
  auto operand = obctx_pop(ctx);

  ob_Array *args = obobj_get_data(operand);
  ob_ObjMethod *method = obobj_get_data(receiver);

  size_t length = obarr_length(args, sizeof(ob_Str));

  obctx_enter_activation(ctx, receiver, receiver);

  activation = obobj_get_data(ctx->activation);
  ob_ObjSlots *env = obobj_get_data(activation->env);

  for (size_t i = 0; i < length; i++) {
    auto param = (ob_Str *)obarr_at(&method->parameters, sizeof(ob_Str), i);
    auto item = (ob_Obj *)obarr_at(args, sizeof(ob_Obj), i);

    obtbl_set(&env->slots, obstr_get_hash(ctx, *param), *item);
  }

  obbc_run(ctx, method->bytecode.size, method->bytecode.data);
  obctx_leave_activation(ctx);

  return true;
}

void oblib_load_method(ob_Context ctx) {
  ob_add_methods(ctx, ctx->proto.method,
                 (ob_MethodEntry[]){{"call:", method_call}, OB_METHODS_END});
}
