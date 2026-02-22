#include <ob/base/Tap.h>

void one() {
  ql_fail_with("failed!");
}

ql_Test const SUITE[] = {
    {"one", one, true},
    QL_SUITE_END,
};

int main() {
  ql_test(SUITE);

  return 0;
}
