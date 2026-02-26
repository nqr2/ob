#include <Test.h>

DEFTEST(always_passes) {
}

DEFTEST(fail_always) {
  fail();
}

DEFSUITE(SUITE){TEST(always_passes), TEST(fail_always), SUITE_END};
