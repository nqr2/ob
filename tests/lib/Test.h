#ifndef TEST_H_INCLUDED
#define TEST_H_INCLUDED

typedef void (*Test)();
typedef bool (*Fixture)();

typedef struct Entry {
  // It's better to use an enum, but we have too many bits to spare so meh.
  struct {
    bool is_test : 1;
    bool is_end : 1;
    bool is_fixture : 1;
    bool is_subsuite : 1;
  };

  char const *name;

  union {
    struct Entry const *suite;
    Test test;
    Fixture fixture;
  };
} Entry;

#define DEFTEST(N) void N()
#define DEFFIXTURE(N) bool N()
#define DEFSUITE(N) Entry const N[] =

#define TEST(F) {.is_test = true, .name = #F, .test = (F)}
#define FIXTURE(F) {.is_fixture = true, .name = #F, .fixture = (F)}
#define SUBSUITE(S) {.is_subsuite = true, .suite = (S)}
#define SUITE_END {.is_end = true}

extern Entry const SUITE[];

void skip();
void skip_with(char const *reason);

void fail();
void fail_with(char const *reason);

void bailout();

int main(int n_args, char const *argv[]);

#endif
