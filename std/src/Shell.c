#include <ob/Context.h>
#include <ob/Parse.h>
#include <ob/lib/Shell.h>

#include "Shell.h"

static void run(ob_Context ctx, unsigned long length, const char *data) {
  ob_run(ctx, length, (void *)data);
  obarr_clear(&ctx->stack);
}

#define RUN(C, M) run((C), LENGTH_##M, DATA_##M)

void oblib_load_shell(ob_Context ctx) {
  (void)ctx;

  ob_ObjSlots *shell = obobj_get_data(ctx->known.shell);

  auto sym_true = obstr_create_literal(ctx, "true");
  obtbl_set(&shell->slots, obstr_get_hash(ctx, sym_true), ctx->known.o_true);

  auto sym_false = obstr_create_literal(ctx, "false");
  obtbl_set(&shell->slots, obstr_get_hash(ctx, sym_false), ctx->known.o_false);

  RUN(ctx, Shell);
}
