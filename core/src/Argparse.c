#include <ob/Argparse.h>

#include <stdlib.h>
#include <string.h>

ob_Flag obarg_create_flag(char short_name, const char *long_name,
                          ob_FlagKind kind, void *pointer) {
  return (ob_Flag){
      .short_name = short_name,
      .long_name = long_name,
      .description = NULL,
      .kind = kind,
      .target = pointer,
  };
}

ob_Parser obarg_create_parser(const ob_Flag *flags) {
  size_t length = 0;

  if (flags != NULL) {
    for (length = 0;; length++) {
      if ((flags[length].long_name == NULL) &&
          (flags[length].short_name == 0)) {
        break;
      }
    }
  }

  return (ob_Parser){.description = NULL, .length = length, .flags = flags};
}

void run_flag(const ob_Flag *flag, size_t *idx, const char **args,
              const ob_Parser **parser) {
  switch (flag->kind) {
  case OBARG_FLAG_SET:
    *(bool *)(flag->target) = true;
    break;
  case OBARG_FLAG_UNSET:
    *(bool *)(flag->target) = false;
    break;

  case OBARG_FLAG_INT:
    *idx += 1;
    *(int *)(flag->target) = atoi(args[*idx]);
    break;

  case OBARG_FLAG_STRING:
    *idx += 1;
    *(const char **)(flag->target) = args[*idx];
    break;

  case OBARG_FLAG_SUBCOMMAND:
    *parser = flag->target;
    break;
  }
}

static const ob_Flag *parse_arg(const ob_Parser *parser, const char *arg) {
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
    return (const ob_Flag *)1;
  }

  return NULL;
}

size_t obarg_parse(const ob_Parser *parser, size_t length, const char **args) {
  for (size_t i = 1; i < length; i++) {
    if (parser == NULL) {
      return i;
    }

    auto flag = parse_arg(parser, args[i]);

    if (flag == (const ob_Flag *)1) {
      parser->positional_arg(parser->userdata, args[i]);
    } else if (flag != NULL) {
      run_flag(flag, &i, args, &parser);
    } else {
      return i;
    }
  }

  return length;
}
