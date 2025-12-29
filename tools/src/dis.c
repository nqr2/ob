#include "ob/Array.h"
#include "ob/Object.h"
#include <ob/Assert.h>
#include <ob/Bytecode.h>
#include <ob/Context.h>
#include <ob/Serial.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void dofile(const char *input_path, ob_Serial *srl) {
  auto input_file = fopen(input_path, "rb");

  if (input_file == NULL) {
    return;
  }

  size_t length = 0;

  fseek(input_file, 0, SEEK_END);

  length = ftell(input_file);

  rewind(input_file);

  unsigned char *data = calloc(length, sizeof(char));
  fread(data, sizeof(char), length, input_file);

  obsrl_load(srl, length, data);
  (void)obsrl_read(srl);
  uint64_t index = 0;
  uint64_t offset = 0;
  ob_Obj obj = NULL;

  while (obtbl_iterate(&srl->identifiers, &index, &offset, (void **)&obj)) {
    printf("\t@ %4lu: %p\n", offset, (void *)obj);
    printf("tag: %d\n", ob_get_tag(obj));

    if (ob_get_tag(obj) == OB_SYMBOL) {
      auto sym = *ob_cast_symbol(obj);
      auto data = obstr_get_data(srl->ctx, sym);
      auto len = obstr_get_length(sym);

      printf("symbol: #'%.*s'\n", len, data);
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
        case OBBC_PUSH_LITERAL:
          name = "LIT";
          break;
        case OBBC_SEND:
          name = "SND";
          break;
        case OBBC_IMPLICIT_SEND:
          name = "IMP";
          break;
        case OBBC_EXTEND:
          break;
        case OBBC_RETURN:
          name = "RET";
          break;
        case OBBC_ARRAY:
          name = "ARR";
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
        auto len = obarr_length(&method->literals, sizeof(ob_Obj));

        for (size_t i = 0; i < len; i++) {
          auto obj = *(ob_Obj *)obarr_at(&method->literals, sizeof(ob_Obj), i);
          printf("  %zu = %p\n", i, (void *)obj);
        }
      }
    }
  }

  free(data);

  fclose(input_file);
}

int main(int argn, char *argv[]) {
  ASSERT(argn == 2, "expected 1 argument, got %d", argn - 1);

  auto input = argv[1];

  auto alloc = oballoc_create();
  auto ctx = obctx_create(&alloc);

  auto srl = (ob_Serial){};
  obsrl_init(&srl, ctx);

  dofile(input, &srl);

  obsrl_free(&srl);

  obctx_destroy(ctx);

  return 0;
}
