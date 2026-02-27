#ifndef TEST_H_INCLUDED
#define TEST_H_INCLUDED

#include <stddef.h>

#include <ob/base/Allocator.h>
#include <ob/Core.h>

typedef void (*TestBody)();
typedef bool (*FixtureBody)();

struct Suite;
struct Entry;

typedef struct Suite {
  char const *name;

  struct {
    bool request_allocator : 1;
    bool request_context : 1;
  };

  struct Suite const **suites;
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

#define DEFTEST(N) static void N()
#define DEFFIXTURE(N) static bool N()

#define TEST(F) {.is_test = true, .name = #F, .test = (F)}
#define FIXTURE(F) {.is_fixture = true, .name = #F, .fixture = (F)}
#define SUITE_END {.is_end = true}

#define SUITES(...)                                                            \
  (Suite const *[]) {                                                          \
    __VA_ARGS__ __VA_OPT__(, ) nullptr                                         \
  }

#define TESTS(...)                                                             \
  (Entry[]) {                                                                  \
    __VA_ARGS__ __VA_OPT__(, ) SUITE_END                                       \
  }

#define DEFSUITE(Name, Suites, Tests, ...)                                     \
  const Suite SUITE_##Name = {.name = #Name,                                   \
                              .suites = (Suites),                              \
                              .entries = (Tests)__VA_OPT__(, ) __VA_ARGS__}

extern const Suite SUITE_;

extern Suite const *top_suites[];
extern size_t top_suites_length;

void skip();
void skip_with(char const *reason);

void fail();
void fail_with(char const *reason);

ql_Allocator* allocator();
ob_Ctx context();

#endif
