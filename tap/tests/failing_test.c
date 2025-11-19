#include "Tap.h"

void one() {
  fail_with("failed!");
}

const Test SUITE[] = {
    {"one", one, true},
    SUITE_END,
};

int main() {
  test(SUITE);

  return 0;
}
