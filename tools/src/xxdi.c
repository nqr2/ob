#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

// This emulates the binary->header conversion from `xxd -i`, thus the name.

void dofile(const char *input_path, const char *output_path,
            const char *symbol) {
  auto input_file = fopen(input_path, "rb");
  auto output_file = fopen(output_path, "wb");

  if (input_file == NULL) {
    return;
  }

  size_t length = 0;

  fseek(input_file, 0, SEEK_END);

  length = ftell(input_file);

  rewind(input_file);

  unsigned char *data = calloc(length, sizeof(char));
  fread(data, sizeof(char), length, input_file);

  fprintf(output_file, "const unsigned long LENGTH_%s = %lu\n", symbol, length);
  fprintf(output_file, "const char* DATA_%s = \"", symbol);

  for (size_t i = 0; i < length; i++) {
    auto chr = data[i];

    if (iscntrl(chr) || chr > 128) {
      switch (chr) {
      case '\n':
        fprintf(output_file, "\\n");
        break;
      default:
        fprintf(output_file, "\\x%02x", chr);
      }
    } else {
      if (chr == '"') {
        fwrite("\\\"", 1, 4, output_file);
      } else {
        fwrite(&chr, 1, 1, output_file);
      }
    }
  }

  fprintf(output_file, "\";\n");

  free(data);

  fclose(input_file);
  fclose(output_file);
}

int main(int argn, const char *argv[]) {
  if (argn != 4) {
    printf("usage: %s <input> <output> <symbol>\n", argv[0]);
    return 1;
  }

  auto input = argv[1];
  auto output = argv[2];
  auto symbol = argv[3];

  dofile(input, output, symbol);

  return 0;
}
