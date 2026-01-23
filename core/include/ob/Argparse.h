#ifndef QL_ARGPARSE_H_INCLUDED
#define QL_ARGPARSE_H_INCLUDED

/*
 * Copyright (C) 2025-2026 nqr2
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 */

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
  const char *long_name;

  /// A description for this option.
  const char *description;

  /// A value to be set when handled. @see ob_FlagKind for what pointers to use.
  void *target;
} ql_Flag;

#define QL_FLAGS_END ((ql_Flag){})

typedef void (*ql_FnPositionalArgument)(void *userdata, const char *argument);

typedef struct {
  const char *description;

  size_t length;
  const ql_Flag *flags;

  /// A callback for positional arguments.
  ql_FnPositionalArgument positional_arg;

  /// Data to pass to this callback.
  void *userdata;
} ql_Parser;

/** @brief Constructs a flag.
 */
ql_Flag ql_create_flag(char short_name, const char *long_name, ql_FlagType kind,
                       void *pointer);

/** @brief Constructs a parser.
 *  @param flags An array of flags, that must have @ref OB_FLAGS_END at the end.
 */
ql_Parser ql_create_parser(const ql_Flag *flags);

/** @brief Parses an argument list.
 * @returns The index of the first invalid argument, or @p length if successful.
 */
size_t ql_parse(const ql_Parser *parser, size_t length, const char **args);

#endif
