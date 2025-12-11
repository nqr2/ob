#ifndef ARGPARSE_H_INCLUDED
#define ARGPARSE_H_INCLUDED

#include <stddef.h>

typedef enum {
  FLAG_SET,
  FLAG_UNSET,
  FLAG_INT,
  FLAG_STRING,
  FLAG_SUBCOMMAND,
} FlagKind;

typedef struct {
  char short_name;
  const char *long_name;
  const char *description;
  FlagKind kind;
  void *target;
} Flag;

#define FLAGS_END ((Flag){})

typedef void (*FnPositionalArgument)(void *userdata, const char *argument);

typedef struct Parser {
  const char *description;

  size_t length;
  const Flag *flags;

  FnPositionalArgument positional_arg;
  void *userdata;
} Parser;

Flag arg_create_flag(char short_name, const char *long_name, FlagKind kind,
                     void *pointer);

Parser arg_create_parser(const Flag *flags);

size_t arg_parse(const Parser *parser, size_t length, const char **args);

#endif
