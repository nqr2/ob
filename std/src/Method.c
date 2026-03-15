#include <ob/bits/AddMethods.h>
#include <ob/core/Bytecode.h>
#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>
#include <ob/lib/Method.h>

extern void invoke(ob_Ctx ctx, ob_Obj invoked, ob_Obj recv, size_t n_args);

static bool method_call(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto operand = ob_pop(ctx);

  auto args = ob_cast_array(operand);

  size_t length = ql_array_length(args, sizeof(ob_Obj));

  for (size_t i = 0; i < length; i++) {
    auto item = *(ob_Obj *)ql_array_at(args, sizeof(ob_Obj), i);

    ob_push(ctx, item);
  }

  obctx_enter_activation(ctx, receiver, nullptr);
  invoke(ctx, receiver, nullptr, length);
  obctx_leave_activation(ctx);

  return true;
}

static bool method_callin(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto operand = ob_pop(ctx);
  auto recv = ob_pop(ctx);

  auto args = ob_cast_array(operand);

  size_t length = ql_array_length(args, sizeof(ob_Obj));

  for (size_t i = 0; i < length; i++) {
    auto item = *(ob_Obj *)ql_array_at(args, sizeof(ob_Obj), i);

    ob_push(ctx, item);
  }

  obctx_enter_activation(ctx, receiver, recv);
  invoke(ctx, receiver, recv, length);
  obctx_leave_activation(ctx);

  return true;
}

void oblib_load_method(ob_Ctx ctx) {
  ob_add_methods(ctx, ctx->proto.method,
                 (ob_MethodEntry[]){{"call:", method_call},
                                    {"call-in:with:", method_callin},
                                    OB_METHODS_END});
}
