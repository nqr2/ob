#include <ql/Assert.h>
#include <ql/Tap.h>

void assert_failure() {
  ql_fail_with("assertion failed");
}

void assert_false_fails() {
  QL_ASSERT(false, "This always fails");
}

void assert_true_succeeds() {
  QL_ASSERT(true, "This never fails");
}

const ql_Test SUITE[] = {
    {"assert(false) fails", assert_false_fails, true},
    {"assert(true) succeeds", assert_true_succeeds, false},
    QL_SUITE_END,
};

int main() {
  ql_assert_add_handler(assert_failure);

  ql_test(SUITE);

  return 0;
}
