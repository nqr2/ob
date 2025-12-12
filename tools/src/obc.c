#include <ob/Array.h>
#include <ob/Context.h>
#include <ob/Parse.h>
#include <ob/Serial.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

void dofile(ob_Context ctx, const char *path) {
  auto file = fopen(path, "r");

  if (file == NULL) {
    return;
  }

  auto data = (ob_Array){};
  auto length = 0L;

  fseek(file, 0, SEEK_END);

  length = ftell(file);

  rewind(file);

  obarr_init(&data, ctx->allocator);
  obarr_reserve(&data, length);
  data.size = length;

  fread(data.data, sizeof(char), data.size, file);

  auto method = ob_load(ctx, data.size, data.data);

  auto srl = (ob_Serial){};
  obsrl_init(&srl, ctx);

  obsrl_write(&srl, method);
  fwrite(srl.buffer.data, sizeof(uint8_t), srl.buffer.size, stdout);

  obsrl_free(&srl);

  // exit:
  obarr_free(&data);
  fclose(file);
}

int main(int argn, char *argv[]) {
  auto alloc = oballoc_create();

  auto ctx = obctx_create(&alloc);

  for (int i = 1; i < argn; i++) {
    dofile(ctx, argv[i]);
  }

  obctx_destroy(ctx);

  return 0;
}
