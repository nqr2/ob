#ifndef OB_CORE_ARGPARSE_H_INCLUDED
#define OB_CORE_ARGPARSE_H_INCLUDED

#include <stddef.h>

typedef enum {
  OBARG_FLAG_SET,
  OBARG_FLAG_UNSET,
  OBARG_FLAG_INT,
  OBARG_FLAG_STRING,
  OBARG_FLAG_SUBCOMMAND,
} ob_FlagKind;

typedef struct {
  char short_name;
  const char *long_name;
  const char *description;
  ob_FlagKind kind;
  void *target;
} ob_Flag;

#define OB_FLAGS_END ((ob_Flag){})

typedef void (*ob_FnPositionalArgument)(void *userdata, const char *argument);

typedef struct Parser {
  const char *description;

  size_t length;
  const ob_Flag *flags;

  ob_FnPositionalArgument positional_arg;
  void *userdata;
} ob_Parser;

ob_Flag obarg_create_flag(char short_name, const char *long_name,
                          ob_FlagKind kind, void *pointer);

ob_Parser obarg_create_parser(const ob_Flag *flags);

size_t obarg_parse(const ob_Parser *parser, size_t length, const char **args);

#endif
