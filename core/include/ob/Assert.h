#ifndef OB_CORE_ASSERT_H_INCLUDED
#define OB_CORE_ASSERT_H_INCLUDED

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
