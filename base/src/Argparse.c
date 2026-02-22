#include <ob/base/Argparse.h>

#include <stdlib.h>
#include <string.h>

ql_Flag ql_create_flag(char short_name, char const *long_name, ql_FlagType kind,
                       void *pointer) {
  return (ql_Flag){
      .short_name = short_name,
      .long_name = long_name,
      .description = NULL,
      .type = kind,
      .target = pointer,
  };
}

ql_Parser ql_create_parser(ql_Flag const *flags) {
  size_t length = 0;

  if (flags != NULL) {
    for (length = 0;; length++) {
      if ((flags[length].long_name == NULL) &&
          (flags[length].short_name == 0)) {
        break;
      }
    }
  }

  return (ql_Parser){.description = NULL, .length = length, .flags = flags};
}

void run_flag(ql_Flag const *flag, size_t *idx, char const *next_arg,
              ql_Parser const **parser) {
  switch (flag->type) {
  case QL_FLAG_SET:
    *(bool *)(flag->target) = true;
    break;
  case QL_FLAG_UNSET:
    *(bool *)(flag->target) = false;
    break;

  case QL_FLAG_INT:
    *idx += 1;
    *(int *)(flag->target) = atoi(next_arg);
    break;

  case QL_FLAG_STRING:
    *idx += 1;
    *(char const **)(flag->target) = next_arg;
    break;

  case QL_FLAG_SUBCOMMAND:
    *parser = flag->target;
    break;
  }
}

static ql_Flag const *parse_arg(ql_Parser const *parser, char const *arg,
                                char const **next) {
  for (size_t i = 0; i < parser->length; i++) {
    auto flag = &parser->flags[i];

    if (arg[0] == '-') {
      if ((flag->long_name != NULL) && (arg[1] == '-')) {
        auto len = strlen(flag->long_name);

        if (strncmp(flag->long_name, arg + 2, len) == 0) {
          char const *val = memchr(arg, '=', strlen(arg));

          if (val != NULL) {
            *next = val + 1;
          }

          return flag;
        }
      }

      if ((flag->short_name != 0) && (arg[1] == flag->short_name)) {
        if (arg[2] != 0) {
          *next = arg + 2;
        }

        return flag;
      }
    }
  }

  if (arg[0] != '-' && (parser->positional_arg != NULL)) {
    return (ql_Flag const *)1;
  }

  return NULL;
}

size_t ql_parse(ql_Parser const *parser, size_t length, char const **args) {
  for (size_t i = 1; i < length; i++) {
    if (parser == NULL) {
      return i;
    }

    char const *next = args[i + 1];
    auto flag = parse_arg(parser, args[i], &next);

    if (flag == (ql_Flag const *)1) {
      parser->positional_arg(parser->userdata, args[i]);
    } else if (flag != NULL) {
      run_flag(flag, &i, next, &parser);
    } else {
      return i;
    }
  }

  return length;
}
