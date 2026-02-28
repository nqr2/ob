#include <ob/base/Argparse.h>
#include <ob/base/Log.h>
#include <ob/core/Context.h>
#include <ob/core/Parse.h>
#include <ob/core/Serial.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct Userdata {
  ob_Ctx context;
  ob_Rdr reader;
};

void dofile(void *udata, char const *path) {
  auto data = (struct Userdata *)udata;

  auto ctx = data->context;
  auto rdr = data->reader;

  auto file = fopen(path, "r");

  if (file == NULL) {
    return;
  }

  auto contents = (ql_Array){};
  auto length = 0L;

  fseek(file, 0, SEEK_END);

  length = ftell(file);

  rewind(file);

  ql_array_init(&contents, ctx->allocator);
  ql_array_reserve(&contents, length);
  contents.size = length;

  fread(contents.data, sizeof(char), contents.size, file);

  obrdr_load(rdr, path, contents.size, contents.data);

  // exit:
  ql_array_free(&contents);
  fclose(file);
}

void dostring(struct Userdata *data, char const *input) {
  auto ctx = data->context;
  auto rdr = data->reader;

  auto length = strlen(input);

  obrdr_load(rdr, "*commandline*", length, input);
}

int main(int argn, char const *argv[]) {
  auto log = ql_log_create_handler();
  ql_log_set_handler(&log);
  ql_log_set_level(QL_LOG_DEBUG);

  auto alloc = ql_alloc_create();

  auto ctx = ob_create(&alloc);

  char const *instr = NULL;

  auto f_i = ql_create_flag('e', NULL, QL_FLAG_STRING, (void *)&instr);

  auto rdr = obrdr_create(ctx);

  auto data = (struct Userdata){.context = ctx, .reader = rdr};

  auto parser = ql_create_parser((ql_Flag[]){f_i});
  parser.userdata = &data;
  parser.positional_arg = dofile;

  size_t arg_index = 0;

  do {
    arg_index = ql_parse(&parser, argn - arg_index, argv + arg_index);

    if (instr != NULL) {
      dostring(&data, instr);
      instr = NULL;
      arg_index++;
    }
  } while (arg_index < (size_t)argn);

  auto srl = (ob_Serial){};
  obsrl_init(&srl, ctx);

  obsrl_write(&srl, obrdr_get_method(rdr));
  fwrite(srl.buffer.data, sizeof(uint8_t), srl.buffer.size, stdout);

  obsrl_free(&srl);

  obrdr_free(rdr);

  // TODO: this

  ob_destroy(ctx);

  return 0;
}
