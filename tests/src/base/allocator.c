#include <Test.h>

#include <ob/base/Allocator.h>
#include <ob/base/Assert.h>

DEFTEST(alloc_0_returns_null) {
  auto null = ql_allocate(allocator(), 0);
  OB_ASSERT_NULL(null);
}

DEFSUITE(allocator, SUITES(), TESTS(TEST(alloc_0_returns_null)),
         .request_allocator = true);
