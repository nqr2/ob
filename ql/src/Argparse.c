#include <ql/Argparse.h>

#include <stdlib.h>
#include <string.h>

ql_Flag ql_create_flag(char short_name, const char *long_name, ql_FlagType kind,
                       void *pointer) {
  return (ql_Flag){
      .short_name = short_name,
      .long_name = long_name,
      .description = NULL,
      .type = kind,
      .target = pointer,
  };
}

ql_Parser ql_create_parser(const ql_Flag *flags) {
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

void run_flag(const ql_Flag *flag, size_t *idx, const char **args,
              const ql_Parser **parser) {
  switch (flag->type) {
  case QL_FLAG_SET:
    *(bool *)(flag->target) = true;
    break;
  case QL_FLAG_UNSET:
    *(bool *)(flag->target) = false;
    break;

  case QL_FLAG_INT:
    *idx += 1;
    *(int *)(flag->target) = atoi(args[*idx]);
    break;

  case QL_FLAG_STRING:
    *idx += 1;
    *(const char **)(flag->target) = args[*idx];
    break;

  case QL_FLAG_SUBCOMMAND:
    *parser = flag->target;
    break;
  }
}

static const ql_Flag *parse_arg(const ql_Parser *parser, const char *arg) {
  for (size_t i = 0; i < parser->length; i++) {
    auto flag = &parser->flags[i];

    if (arg[0] == '-') {
      if ((flag->long_name != NULL) && (arg[1] == '-')) {
        if (strcmp(flag->long_name, arg + 2) == 0) {
          return flag;
        }
      }

      if ((flag->short_name != 0) && (arg[1] == flag->short_name)) {
        return flag;
      }
    }
  }

  if (arg[0] != '-' && (parser->positional_arg != NULL)) {
    return (const ql_Flag *)1;
  }

  return NULL;
}

size_t ql_parse(const ql_Parser *parser, size_t length, const char **args) {
  for (size_t i = 1; i < length; i++) {
    if (parser == NULL) {
      return i;
    }

    auto flag = parse_arg(parser, args[i]);

    if (flag == (const ql_Flag *)1) {
      parser->positional_arg(parser->userdata, args[i]);
    } else if (flag != NULL) {
      run_flag(flag, &i, args, &parser);
    } else {
      return i;
    }
  }

  return length;
}
