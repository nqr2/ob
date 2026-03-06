// "Extended diff"

/*
 * Exits with 0 if the inputs are "the same", and 1 if they are not.
 * Comparisons are made line-by-line, where:
 * - If a line ends in one of:
 *   - '(any)' : This line is not checked.
 *   - '(pat)' : This is checked against a "pattern" (TODO)
 * - Otherwise, the check is done as usual.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ob/base/Argparse.h>

// Double this if it aborts with "BUFFER TOO SHORT" (on valid input of course)
constexpr size_t LINE_MAX = 256;

#define failwith(Message)                                                      \
  perror2((Message));                                                          \
  goto failure

#define panic(Message)                                                         \
  perror2((Message));                                                          \
  abort()

void perror2(char const *message) {
  if (errno != 0) {
    perror(message);
  } else {
    fputs(message, stderr);
    fputc('\n', stderr);
  }
}

// Yes clang-tidy, I know this has a lot of conditions, those checks are the
// whole point of this function. Please stop warning me about it.

// NOLINTBEGIN(readability-function-cognitive-complexity)
bool runtest(char const *expect_path, char const *input_path) {
  char expect_line[LINE_MAX] = {};
  char input_line[LINE_MAX] = {};

  FILE *expect = nullptr;
  FILE *input = nullptr;

  if (expect_path == nullptr) {
    failwith("expect file not provided");
  }

  if (input_path == nullptr) {
    failwith("input file not provided");
  }

  expect = fopen(expect_path, "r");
  input = fopen(input_path, "r");

  if (expect == nullptr) {
    failwith("could not open expect file");
  }

  if (input == nullptr) {
    failwith("could not open input file");
  }

  for (int line = 1;; line++) {
    memset(expect_line, 0, LINE_MAX);
    memset(input_line, 0, LINE_MAX);

    auto egot = fgets(expect_line, LINE_MAX, expect);
    auto igot = fgets(input_line, LINE_MAX, input);

    /* fgets can return NULL if it reads at an EOF, so we check that before
     * checking for errors
     */

    if (feof(expect) != feof(input)) {
      if (feof(expect)) {
        failwith("excess lines in input");
      }

      if (feof(input)) {
        failwith("early EOF in input");
      }
    }

    if (feof(expect)) {
      break;
    }

    if (egot == nullptr) {
      failwith("failed to read an expect line");
    }

    if (igot == nullptr) {
      failwith("failed to read an input line");
    }

    /* if the line buffer is too short, there will be a non-NUL character at the
     * end, since memset ensures this is \0 otherwise.
     */

    if (expect_line[LINE_MAX - 1] != 0) {
      panic("BUFFER TOO SHORT on expect");
    }

    /* this is only a failure since bugs can cause overlong lines */
    if (input_line[LINE_MAX - 1] != 0) {
      failwith("BUFFER TOO SHORT on input");
    }

    auto expect_len = strlen(expect_line) - 1;
    auto input_len = strlen(input_line) - 1;

    expect_line[expect_len] = 0;
    input_line[input_len] = 0;

    // The checks in question:

    /* check for (any) */
    if (strncmp(expect_line + (expect_len - 5), "(any)", 4) == 0) {
      continue;
    }

    /* regular case */
    if (strcmp(expect_line, input_line) != 0) {
      fprintf(stderr,
              "mismatch at line %d\n"
              "  expected:    %s\n"
              "       got:    %s\n",
              line, expect_line, input_line);
      goto failure;
    }
  }

  fclose(expect);
  fclose(input);

  return true;

failure:
  if (expect != nullptr) {
    fclose(expect);
  }

  if (input != nullptr) {
    fclose(input);
  }

  return false;
}
// NOLINTEND(readability-function-cognitive-complexity)

int main(int argn, char const *argv[]) {
  char const *expect = nullptr;
  char const *test = nullptr;

  auto f_expect = ql_create_flag('e', nullptr, QL_FLAG_STRING, (void *)&expect);
  auto f_test = ql_create_flag('i', nullptr, QL_FLAG_STRING, (void *)&test);

  auto parser = ql_create_parser((ql_Flag[]){f_expect, f_test, QL_FLAGS_END});

  ql_parse(&parser, argn, argv);

  auto passed = runtest(expect, test);

  return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
