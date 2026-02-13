#include <ob/core/Bytecode.h>
#include <ob/core/Context.h>
#include <ob/core/Object.h>
#include <ob/core/Serial.h>
#include <ob/core/String.h>

#define QL_LOG_MODULE "dis"
#include <ql/Assert.h>
#include <ql/Log.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void dofile(const char *input_path, FILE *input_file, ob_Serial *srl) {
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
      printf("bc:\n");

      auto method = ob_cast_method(obj);
      auto len = method->bytecode.size;

      ob_Opcode opcode = 0;
      size_t data = 0;

      auto code = (const uint8_t *)method->bytecode.data;
      auto start = code;

      while ((size_t)(code - start) < len) {
        auto byte = *code;
        auto offset = (size_t)(code - start);
        code = obbc_read_insn(code, &opcode, &data);

        const char *name = "???";

        switch (opcode) {
        case OB_OP_PUSH:
          name = "literal";
          break;
        case OB_OP_SEND:
          name = "send";
          break;
        case OB_OP_IMPLICIT:
          name = "implicit";
          break;
        case OB_OP_EXTEND:
          break;
        case OB_OP_RETURN:
          name = "return";
          break;
        case OB_OP_ARRAY:
          name = "array";
          break;
        default:
          break;
        }

        printf("  %03zx : %02x", offset, byte);
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
