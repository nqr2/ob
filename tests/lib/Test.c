#include <Test.h>

#include <ob/base/Argparse.h>
#include <ob/base/Assert.h>
#include <ob/base/Log.h>

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ql_Allocator *alloc = nullptr;
static ob_Ctx ctx = nullptr;

enum {
  PASS = -1,
  FAIL = -2,
};

static jmp_buf jbuf;
static char const *throw_message;

static struct {
  bool list_tests;
  char const *run_test;
  int log_level;
} options = {};

static char const *prefix_stack[16] = {};
static int prefix_top = 0;

static void print_prefix() {
  for (int i = 0; i < prefix_top; i++) {
    printf("%s/", prefix_stack[i]);
  }
}

static void list_tests(Suite const *suite) {
  auto entries = suite->entries;

  prefix_stack[prefix_top++] = suite->name;

  for (int i = 0; suite->suites[i] != nullptr; i++) {
    list_tests(suite->suites[i]);
  }

  for (int i = 0; entries[i].name != nullptr; i++) {
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

static bool run_entry(Entry const *entry) {
  ql_assert_add_handler(assert_failure);

  switch (setjmp(jbuf)) {
  case FAIL:
    return false;
  case PASS:
    return true;

  case 0: {
    entry->body();
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

static void setup_allocator() {
  auto allocator = ql_alloc_create();

  alloc = ql_allocate(&allocator, sizeof(ql_Allocator));
  *alloc = allocator;
}

static void setup_context() {
  ctx = ob_create(allocator());
}

static Status run_named(char const *test, Suite const *suite) {
  auto len = strlen(test);
  char const *prefix = memchr(test, '/', len);

  if (prefix == nullptr) {
    if (suite->request_allocator) {
      setup_allocator();
    }

    if (suite->request_context) {
      setup_allocator();
      setup_context();
    }

    for (int i = 0; suite->entries[i].name != nullptr; i++) {
      auto entry = &suite->entries[i];

      if (!entry->is_test) {
        auto pass = run_entry(entry);

        if (!pass) {
          return FIXTURE_FAILED;
        }

        continue;
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

static void throw(int val) {
  longjmp(jbuf, val);
}

void skip() {
  throw(PASS);
}

void skip_with(char const *reason) {
  throw_message = reason;
  skip();
}

void fail() {
  throw(FAIL);
}

void fail_with(char const *reason) {
  throw_message = reason;
  fail();
}

ql_Allocator *allocator() {
  return alloc;
}

ob_Ctx context() {
  return ctx;
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

  if (ctx != nullptr) {
    ob_destroy(ctx);
  }

  // TODO: check leaks and fail accordingly
  if (alloc != nullptr) {
    ql_deallocate(alloc, sizeof(ql_Allocator), alloc);
  }

  return exit;
}
