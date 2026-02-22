#include <ob/base/Allocator.h>
#include <ob/base/Assert.h>
#include <ob/base/Tap.h>

void assert_failure() {
  ql_fail_with("assertion failed");
}

ql_Allocator libc;

void alloc_0_returns_null() {
  auto null = ql_allocate(&libc, 0);
  OB_ASSERT_NULL(null);
}

ql_Test const SUITE[] = {
    {"allocate(0) returns NULL", alloc_0_returns_null, false},

    QL_SUITE_END,
};

int main() {
  libc = ql_alloc_create();

  ql_assert_add_handler(assert_failure);

  ql_test(SUITE);

  return 0;
}
