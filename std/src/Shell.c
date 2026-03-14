#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>
#include <ob/lib/Shell.h>

#include <string.h>

static void add_shell_slot(ob_Ctx ctx, ob_ObjSlots *slots, char const *name,
                           ob_Obj value) {
  auto str = obstr_create(ctx, strlen(name), name);

  obslot_add(&slots->slots, str, value);
}

void oblib_load_shell(ob_Ctx ctx) {
  (void)ctx;

  auto shell = ob_cast_slots(ctx->known.shell);

  add_shell_slot(ctx, shell, "true", ctx->known.o_true);
  add_shell_slot(ctx, shell, "false", ctx->known.o_false);
  add_shell_slot(ctx, shell, "shell", ctx->known.shell);
}
