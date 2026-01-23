#ifndef QL_ASSERT_H_INCLUDED
#define QL_ASSERT_H_INCLUDED

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
 * @brief Assertions.
 */

#define QL_ASSERT(Condition, Message, ...)                                     \
  do {                                                                         \
    if (!(Condition)) {                                                        \
      ql_assert__report(__FILE__, __LINE__, __func__, #Condition);             \
      ql_assert__message(Message "\n" __VA_OPT__(, ) __VA_ARGS__);             \
      ql_assert__fail();                                                       \
    }                                                                          \
  } while (false)

#define QL_ASSERT_NONNULL(P) QL_ASSERT((P) != NULL, "unexpected NULL: %p", (P))

#define QL_ASSERT_NULL(P) QL_ASSERT((P) == NULL, "unexpected non-NULL: %p", (P))

typedef void (*ql_FnAssertFailure)();

void ql_assert__report(const char *file, int line, const char *function,
                       const char *condition);

void ql_assert__message(const char *message, ...);

void ql_assert__fail();

void ql_assert_add_handler(ql_FnAssertFailure fail);

#endif
