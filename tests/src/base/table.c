#include <Test.h>

#include <ob/base/Allocator.h>
#include <ob/base/Assert.h>
#include <ob/base/Table.h>

static ql_Allocator libc;

DEFTEST(can_create) {
  auto tbl = (ql_Table){};
  ql_table_init(&tbl, &libc);

  ql_table_free(&tbl);
}

DEFTEST(can_add_an_element) {
  auto tbl = (ql_Table){};
  ql_table_init(&tbl, &libc);

  uint64_t hash = 0;
  void *pointer = (void *)0xbabacafedeadbeef;

  auto is_new = ql_table_set(&tbl, hash, pointer);

  QL_ASSERT(is_new, "fresh entry in empty table was not new");

  ql_table_free(&tbl);
}

DEFTEST(can_get_an_element) {
  auto tbl = (ql_Table){};
  ql_table_init(&tbl, &libc);

  uint64_t hash = 0;
  void *original = (void *)0xbabacafedeadbeef;

  auto is_new = ql_table_set(&tbl, hash, original);
  QL_ASSERT(is_new, "fresh entry in empty table was not new");

  void *from_table = NULL;
  auto entry_found = ql_table_get(&tbl, hash, &from_table);
  QL_ASSERT(entry_found, "could not find entry known to exist");

  QL_ASSERT(from_table == original,
            "got different values from table: %p vs. %p", from_table, original);

  ql_table_free(&tbl);
}

DEFTEST(can_iterate) {
  auto tbl = (ql_Table){};
  ql_table_init(&tbl, &libc);

  auto first = -1;
  auto second = -1;
  auto third = -1;
  auto fourth = -1;

  auto is_new = ql_table_set(&tbl, 0, &first);
  QL_ASSERT(is_new, "fresh entry in empty table was not new: 0");

  is_new = ql_table_set(&tbl, 1, &second);
  QL_ASSERT(is_new, "fresh entry in empty table was not new: 1");

  is_new = ql_table_set(&tbl, 2, &third);
  QL_ASSERT(is_new, "fresh entry in empty table was not new: 2");

  is_new = ql_table_set(&tbl, 3, &fourth);
  QL_ASSERT(is_new, "fresh entry in empty table was not new: 3");

  uint64_t index = 0;
  uint64_t key = 0;
  void *value = NULL;

  while (ql_table_iterate(&tbl, &index, &key, &value)) {
    *(int *)value = (int)key;
  }

  QL_ASSERT(first == 0, "did not iterate though index: 0");
  QL_ASSERT(second == 1, "did not iterate though index: 1");
  QL_ASSERT(third == 2, "did not iterate though index: 2");
  QL_ASSERT(fourth == 3, "did not iterate though index: 3");

  ql_table_free(&tbl);
}

DEFFIXTURE(init_alloc) {
  libc = ql_alloc_create();
  return true;
}

DEFSUITE(table, SUITES(),
         TESTS(FIXTURE(init_alloc), TEST(can_create), TEST(can_add_an_element),
               TEST(can_get_an_element), TEST(can_iterate)));
