#include <ql/Allocator.h>
#include <ql/Assert.h>
#include <ql/Tap.h>

void assert_failure() {
  ql_fail_with("assertion failed");
}

ql_Allocator libc;

void alloc_0_returns_null() {
  auto null = ql_allocate(&libc, 0);
  QL_ASSERT_NULL(null);
}

const ql_Test SUITE[] = {
    {"allocate(0) returns NULL", alloc_0_returns_null, false},

    QL_SUITE_END,
};

int main() {
  libc = ql_alloc_create();

  ql_assert_add_handler(assert_failure);

  ql_test(SUITE);

  return 0;
}
