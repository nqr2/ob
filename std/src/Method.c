#include <ob/bits/AddMethods.h>
#include <ob/lib/Method.h>

#include <ob/core/Bytecode.h>
#include <ob/core/Context.h>

static bool method_call(ob_Context ctx) {
  auto activation = ob_cast_activation(ctx->this_activation);
  auto receiver = activation->receiver;
  auto operand = ob_pop(ctx);

  auto args = ob_cast_array(operand);
  auto method = ob_cast_method(receiver);

  size_t length = ql_array_length(args, sizeof(ob_Str));

  obctx_enter_activation(ctx, receiver, receiver);

  activation = ob_cast_activation(ctx->this_activation);
  auto env = ob_cast_slots(activation->env);

  for (size_t i = 0; i < length; i++) {
    auto param = (ob_Str *)ql_array_at(&method->parameters, sizeof(ob_Str), i);
    auto item = (ob_Obj *)ql_array_at(args, sizeof(ob_Obj), i);

    ql_table_set(&env->slots, obstr_get_hash(ctx, *param), *item);
  }

  obbc_run(ctx, method->bytecode.size, method->bytecode.data);
  obctx_leave_activation(ctx);

  return true;
}

void oblib_load_method(ob_Context ctx) {
  ob_add_methods(ctx, ctx->proto.method,
                 (ob_MethodEntry[]){{"call:", method_call}, OB_METHODS_END});
}
