#include <ob/Argparse.h>
#include <ob/Array.h>
#include <ob/Context.h>
#include <ob/Log.h>
#include <ob/Parse.h>
#include <ob/Serial.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

void dofile(void *udata, const char *path) {
  ob_Context ctx = udata;
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

void dostring(ob_Context ctx, const char *input) {
  auto length = strlen(input);

  auto method = ob_load(ctx, length, input);

  auto srl = (ob_Serial){};
  obsrl_init(&srl, ctx);

  obsrl_write(&srl, method);
  fwrite(srl.buffer.data, sizeof(uint8_t), srl.buffer.size, stdout);

  obsrl_free(&srl);
}

int main(int argn, const char *argv[]) {
  auto log = oblog_create_handler();
  oblog_set_handler(&log);
  oblog_set_level(OB_LOG_DEBUG);

  auto alloc = oballoc_create();

  auto ctx = obctx_create(&alloc);

  const char *instr = NULL;

  auto f_i = obarg_create_flag('e', NULL, OBARG_FLAG_STRING, (void *)&instr);

  auto parser = obarg_create_parser((ob_Flag[]){f_i});
  parser.userdata = ctx;
  parser.positional_arg = dofile;

  size_t arg_index = 0;

  do {
    arg_index = obarg_parse(&parser, argn - arg_index, argv + arg_index);

    if (instr != NULL) {
      dostring(ctx, instr);
      instr = NULL;
    }
  } while (arg_index < (size_t)argn);

  obctx_destroy(ctx);

  return 0;
}
