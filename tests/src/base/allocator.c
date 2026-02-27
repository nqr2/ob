#include <Test.h>

#include <ob/base/Allocator.h>
#include <ob/base/Assert.h>

static ql_Allocator libc;

DEFTEST(alloc_0_returns_null) {
  auto null = ql_allocate(&libc, 0);
  OB_ASSERT_NULL(null);
}

DEFFIXTURE(setup) {
  libc = ql_alloc_create();
  return true;
}

DEFSUITE(allocator, SUITES(),
         TESTS(FIXTURE(setup), TEST(alloc_0_returns_null)));
