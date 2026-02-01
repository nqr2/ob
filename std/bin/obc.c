#include <ob/core/Context.h>
#include <ob/core/Serial.h>

#include <ql/Argparse.h>
#include <ql/Log.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

void dofile(void *udata, const char *path) {
  ob_Ctx ctx = udata;
  auto file = fopen(path, "r");

  if (file == NULL) {
    return;
  }

  auto data = (ql_Array){};
  auto length = 0L;

  fseek(file, 0, SEEK_END);

  length = ftell(file);

  rewind(file);

  ql_array_init(&data, ctx->allocator);
  ql_array_reserve(&data, length);
  data.size = length;

  fread(data.data, sizeof(char), data.size, file);

  auto method = ob_load_ext(ctx, path, data.size, data.data);

  auto srl = (ob_Serial){};
  obsrl_init(&srl, ctx);

  obsrl_write(&srl, method);
  fwrite(srl.buffer.data, sizeof(uint8_t), srl.buffer.size, stdout);

  obsrl_free(&srl);

  // exit:
  ql_array_free(&data);
  fclose(file);
}

void dostring(ob_Ctx ctx, const char *input) {
  auto length = strlen(input);

  auto method = ob_load_ext(ctx, "*command*", length, input);

  auto srl = (ob_Serial){};
  obsrl_init(&srl, ctx);

  obsrl_write(&srl, method);
  fwrite(srl.buffer.data, sizeof(uint8_t), srl.buffer.size, stdout);

  obsrl_free(&srl);
}

int main(int argn, const char *argv[]) {
  auto log = ql_log_create_handler();
  ql_log_set_handler(&log);
  ql_log_set_level(QL_LOG_DEBUG);

  auto alloc = ql_alloc_create();

  auto ctx = ob_create(&alloc);

  const char *instr = NULL;

  auto f_i = ql_create_flag('e', NULL, QL_FLAG_STRING, (void *)&instr);

  auto parser = ql_create_parser((ql_Flag[]){f_i});
  parser.userdata = ctx;
  parser.positional_arg = dofile;

  size_t arg_index = 0;

  do {
    arg_index = ql_parse(&parser, argn - arg_index, argv + arg_index);

    if (instr != NULL) {
      dostring(ctx, instr);
      instr = NULL;
    }
  } while (arg_index < (size_t)argn);

  ob_destroy(ctx);

  return 0;
}
