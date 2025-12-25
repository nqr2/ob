#include <ob/Context.h>
#include <ob/lib/Shell.h>

void oblib_load_shell(ob_Context ctx) {
  (void)ctx;

  auto shell = ob_cast_slots(ctx->known.shell);

  auto sym_true = obstr_create_literal(ctx, "true");
  obtbl_set(&shell->slots, obstr_get_hash(ctx, sym_true), ctx->known.o_true);

  auto sym_false = obstr_create_literal(ctx, "false");
  obtbl_set(&shell->slots, obstr_get_hash(ctx, sym_false), ctx->known.o_false);
}
