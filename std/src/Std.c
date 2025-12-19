#include <ob/Std.h>
#include <ob/lib/Method.h>
#include <ob/lib/Object.h>
#include <ob/lib/String.h>

void oblib_load_all(ob_Context ctx) {
  oblib_load_object(ctx);
  oblib_load_method(ctx);
  oblib_load_string(ctx);
}
