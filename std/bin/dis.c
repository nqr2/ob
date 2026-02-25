#define OB_LOG_MODULE "dis"

#include <ob/base/Assert.h>
#include <ob/base/Log.h>
#include <ob/core/Bytecode.h>
#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/Serial.h>
#include <ob/core/String.h>

#include <stdbit.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t const *read_int(uint8_t const *bytes, uint64_t *result) {
  auto shift = 0;
  auto byte = *bytes;

  *result = 0;

  do {
    byte = *bytes;
    *result |= (byte & 0x7f) << (shift * 7);
    shift++;
    bytes++;
  } while ((byte & 0x80) != 0);

  return bytes;
}

size_t skip_leb(uint8_t const *data) {
  size_t len = 0;

  for (len = 0; data[len] & 0x80; len++) {
  }

  return len + 1;
}

void dofile(char const *input_path, FILE *input_file, ob_Serial *srl) {
  size_t length = 0;

  auto data = (ql_Array){};
  ql_array_init(&data, srl->ctx->allocator);

  if (input_file != stdin) {
    fseek(input_file, 0, SEEK_END);

    length = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    ql_array_reserve(&data, length * sizeof(char));
    fread(data.data, sizeof(char), length, input_file);
  } else {
    // at least on my machine the above fails, so we check for errors VERY
    // aggressively here

    while (!feof(stdin) && !ferror(stdin)) {
      char tmp = 0;
      fread(&tmp, sizeof(char), 1, stdin);
      ql_array_push(&data, sizeof(char), &tmp);
    }
  }

  obsrl_load(srl, data.size, data.data);
  (void)obsrl_read(srl);
  uint64_t index = 0;
  uint64_t offset = 0;
  ob_Obj obj = NULL;

  while (ql_table_iterate(&srl->identifiers, &index, &offset, (void **)&obj)) {
    printf("\t@ %4lu: %p\n", offset, (void *)obj);
    printf("tag: %d\n", ob_get_tag(obj));

    if (ob_get_tag(obj) == OB_SYMBOL) {
      auto sym = *ob_cast_symbol(obj);
      auto data = obstr_get_data(srl->ctx, sym);
      auto len = obstr_get_length(sym);

      printf("symbol: #'%.*s'\n", (int)len, data);
    }

    if (ob_get_tag(obj) == OB_METHOD) {
      auto method = ob_cast_method(obj);

      printf("param:\n");

      auto len = ql_array_length(&method->parameters, sizeof(ob_Str));
      for (size_t i = 0; i < len; i++) {
        auto param =
            *(ob_Str *)ql_array_at(&method->parameters, sizeof(ob_Str), i);

        printf("  [%zu] = '%.*s'\n", i, (int)obstr_get_length(param),
               obstr_get_data(srl->ctx, param));
      }

      printf("bc:\n");

      len = method->bytecode.size;

      ob_Opcode opcode = 0;
      size_t data = 0;

      auto code = (uint8_t const *)method->bytecode.data;
      auto start = code;

      char const *path = "???";
      char const *this_line = "";
      size_t line = 0;
      size_t column = 0;

      while ((size_t)(code - start) < len) {
        int size = stdc_bit_width(method->bytecode.size) / 4 + 1;

        auto byte = *code;
        auto offset = (size_t)(code - start);
        code = obbc_read_insn(code, &opcode, &data);

        char const *name = "???";

        if (opcode == OB_OP_FILENAME) {
          path = code;
          printf(" in file '%.*s'\n", data, path);
          code += data;
          continue;
        }

        if (opcode == OB_OP_DEBUG) {
          uint64_t dline = 0;
          uint64_t dcol = 0;

          if (data == 0) {
            code = read_int(code, &dcol);
            column += dcol;
          } else {
            code = read_int(code, &dcol);
            code = read_int(code, &dline);

            line += dline;
            column = dcol;

            this_line = code;
            code += data;
          }

          printf(" @ '%s' %lu:%lu - %s\n", path, line + 1, column + 1,
                 this_line);

          continue;
        }

        if (opcode == OB_OP_EXTRA) {
          name = obbc_extopcode_name(data);
        } else {
          name = obbc_opcode_name(opcode);
        }

        printf("  %.*zx : %02x", size, offset, byte);
        printf("\t%s %zd", name, data);

        putchar('\n');
      }

      puts("lit:");

      {
        auto len = ql_array_length(&method->literals, sizeof(ob_Obj));

        for (size_t i = 0; i < len; i++) {
          auto obj =
              *(ob_Obj *)ql_array_at(&method->literals, sizeof(ob_Obj), i);
          printf("  %zu = %p\n", i, (void *)obj);
        }
      }
    }
  }

  ql_array_free(&data);
}

int main(int argn, char *argv[]) {
  auto log = ql_log_create_handler();
  ql_log_set_handler(&log);
  ql_log_set_level(QL_LOG_DEBUG);

  auto file = stdin;
  auto input = "*stdin*";

  if (argn == 2) {
    input = argv[1];
    file = fopen(input, "r");

    QL_ASSERT(file != NULL, "could not open file '%s'", input);
  }

  QL_ASSERT(argn <= 2, "expected 0 or 1 arguments, got %d", argn - 1);

  auto alloc = ql_alloc_create();
  auto ctx = ob_create(&alloc);

  auto srl = (ob_Serial){};
  obsrl_init(&srl, ctx);

  dofile(input, file, &srl);

  obsrl_free(&srl);

  ob_destroy(ctx);

  if (file != stdin) {
    fclose(file);
  }

  return 0;
}
