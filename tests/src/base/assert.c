#include <Test.h>

#include <ob/base/Assert.h>

DEFTEST(fail_assert_false) {
  QL_ASSERT(false, "This always fails");
}

DEFTEST(assert_true_ok) {
  QL_ASSERT(true, "This never fails");
}

DEFSUITE(assert, SUITES(),
         TESTS(TEST(fail_assert_false), TEST(assert_true_ok)));
