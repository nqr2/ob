#include <ob/Std.h>

#include <ob/lib/Activation.h>
#include <ob/lib/Method.h>
#include <ob/lib/Object.h>
#include <ob/lib/Shell.h>
#include <ob/lib/Slots.h>
#include <ob/lib/String.h>

#include <ob/lib/World.h>

void oblib_load_all(ob_Context ctx) {
  oblib_load_object(ctx);
  oblib_load_method(ctx);
  oblib_load_string(ctx);
  oblib_load_slots(ctx);
  oblib_load_activation(ctx);
  oblib_load_shell(ctx);

  oblib_load_world(ctx);
}
