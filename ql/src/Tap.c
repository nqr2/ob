#include <ql/Tap.h>

#include <setjmp.h>
#include <stdio.h>
#include <threads.h>

enum {
  FAIL = -1,
  SKIP = -2,
  BAILOUT = -3,
};

jmp_buf thisbuf;
const char *thisreason;

void ql_skip() {
  longjmp(thisbuf, SKIP);
}

void ql_skip_with(const char *reason) {
  thisreason = reason;
  ql_skip();
}

void ql_fail() {
  longjmp(thisbuf, FAIL);
}

void ql_fail_with(const char *reason) {
  thisreason = reason;
  ql_fail();
}

void ql_bailout() {
  longjmp(thisbuf, BAILOUT);
}

void runtest(const ql_Test *test, int index) {
  thisreason = NULL;

  auto pass = true;
  auto skipped = false;

  switch (setjmp(thisbuf)) {
  case FAIL:
    pass = false;
    break;
  case SKIP:
    skipped = true;
    break;
  case BAILOUT:
    puts("Bail out!");
    return;
  default: {
    test->body();
  } break;
  }

  if (!pass) {
    printf("not ");
  }

  printf("ok %d - %s", index + 1, test->name);

  if (test->should_fail || skipped) {
    printf(" #");

    if (test->should_fail) {
      printf(" TODO");
    } else if (skipped) {
      printf(" SKIP");
    }

    if (thisreason != NULL) {
      printf(" %s", thisreason);
    }
  }

  putchar('\n');
}

void ql_test(const ql_Test *suite) {
  int count = 0;

  for (;; count++) {
    if (suite[count].body == NULL) {
      break;
    }
  }

  printf("TAP version 13\n1..%d\n", count);

  for (int i = 0;; i++) {
    if (suite[i].body == NULL) {
      break;
    }

    runtest(&suite[i], i);
  }
}
