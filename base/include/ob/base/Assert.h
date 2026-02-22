#ifndef OB_BASE_ASSERT_H_INCLUDED
#define OB_BASE_ASSERT_H_INCLUDED

/** @file
 *
 * @brief Assertions.
 */

#define OB_ASSERT(Condition, Message, ...)                                     \
  do {                                                                         \
    if (!(Condition)) {                                                        \
      ql_assert__report(__FILE__, __LINE__, __func__, #Condition);             \
      ql_assert__message(Message "\n" __VA_OPT__(, ) __VA_ARGS__);             \
      ql_assert__fail();                                                       \
    }                                                                          \
  } while (false)

#define QL_ASSERT(...) OB_ASSERT(__VA_ARGS__)

#define OB_ASSERT_NONNULL(P) OB_ASSERT((P) != NULL, "unexpected NULL for " #P)

#define OB_ASSERT_NULL(P) OB_ASSERT((P) == NULL, "unexpected non-NULL: %p", (P))

typedef void (*ql_FnAssertFailure)();

void ql_assert__report(char const *file, int line, char const *function,
                       char const *condition);

void ql_assert__message(char const *message, ...);

[[noreturn]]
void ql_assert__fail();

void ql_assert_add_handler(ql_FnAssertFailure fail);

#endif
