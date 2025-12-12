#include "Tap.h"

#include <ob/Assert.h>

void assert_failure() {
  fail_with("assertion failed");
}

void assert_false_fails() {
  ASSERT(false, "This always fails");
}

void assert_true_succeeds() {
  ASSERT(true, "This never fails");
}

const Test SUITE[] = {
    {"assert(false) fails", assert_false_fails, true},
    {"assert(true) succeeds", assert_true_succeeds, false},
    SUITE_END,
};

int main() {
  obassert_add_handler(assert_failure);

  test(SUITE);

  return 0;
}
