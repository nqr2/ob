#include "Assert.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

FnAssertFailure handler = NULL;

void assert__report(const char *file, int line, const char *function,
                    const char *condition) {
  fprintf(stderr,
          "```\n"
          "Assertion failed\n"
          "File: %s line: %d (%s)\n"
          "Checked: `%s`\n"
          "Message: ",
          file, line, function, condition);
}

void assert__message(const char *message, ...) {
  va_list args;
  va_start(args, message);

  vfprintf(stderr, message, args);

  fprintf(stderr, "```\n");

  va_end(args);
}

void assert__fail() {
  if (handler != NULL) {
    handler();
  }

  exit(EXIT_FAILURE);
}

void assert_add_handler(FnAssertFailure fail) {
  handler = fail;
}
