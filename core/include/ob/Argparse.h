#ifndef OB_CORE_ARGPARSE_H_INCLUDED
#define OB_CORE_ARGPARSE_H_INCLUDED

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
  OBARG_FLAG_SET,

  /// Set a boolean flag to @c false. @c *flag->target must have type @c bool.
  OBARG_FLAG_UNSET,

  /// Parse and set an integer value. @c *flag->target must have type @c bool.
  OBARG_FLAG_INT,

  /// Set a string value. @c *flag->target must have type @c char*.
  OBARG_FLAG_STRING,

  /// Use another parser for the remaining arguments. @c *flag->target must have
  /// type @ref ob_Parser.
  OBARG_FLAG_SUBCOMMAND,
} ob_FlagKind;

/// A parser option.
typedef struct {
  ob_FlagKind kind;

  /// The name in a short option (starting with a @-), or @c '@\0' to not parse.
  char short_name;

  /// The name in a long option (starting with a @--), or @c NULL to not parse.
  const char *long_name;

  /// A description for this option.
  const char *description;

  /// A value to be set when handled. @see ob_FlagKind for what pointers to use.
  void *target;
} ob_Flag;

#define OB_FLAGS_END ((ob_Flag){})

typedef void (*ob_FnPositionalArgument)(void *userdata, const char *argument);

typedef struct Parser {
  const char *description;

  size_t length;
  const ob_Flag *flags;

  /// A callback for positional arguments.
  ob_FnPositionalArgument positional_arg;

  /// Data to pass to this callback.
  void *userdata;
} ob_Parser;

/** @brief Constructs a flag.
 */
ob_Flag obarg_create_flag(char short_name, const char *long_name,
                          ob_FlagKind kind, void *pointer);

/** @brief Constructs a parser.
 *  @param flags An array of flags, that must have @ref OB_FLAGS_END at the end.
 */
ob_Parser obarg_create_parser(const ob_Flag *flags);

/** @brief Parses an argument list.
 * @returns The index of the first invalid argument, or @p length if successful.
 */
size_t obarg_parse(const ob_Parser *parser, size_t length, const char **args);

#endif
