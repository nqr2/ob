#include "Tap.h"

void one() {
  skip_with("skipped!");
}

const Test SUITE[] = {
    {"one", one, false},
    SUITE_END,
};

int main() {
  test(SUITE);

  return 0;
}
