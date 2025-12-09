#ifndef ARGPARSE_H_INCLUDED
#define ARGPARSE_H_INCLUDED

#include <stddef.h>

typedef enum {
  FLAG_SET,
  FLAG_UNSET,
} FlagKind;

typedef struct {
  char short_name;
  const char *long_name;
  const char *description;
  FlagKind kind;
  void *target;
} Flag;

typedef struct Parser {
  bool is_subcommand;

  const char *description;

  union {
    struct {
      const char *name;
      const struct Parser *parser;
    } subcommand;

    struct {
      const char *executable;
      size_t length;
      const Flag *flags;
    } command;
  } as;
} Parser;

Flag arg_create_flag(char short_name, const char *long_name, FlagKind kind,
                     void *pointer);

Parser arg_create_parser(const char *executable,
                         size_t length, const Flag *flags);

Parser arg_create_subcommand(const char *name, const Parser *parser);

void arg_parse(const Parser *parser, size_t length, const char **args);

#endif
