#include <ob/bits/AddMethods.h>
#include <ob/lib/World.h>

#include <ob/Context.h>
#include <ob/Parse.h>

#include "Boolean.h"
#include "Shell.h"

static void run(ob_Context ctx, unsigned long length,
                const unsigned char *data) {
  ob_run(ctx, length, (void *)data);
}

#define RUN(C, M) run((C), LENGTH_##M, DATA_##M)

void oblib_load_world(ob_Context ctx) {
  RUN(ctx, Shell);
  RUN(ctx, Boolean);

  ql_array_clear(&ctx->stack);
}
