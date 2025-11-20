#ifndef TAP_TAP_H_INCLUDED
#define TAP_TAP_H_INCLUDED

#include <stddef.h>

typedef void (*FnTest)();

typedef struct {
  const char *name;
  FnTest body;
  bool should_fail;
} Test;

#define SUITE_END ((Test){.name = NULL, .body = NULL, .should_fail = false})

void skip();
void skip_with(const char *reason);

void fail();
void fail_with(const char *reason);

void bailout();

void test(const Test *suite);

#ifdef TAP_ASSERT
#undef TAP_ASSERT
#endif

#define TAP_ASSERT(Condition)                                                  \
  do {                                                                         \
    if (!(Condition)) {                                                        \
      fail_with(#Condition);                                                   \
    }                                                                          \
  } while (false)

#endif
