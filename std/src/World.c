#include <ob/bits/AddMethods.h>
#include <ob/lib/World.h>

#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>

static const uint8_t SHELL[] = {
#embed "Shell.obc"
};

static const uint8_t BOOLEAN[] = {
#embed "Boolean.obc"
};

static void run(ob_Ctx ctx, unsigned long length, const unsigned char *data) {
  ob_run(ctx, length, (void *)data);
}

#define RUN(C, M) run((C), sizeof(M), (M))

void oblib_load_world(ob_Ctx ctx) {
  RUN(ctx, SHELL);
  RUN(ctx, BOOLEAN);

  ql_array_clear(&ctx->stack);
}
