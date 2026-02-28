#ifndef TEST_H_INCLUDED
#define TEST_H_INCLUDED

#include <stddef.h>

#include <ob/Core.h>
#include <ob/base/Allocator.h>

struct Suite;
struct Entry;

/** The "body" of a unit test or a fixture.
 *
 * A test/fixture is said to "succeed" if it returns successfully, and to "fail"
 * if at any point @ref fail or @ref fail_with is called. @ref skip and
 * @ref skip_with will instead "force" it to pass.
 */
typedef void (*Body)();

/// A test suite.
typedef struct Suite {
  char const *name;

  struct {
    /// Set to @c true if this suite requires an allocator.
    bool request_allocator : 1;

    /// Set to @c true if this suite requires a @ref ob_Ctx.
    bool request_context : 1;
  };

  struct Suite const **suites;

  /// An ordered array of tests and fixtures.
  struct Entry const *entries;
} Suite;

typedef struct Entry {
  struct {
    /// @c true if this is a test, and @c false if this is a fixture.
    bool is_test : 1;
  };

  /// The name of this entry, or @c NULL for an "end-of-suite" sentinel.
  char const *name;

  /// The body of this entry.
  Body body;
} Entry;

#define DEFTEST(N) static void N()
#define DEFFIXTURE(N) static void N()

#define TEST(F) {.is_test = true, .name = #F, .body = (F)}
#define FIXTURE(F) {.is_test = false, .name = #F, .body = (F)}

#define SUITES(...)                                                            \
  (Suite const *[]) {                                                          \
    __VA_ARGS__ __VA_OPT__(, ) nullptr                                         \
  }

#define TESTS(...)                                                             \
  (Entry[]) {                                                                  \
    __VA_ARGS__ __VA_OPT__(, ) {                                               \
      .name = nullptr                                                          \
    }                                                                          \
  }

#define DEFSUITE(Name, Suites, Tests, ...)                                     \
  const Suite SUITE_##Name = {.name = #Name,                                   \
                              .suites = (Suites),                              \
                              .entries = (Tests)__VA_OPT__(, ) __VA_ARGS__}

/// Skip any code afterwards, and pass this test.
void skip();

/// Skip any code afterwards, and fail this test.
void fail();

void skip_with(char const *reason);
void fail_with(char const *reason);

/// @returns An usable allocator if @ref Suite::request_allocator is @c true,
/// and @c NULL otherwise.
ql_Allocator *allocator();

/// @returns An usable @ref ob_Ctx if @ref Suite::request_context is @c true,
/// and @c NULL otherwise.
ob_Ctx context();

/** @details
 * The executable must contain a "top" test suite, of the form
 *
 *   DEFSUITE(, SUITES(...), TESTS(...))
 *
 * which is then read by the command line. The macro expansion from that results
 * in the definition of this variable, and any test suite must be a subsuite of
 * this one for it's tests to be known by CMake.
 */
extern Suite const SUITE_;

#endif
