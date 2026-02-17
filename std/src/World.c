#include <ob/bits/AddMethods.h>
#include <ob/lib/World.h>

#include <ob/core/Bytecode.h>
#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/Serial.h>
#include <ob/core/String.h>

static uint8_t const WORLD[] = {
#embed "World.obc"
};

static ob_Obj deserialize(ob_Ctx ctx) {
  auto srl = (ob_Serial){};
  obsrl_init(&srl, ctx);

  obsrl_load(&srl, sizeof(WORLD), WORLD);
  auto chunk = obsrl_read(&srl);

  obsrl_free(&srl);

  return chunk;
}

void oblib_load_world(ob_Ctx ctx) {
  auto chunk = deserialize(ctx);

  // We have to override this env to set in the shell (for the globals)
  obctx_enter_activation(ctx, chunk, ctx->known.shell);
  ob_cast_activation(ctx->this_activation)->env = ctx->known.shell;

  auto method = ob_cast_method(chunk);
  obbc_run(ctx, method->bytecode.size, method->bytecode.data);

  obctx_leave_activation(ctx);

  ql_array_clear(&ctx->stack);
  ob_gc(ctx, true);
}
