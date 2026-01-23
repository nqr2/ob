#include <ql/Assert.h>
#include <ql/Hash.h>
#include <ql/Tap.h>

#include <inttypes.h>

#define ASSERT_HASH_EQ(L, R)                                                   \
  QL_ASSERT(L == R,                                                            \
            "%" PRIx64 " =/= "                                                 \
            "%" PRIx64,                                                        \
            L, R)

void assert_failure() {
  ql_fail_with("assertion failed");
}

void known_results() {
  // From the IETF:
  // https://www.ietf.org/archive/id/draft-eastlake-fnv-21.html#name-a-few-test-vectors
  ASSERT_HASH_EQ(ql_hash_literal(""), 0xcbf29ce484222325);
  ASSERT_HASH_EQ(ql_hash_literal("a"), 0xaf63dc4c8601ec8c);
  ASSERT_HASH_EQ(ql_hash_literal("foobar"), 0x85944171f73967e8);

  // From the author:
  // http://www.isthe.com/chongo/tech/comp/fnv/index.html
  const uint8_t bytes[] = {0xd5, 0x6b, 0xb9, 0x53, 0x42, 0x87, 0x08, 0x36};
  ASSERT_HASH_EQ(ql_hash_start(8, bytes), 0);
}

void hash_continue_works() {
  auto foo = ql_hash_literal("foo");
  auto foobar = ql_hash_continue(foo, 3, "bar");

  ASSERT_HASH_EQ(foobar, ql_hash_literal("foobar"));
}

const ql_Test SUITE[] = {
    {"known results", known_results, false},

    {"hash_continue works", hash_continue_works, false},

    // The vulnerability mentioned in Wikipedia, and a set of collissions
    // in StackOverflow seem to only hold with the 32-bit hashes.

    QL_SUITE_END,
};

int main() {
  ql_assert_add_handler(assert_failure);
  ql_test(SUITE);

  return 0;
}
