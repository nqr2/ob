#include <ob/Context.h>
#include <ob/bits/AddMethods.h>
#include <ob/lib/String.h>

static bool str_intern(ob_Context ctx) {
  ob_ObjActivation *activation = obobj_get_data(ctx->activation);
  auto receiver = activation->receiver;

  auto str = (ob_Str *)obobj_get_data(receiver);
  auto obj = obctx_alloc_symbol(ctx, *str);

  obctx_push(ctx, obj);

  return true;
}

static bool str_length(ob_Context ctx) {
  ob_ObjActivation *activation = obobj_get_data(ctx->activation);
  auto receiver = activation->receiver;

  auto str = (ob_Str *)obobj_get_data(receiver);
  auto len = obstr_get_length(*str);
  auto obj = obctx_alloc_number(ctx, obnum_of_int((int64_t)len));

  obctx_push(ctx, obj);

  return true;
}

void oblib_load_string(ob_Context ctx) {
  ob_add_methods(ctx, ctx->proto_string,
                 (ob_MethodEntry[]){{"intern", str_intern},
                                    {"length", str_length},
                                    OB_METHODS_END});
}
