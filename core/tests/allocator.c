#include "Tap.h"

#include <ob/Allocator.h>
#include <ob/Assert.h>

void assert_failure() {
  fail_with("assertion failed");
}

ql_Allocator libc;

void alloc_0_returns_null() {
  auto null = ql_allocate(&libc, 0);
  QL_ASSERT_NULL(null);
}

const Test SUITE[] = {
    {"allocate(0) returns NULL", alloc_0_returns_null, false},

    SUITE_END,
};

int main() {
  libc = ql_alloc_create();

  ql_assert_add_handler(assert_failure);

  test(SUITE);

  return 0;
}
