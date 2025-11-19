#include "Tap.h"

void one() {
}

const Test SUITE[] = {
    {"pass", one, false},
    SUITE_END,
};

int main() {
  test(SUITE);

  return 0;
}
