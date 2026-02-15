#include <ob/bits/AddMethods.h>
#include <ob/lib/World.h>

#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>

static uint8_t const WORLD[] = {
#embed "World.obc"
};

void oblib_load_world(ob_Ctx ctx) {
  ob_run(ctx, sizeof(WORLD), WORLD);
  ql_array_clear(&ctx->stack);
}
