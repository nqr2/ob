#include <Test.h>

#include <ob/base/Argparse.h>
#include <ob/base/Log.h>

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

enum {
  PASS = -1,
  FAIL = -2,
};

jmp_buf buf;
char const *reason;

struct {
  bool list_tests;
  char const *run_test;
  int log_level;
} options = {};

int run_entry(Entry const *entry) {
  if (entry->is_end) {
    return PASS;
  }

  QL_INFO("running entry named '%s'", entry->name);

  auto status = PASS;

  switch (setjmp(buf)) {
  case FAIL:
    status = FAIL;
    break;
  case PASS:
    return PASS;
  case 0: {
    if (entry->is_test) { // test case
      entry->test();
    } else { // suite
      // uhhh run every test in the suite?
    }
  } break;
  default:
    return FAIL;
  }

  if (entry->name != NULL) {
    if (strncmp(entry->name, "fail_", 5) == 0) {
      if (status == FAIL) {
        status = PASS;
      } else if (status == PASS) {
        status = FAIL;
      }
    }
  }

  return status;
}

int main(int n_args, char const *argv[]) {
  auto f_list = ql_create_flag('l', "list", QL_FLAG_SET, &options.list_tests);
  auto f_run =
      ql_create_flag('r', "run", QL_FLAG_STRING, (void *)&options.run_test);
  auto f_verbosity =
      ql_create_flag('v', nullptr, QL_FLAG_INT, &options.log_level);

  auto parser = ql_create_parser((ql_Flag[]){f_list, f_run, f_verbosity});

  ql_parse(&parser, n_args, argv);

  auto log = ql_log_create_handler();
  ql_log_set_handler(&log);
  ql_log_set_level(options.log_level);

  // bool passed = true;

  auto entries = SUITE.entries;

  if (options.run_test != nullptr) {
    auto found = false;

    // uhhhmmm
    for (int i = 0; !entries[i].is_end; i++) {
      auto entry = entries + i;

      QL_WARN("at index: %d", i);

      if (entries[i].is_fixture) {
        QL_INFO("running fixture: '%s", entry->name);
        entry->fixture();
        continue;
      }

      QL_WARN("name: %s", entry->name);

      if (strcmp(entry->name, options.run_test) == 0) {
        auto status = run_entry(entry);

        if (status == FAIL) {
          exit(EXIT_FAILURE);
        }

        found = true;
      }
    }

    if (!found) {
      QL_ERROR("entry not found: '%s'", options.run_test);
      exit(EXIT_FAILURE);
    }
  }

  if (options.list_tests) {
    for (int i = 0; !entries[i].is_end; i++) {
      if (entries[i].is_test) {
        puts(entries[i].name);
      }
    }
  }

  return 0;
}

static void throw(int val) {
  longjmp(buf, val);
}

void skip() {
  throw(PASS);
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
