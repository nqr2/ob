// Test harness tests

#include <Test.h>
#include <ob/base/Assert.h>

bool value_set = false;

DEFTEST(always_pass) {
}

DEFTEST(fail_always) {
  fail();
}

DEFTEST(skipped) {
  skip_with("this always skips");
}

DEFFIXTURE(a_fixture) {
  value_set = true;
  return true;
}

DEFTEST(fixture_must_run) {
  OB_ASSERT(value_set == true, "`value_set` must be set to true by a fixture");
}

DEFSUITE(drv, SUITES(),
         TESTS(TEST(always_pass), TEST(fail_always), TEST(skipped),
               FIXTURE(a_fixture), TEST(fixture_must_run)));
