#ifndef OB_CORE_ASSERT_H_INCLUDED
#define OB_CORE_ASSERT_H_INCLUDED

/*
 * Copyright (C) 2025 nqr2
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
 * @brief Assertions.
 */

#define ASSERT(Condition, Message, ...)                                        \
  do {                                                                         \
    if (!(Condition)) {                                                        \
      obassert__report(__FILE__, __LINE__, __func__, #Condition);              \
      obassert__message(Message "\n" __VA_OPT__(, ) __VA_ARGS__);              \
      obassert__fail();                                                        \
    }                                                                          \
  } while (false)

#define ASSERT_NONNULL(P) ASSERT(P != NULL, "unexpected NULL: %p", (P))

#define ASSERT_NULL(P) ASSERT(P == NULL, "unexpected non-NULL: %p", (P))

typedef void (*FnAssertFailure)();

void obassert__report(const char *file, int line, const char *function,
                      const char *condition);

void obassert__message(const char *message, ...);

void obassert__fail();

void obassert_add_handler(FnAssertFailure fail);

#endif
