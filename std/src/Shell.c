#include <ob/Parse.h>
#include <ob/lib/Shell.h>

#include "Shell.h"
#include "ob/Context.h"

static void run(ob_Context ctx, unsigned long length, const char *data) {
  ob_run(ctx, length, (void *)data);

  (void)obctx_pop(ctx);
}

#define RUN(C, M) run((C), LENGTH_##M, DATA_##M)

void oblib_load_shell(ob_Context ctx) {
  (void)ctx;

  RUN(ctx, Shell);
}
