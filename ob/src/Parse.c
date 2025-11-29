#include "Parse.h"

#include "Assert.h"
#include "Bytecode.h"
#include "Macros.h"
#include "Object.h"

const char *read(Context ctx, Obj *output, size_t length, const char *text) {
  IGNORE ctx;
  IGNORE output;
  IGNORE length;
  IGNORE text;

  return text;
}

Obj load_file(Context ctx, size_t length, const char *text) {
  Obj closure = NULL;

  while (length > 0) {
    Obj statement = NULL;
    auto next = read(ctx, &statement, length, text);

    ASSERT(next != text, "didn't read anything just now");

    length -= next - text;
    text = next;

    // TODO: add a `statement` to the `closure`.
  }

  return closure;
}

void run_file(Context ctx, size_t length, const char *text) {
  auto chunk = load_file(ctx, length, text);
  ObjMethod *method = obj_payload(chunk);
  bc_run(ctx, method->bytecode.size, method->bytecode.data);
}
