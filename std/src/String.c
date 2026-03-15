#include <ob/bits/AddMethods.h>
#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>
#include <ob/lib/String.h>

static bool str_length(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);

  auto str = ob_cast_string(receiver);
  auto len = obstr_get_length(*str);
  auto obj = ob_create_number(ctx, ql_number_of_int((int64_t)len));

  ob_push(ctx, obj);

  return true;
}

static bool str_concat(ob_Ctx ctx) {
  auto receiver = ob_get_receiver(ctx);
  auto operand = ob_pop(ctx);

  auto left = *ob_cast_string(receiver);
  auto right = *ob_cast_string(operand);

  auto result = obstr_concat(ctx, left, right);
  auto obj = ob_wrap_string(ctx, result);

  ob_push(ctx, obj);

  return true;
}

void oblib_load_string(ob_Ctx ctx) {
  ob_add_methods(ctx, ctx->proto.string,
                 (ob_MethodEntry[]){{"length", str_length},
                                    {"+", str_concat},
                                    OB_METHODS_END});
}
