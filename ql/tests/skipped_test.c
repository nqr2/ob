#include <ql/Tap.h>

void one() {
  ql_skip_with("skipped!");
}

const ql_Test SUITE[] = {
    {"one", one, false},
    QL_SUITE_END,
};

int main() {
  ql_test(SUITE);

  return 0;
}
