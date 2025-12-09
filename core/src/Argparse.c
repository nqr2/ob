#include "Argparse.h"

#include <stdlib.h>
#include <string.h>

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

Parser arg_create_parser(const Flag *flags) {
  size_t length = 0;

  if (flags != NULL) {
    for (length = 0;; length++) {
      if ((flags[length].long_name == NULL) &&
          (flags[length].short_name == 0)) {
        break;
      }
    }
  }

  return (Parser){.description = NULL, .length = length, .flags = flags};
}

void run_flag(const Flag *flag, size_t *idx, const char **args) {
  switch (flag->kind) {
  case FLAG_SET:
    *(bool *)(flag->target) = true;
    break;
  case FLAG_UNSET:
    *(bool *)(flag->target) = false;
    break;

  case FLAG_INT:
    *idx += 1;
    *(int *)(flag->target) = atoi(args[*idx]);
    break;
  }
}

static bool parse_arg(const Parser *parser, size_t *idx, const char *arg,
                      const char **args) {
  for (size_t i = 0; i < parser->length; i++) {
    auto flag = &parser->flags[i];

    if (arg[0] == '-') {
      if ((flag->long_name != NULL) && (arg[1] == '-')) {
        if (strcmp(flag->long_name, arg + 2) == 0) {
          run_flag(flag, idx, args);
        }
      } else {
        if ((flag->short_name != 0) && (arg[1] == flag->short_name)) {
          run_flag(flag, idx, args);
        }
      }
    } else {
      break;
    }
  }

  return true;
}

size_t arg_parse(const Parser *parser, size_t length, const char **args) {
  (void)parser;
  (void)length;
  (void)args;

  for (size_t i = 1; i < length; i++) {
    if (parse_arg(parser, &i, args[i], args)) {
      return i;
    }
  }

  return length;
}
