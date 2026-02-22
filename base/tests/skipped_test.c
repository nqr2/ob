#include <ob/base/Tap.h>

void one() {
  ql_skip_with("skipped!");
}

ql_Test const SUITE[] = {
    {"one", one, false},
    QL_SUITE_END,
};

int main() {
  ql_test(SUITE);

  return 0;
}
