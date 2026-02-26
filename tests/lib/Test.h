#ifndef TEST_H_INCLUDED
#define TEST_H_INCLUDED

#include <stddef.h>

typedef void (*Body)();

typedef struct Entry {
  struct {
    bool is_test : 1;
    bool is_end : 1;
  };

  char const *name;

  union {
    struct Entry const *suite;
    Body body;
  };
} Entry;

#define DEFTEST(N) void N()
#define DEFSUITE(N) Entry const N[] =

#define TEST(F) {.is_test = true, .name = #F, .body = (F)}
#define FIXTURE(F) {.is_test = true, .name = nullptr, .body = (F)}
#define SUBSUITE(S) {.is_test = false, .suite = (S)}
#define SUITE_END {.is_end = true}

extern Entry const SUITE[];

void skip();
void skip_with(char const *reason);

void fail();
void fail_with(char const *reason);

void bailout();

int main(int n_args, char const *argv[]);

#endif
