#include <Test.h>

#include <ob/base/Argparse.h>
#include <ob/base/Assert.h>
#include <ob/base/Log.h>

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  PASS = -1,
  FAIL = -2,
};

jmp_buf jbuf;
char const *throw_message;

struct {
  bool list_tests;
  char const *run_test;
  int log_level;
} options = {};

char const *prefix_stack[16] = {};
int prefix_top = 0;

void print_prefix() {
  for (int i = 0; i < prefix_top; i++) {
    printf("%s/", prefix_stack[i]);
  }
}

void list_tests(Suite const *suite) {
  auto entries = suite->entries;

  prefix_stack[prefix_top++] = suite->name;

  for (int i = 0; suite->suites[i] != nullptr; i++) {
    list_tests(suite->suites[i]);
  }

  for (int i = 0; !entries[i].is_end; i++) {
    if (entries[i].is_test) {
      print_prefix();
      puts(entries[i].name);
    }
  }

  prefix_top--;
}

static void assert_failure() {
  fail_with("assertion failed");
}

bool run_entry(Entry const *entry) {
  ql_assert_add_handler(assert_failure);

  switch (setjmp(jbuf)) {
  case FAIL:
    return false;
  case PASS:
    return true;

  case 0: {
    entry->test();
  }; break;

  default:
    break;
  }

  return true;
}

typedef enum {
  OK,
  TEST_FAILED,
  FIXTURE_FAILED,
  TEST_NOT_FOUND,
  SUITE_NOT_FOUND,
} Status;

Status run_named(char const *test, Suite const *suite) {
  auto len = strlen(test);
  char const *prefix = memchr(test, '/', len);

  if (prefix == nullptr) {
    for (int i = 0; !suite->entries[i].is_end; i++) {
      auto entry = &suite->entries[i];

      if (entry->is_fixture) {
        auto pass = entry->fixture();

        if (!pass) {
          return FIXTURE_FAILED;
        }
      }

      puts(entry->name);
      if (strcmp(test, entry->name) == 0) {
        auto pass = run_entry(entry);

        if (!pass) {
          return TEST_FAILED;
        }

        return OK;
      }
    }

    return TEST_NOT_FOUND;
  }

  prefix++;
  len = prefix - test - 1;

  Suite const *inner = nullptr;

  for (int i = 0; suite->suites[i] != nullptr; i++) {
    if (strncmp(test, suite->suites[i]->name, len) == 0) {
      inner = suite->suites[i];
      break;
    }
  }

  if (suite != nullptr) {
    return run_named(prefix, inner);
  }

  return SUITE_NOT_FOUND;
}

int main(int n_args, char const *argv[]) {
  auto f_list = ql_create_flag('l', "list", QL_FLAG_SET, &options.list_tests);
  auto f_run =
      ql_create_flag('r', "run", QL_FLAG_STRING, (void *)&options.run_test);
  auto f_verbosity =
      ql_create_flag('v', nullptr, QL_FLAG_INT, &options.log_level);

  auto parser =
      ql_create_parser((ql_Flag[]){f_list, f_run, f_verbosity, QL_FLAGS_END});

  ql_parse(&parser, n_args, argv);

  auto log = ql_log_create_handler();
  ql_log_set_handler(&log);
  ql_log_set_level(options.log_level);

  auto exit = EXIT_SUCCESS;

  if (options.run_test != nullptr) {
    auto status = run_named(options.run_test, &SUITE_);

    switch (status) {
    case OK:
      break;
    case TEST_FAILED:
      puts("test failed");
      exit = EXIT_FAILURE;
      break;
    case FIXTURE_FAILED:
      puts("fixture failed");
      exit = EXIT_FAILURE;
      break;
    case TEST_NOT_FOUND:
      puts("test not found");
      exit = EXIT_FAILURE;
      break;
    case SUITE_NOT_FOUND:
      puts("suite not found");
      exit = EXIT_FAILURE;
      break;
    }
  }

  if (options.list_tests) {
    for (int i = 0; SUITE_.suites[i] != nullptr; i++) {
      list_tests(SUITE_.suites[i]);
    }
  }

  return exit;
}

static void throw(int val) {
  longjmp(jbuf, val);
}

void skip() {
  throw(PASS);
}

void skip_with(char const *rsn) {
  throw_message = rsn;
  skip();
}

void fail() {
  throw(FAIL);
}

void fail_with(char const *rsn) {
  throw_message = rsn;
  fail();
}
