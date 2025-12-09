#include "Argparse.h"

Flag arg_create_flag(char short_name, const char *long_name, FlagKind kind,
                     void *pointer) {
  return (Flag){
      .short_name = short_name,
      .long_name = long_name,
      .description = NULL,
      .kind = kind,
      .target = pointer,
  };
}

Parser arg_create_parser(const char *executable, size_t length,
                         const Flag *flags) {
  return (Parser){
      .is_subcommand = false,
      .description = NULL,

      .as.command = {.executable = executable,
                     .length = length,
                     .flags = flags},
  };
}

Parser arg_create_subcommand(const char *name, const Parser *parser) {
  return (Parser){.is_subcommand = true,
                  .description = NULL,

                  .as.subcommand = {.name = name, .parser = parser}};
}

void arg_parse(const Parser *parser, size_t length, const char **args) {
  (void)parser;
  (void)length;
  (void)args;
}
