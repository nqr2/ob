#include <ql/Tap.h>

void one() {
}

const ql_Test SUITE[] = {
    {"pass", one, false},
    QL_SUITE_END,
};

int main() {
  ql_test(SUITE);

  return 0;
}
