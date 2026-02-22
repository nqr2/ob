#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>
#include <ob/lib/Shell.h>

void oblib_load_shell(ob_Ctx ctx) {
  (void)ctx;

  auto shell = ob_cast_slots(ctx->known.shell);

  auto sym_true = obstr_create_literal(ctx, "true");
  ql_table_set(&shell->slots, obstr_get_hash(ctx, sym_true), ctx->known.o_true);

  auto sym_false = obstr_create_literal(ctx, "false");
  ql_table_set(&shell->slots, obstr_get_hash(ctx, sym_false),
               ctx->known.o_false);

  auto sym_shell = obstr_create_literal(ctx, "shell");
  ql_table_set(&shell->slots, obstr_get_hash(ctx, sym_shell), ctx->known.shell);
}
