#ifndef TEST_H_INCLUDED
#define TEST_H_INCLUDED

typedef void (*TestBody)();
typedef bool (*FixtureBody)();

struct Suite;
struct Entry;

typedef struct Suite {
  char const *name;

  struct Suite const *suites;
  struct Entry const *entries;
} Suite;

typedef struct Entry {
  struct {
    bool is_test : 1;
    bool is_end : 1;
    bool is_fixture : 1;
  };

  char const *name;

  union {
    TestBody test;
    FixtureBody fixture;
  };
} Entry;

#define DEFTEST(N) void N()
#define DEFFIXTURE(N) bool N()

#define TEST(F) {.is_test = true, .name = #F, .test = (F)}
#define FIXTURE(F) {.is_fixture = true, .name = #F, .fixture = (F)}
#define SUITE_END {.is_end = true}

#define DEFSUITE(Name, Suites, ...)                                            \
  Suite const Name = {.name = #Name,                                           \
                      .suites = (Suite[])Suites,                               \
                      .entries = (Entry[]){__VA_ARGS__}}

extern Suite const SUITE;

void skip();
void skip_with(char const *reason);

void fail();
void fail_with(char const *reason);

#endif
