#include <ob/bits/AddMethods.h>
#include <ob/lib/String.h>

#include <ob/Context.h>

static bool str_intern(ob_Context ctx) {
  auto receiver = obctx_get_receiver(ctx);

  auto str = ob_cast_string(receiver);
  auto obj = obctx_alloc_symbol(ctx, *str);

  obctx_push(ctx, obj);

  return true;
}

static bool str_length(ob_Context ctx) {
  auto receiver = obctx_get_receiver(ctx);

  auto str = ob_cast_string(receiver);
  auto len = obstr_get_length(*str);
  auto obj = obctx_alloc_number(ctx, obnum_of_int((int64_t)len));

  obctx_push(ctx, obj);

  return true;
}

static bool str_concat(ob_Context ctx) {
  auto receiver = obctx_get_receiver(ctx);
  auto operand = obctx_pop(ctx);

  auto left = *ob_cast_string(receiver);
  auto right = *ob_cast_string(operand);

  auto result = obstr_concat(ctx, left, right);
  auto obj = obctx_alloc_string(ctx, result);

  obctx_push(ctx, obj);

  return true;
}

void oblib_load_string(ob_Context ctx) {
  ob_add_methods(ctx, ctx->proto.string,
                 (ob_MethodEntry[]){{"intern", str_intern},
                                    {"length", str_length},
                                    {"+", str_concat},
                                    OB_METHODS_END});
}
