#include <ql/Tap.h>

void one() {
  ql_fail_with("failed!");
}

const ql_Test SUITE[] = {
    {"one", one, true},
    QL_SUITE_END,
};

int main() {
  ql_test(SUITE);

  return 0;
}
