#ifndef OB_BASE_TAP_H_INCLUDED
#define OB_BASE_TAP_H_INCLUDED

#include <stddef.h>

typedef void (*ql_FnTest)();

typedef struct {
  char const *name;
  ql_FnTest body;
  bool should_fail;
} ql_Test;

#define QL_PASS(Function)                                                      \
  {.name = #Function, .body = (Function), .should_fail = false}

#define QL_FAIL(Function)                                                      \
  {.name = #Function, .body = (Function), .should_fail = true}

#define QL_FIXTURE(Function)                                                   \
  {.name = NULL, .body = (Function), .should_fail = false}

#define QL_SUITE_END {.name = NULL, .body = NULL, .should_fail = false}

void ql_skip();
void ql_skip_with(char const *reason);

void ql_fail();
void ql_fail_with(char const *reason);

void ql_bailout();

bool ql_test(ql_Test const *suite);

#endif
