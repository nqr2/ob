#include <Test.h>

#include <ob/base/Argparse.h>

#include <setjmp.h>
#include <stdio.h>
#include <threads.h>

int main(int n_args, char const *argv[]) {
  auto list = false;

  auto f_list = ql_create_flag('l', "list", QL_FLAG_SET, &list);

  auto parser = ql_create_parser((ql_Flag[]){f_list});

  ql_parse(&parser, n_args, argv);

  if (list) {
    for (int i = 0; !SUITE[i].is_end; i++) {
      puts(SUITE[i].name);
    }
  }
}

enum {
  FAIL = -1,
  SKIP = -2,
  BAILOUT = -3,
};

jmp_buf buf;
char const *reason;

static void throw(int val) {
  longjmp(buf, val);
}

void skip() {
  throw(SKIP);
}

void skip_with(char const *rsn) {
  reason = rsn;
  skip();
}

void fail() {
  throw(FAIL);
}

void fail_with(char const *rsn) {
  reason = rsn;
  fail();
}

void bailout() {
  throw(BAILOUT);
}
/*
bool runtest(Test const *test, int index) {
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
    return false;
  case 0: {
    test->body();
  } break;
  }

  if (!pass && test->should_fail) {
    pass = true;
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
  }

  if (thisreason != NULL) {
    printf(" # %s", thisreason);
  }

  putchar('\n');

  return pass;
}

bool ql_test(Test const *suite) {
  int count = 0;

  for (;; count++) {
    if (suite[count].body == NULL) {
      break;
    }
  }

  printf("TAP version 13\n1..%d\n", count);

  auto passed = true;

  for (int i = 0;; i++) {
    if (suite[i].body == NULL) {
      break;
    }

    auto this_passed = runtest(&suite[i], i);

    if (!this_passed) {
      passed = false;
    }
  }

  return passed;
}
*/
