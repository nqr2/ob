#include "Array.h"
#include "Context.h"
#include "Parse.h"
#include "Serial.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

void dofile(Context ctx, const char *path) {
  auto file = fopen(path, "r");

  if (file == NULL) {
    return;
  }

  auto data = (Array){};
  auto length = 0L;

  fseek(file, 0, SEEK_END);

  length = ftell(file);

  rewind(file);

  arr_init(&data, ctx->allocator);
  arr_reserve(&data, length);
  data.size = length;

  fread(data.data, sizeof(char), data.size, file);

  auto method = load_file(ctx, data.size, data.data);

  auto srl = (Serial){};
  srl_init(&srl, ctx);

  srl_write(&srl, method);
  fwrite(srl.buffer.data, sizeof(uint8_t), srl.buffer.size, stdout);

  srl_free(&srl);

  // exit:
  arr_free(&data);
  fclose(file);
}

int main(int argn, char *argv[]) {
  auto alloc = get_libc_allocator();

  auto ctx = ctx_create(&alloc);

  for (int i = 1; i < argn; i++) {
    dofile(ctx, argv[i]);
  }

  ctx_destroy(ctx);

  return 0;
}
