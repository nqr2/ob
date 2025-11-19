#ifndef ASSERT_H_INCLUDED
#define ASSERT_H_INCLUDED

#define ASSERT(Condition, Message, ...)                                        \
  do {                                                                         \
    if (!(Condition)) {                                                        \
      assert__report(__FILE__, __LINE__, __FUNCTION__, #Condition);            \
      assert__message(Message "\n" __VA_OPT__(, ) __VA_ARGS__);                \
      assert__fail();                                                          \
    }                                                                          \
  } while (false)

typedef void (*FnAssertFailure)();

void assert__report(const char *file, int line, const char *function,
                    const char *condition);

void assert__message(const char *message, ...);

void assert__fail();

void assert_add_handler(FnAssertFailure fail);

#endif
