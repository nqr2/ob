#ifndef OB_BASE_ARGPARSE_H_INCLUDED
#define OB_BASE_ARGPARSE_H_INCLUDED

/** @file
 *
 * @brief "Declarative" command-line argument parsing.
 */

#include <stddef.h>

/// Decides the action to perform when a flag is handled.
typedef enum {
  /// Set a boolean flag to @c true. @c *flag->target must have type @c bool.
  QL_FLAG_SET,

  /// Set a boolean flag to @c false. @c *flag->target must have type @c bool.
  QL_FLAG_UNSET,

  /// Parse and set an integer value. @c *flag->target must have type @c bool.
  QL_FLAG_INT,

  /// Set a string value. @c *flag->target must have type @c char*.
  QL_FLAG_STRING,

  /// Use another parser for the remaining arguments. @c *flag->target must have
  /// type @ref ob_Parser.
  QL_FLAG_SUBCOMMAND,
} ql_FlagType;

/// A parser option.
typedef struct {
  ql_FlagType type;

  /// The name in a short option (starting with a @-), or @c '@\0' to not parse.
  char short_name;

  /// The name in a long option (starting with a @--), or @c NULL to not parse.
  char const *long_name;

  /// A description for this option.
  char const *description;

  /// A value to be set when handled. @see ob_FlagKind for what pointers to use.
  void *target;
} ql_Flag;

#define QL_FLAGS_END ((ql_Flag){})

typedef void (*ql_FnPositionalArgument)(void *userdata, const char *argument);

typedef struct {
  char const *description;

  size_t length;
  ql_Flag const *flags;

  /// A callback for positional arguments.
  ql_FnPositionalArgument positional_arg;

  /// Data to pass to this callback.
  void *userdata;
} ql_Parser;

/** @brief Constructs a flag.
 */
ql_Flag ql_create_flag(char short_name, char const *long_name, ql_FlagType kind,
                       void *pointer);

/** @brief Constructs a parser.
 *  @param flags An array of flags, that must have @ref OB_FLAGS_END at the end.
 */
ql_Parser ql_create_parser(ql_Flag const *flags);

/** @brief Parses an argument list.
 * @returns The index of the first invalid argument, or @p length if successful.
 */
size_t ql_parse(ql_Parser const *parser, size_t length, char const **args);

#endif
