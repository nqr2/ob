#ifndef TAP_TAP_H_INCLUDED
#define TAP_TAP_H_INCLUDED

typedef void (*FnTest)();

typedef struct {
  const char *name;
  FnTest body;
  bool should_fail;
} Test;

void skip();
void skip_with(const char *reason);

void fail();
void fail_with(const char *reason);

void bailout();

void test(const Test *suite);

#ifdef ASSERT
#undef ASSERT
#endif

#define ASSERT(Condition)                                                      \
  do {                                                                         \
    if (!(Condition)) {                                                        \
      fail_with(#Condition);                                                   \
    }                                                                          \
  } while (false)

#endif
