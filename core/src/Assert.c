#include <ob/Assert.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

FnAssertFailure handler = NULL;

void obassert__report(const char *file, int line, const char *function,
                      const char *condition) {
  fprintf(stderr,
          "```\n"
          "Assertion failed\n"
          "File: %s line: %d (%s)\n"
          "Checked: `%s`\n"
          "Message: ",
          file, line, function, condition);
}

void obassert__message(const char *message, ...) {
  va_list args;
  va_start(args, message);

  vfprintf(stderr, message, args);

  fprintf(stderr, "```\n");

  va_end(args);
}

void obassert__fail() {
  if (handler != NULL) {
    handler();
  }

  exit(EXIT_FAILURE);
}

void obassert_add_handler(FnAssertFailure fail) {
  handler = fail;
}
