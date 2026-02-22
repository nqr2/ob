#include <ob/base/Tap.h>

void one() {
}

ql_Test const SUITE[] = {
    {"pass", one, false},
    QL_SUITE_END,
};

int main() {
  ql_test(SUITE);

  return 0;
}
