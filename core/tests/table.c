#include "Tap.h"

#include <ob/Allocator.h>
#include <ob/Assert.h>
#include <ob/Table.h>

void assert_failure() {
  fail_with("assertion failed");
}

ql_Allocator libc;

void can_create() {
  auto tbl = (ob_Table){};
  obtbl_init(&tbl, &libc);

  obtbl_free(&tbl);
}

void can_add_an_element() {
  auto tbl = (ob_Table){};
  obtbl_init(&tbl, &libc);

  uint64_t hash = 0;
  void *pointer = (void *)0xbabacafedeadbeef;

  auto is_new = obtbl_set(&tbl, hash, pointer);

  ASSERT(is_new, "fresh entry in empty table was not new");

  obtbl_free(&tbl);
}

void can_get_an_element() {
  auto tbl = (ob_Table){};
  obtbl_init(&tbl, &libc);

  uint64_t hash = 0;
  void *original = (void *)0xbabacafedeadbeef;

  auto is_new = obtbl_set(&tbl, hash, original);
  ASSERT(is_new, "fresh entry in empty table was not new");

  void *from_table = NULL;
  auto entry_found = obtbl_get(&tbl, hash, &from_table);
  ASSERT(entry_found, "could not find entry known to exist");

  ASSERT(from_table == original, "got different values from table: %p vs. %p",
         from_table, original);

  obtbl_free(&tbl);
}

void can_iterate() {
  auto tbl = (ob_Table){};
  obtbl_init(&tbl, &libc);

  auto first = -1;
  auto second = -1;
  auto third = -1;
  auto fourth = -1;

  auto is_new = obtbl_set(&tbl, 0, &first);
  ASSERT(is_new, "fresh entry in empty table was not new: 0");

  is_new = obtbl_set(&tbl, 1, &second);
  ASSERT(is_new, "fresh entry in empty table was not new: 1");

  is_new = obtbl_set(&tbl, 2, &third);
  ASSERT(is_new, "fresh entry in empty table was not new: 2");

  is_new = obtbl_set(&tbl, 3, &fourth);
  ASSERT(is_new, "fresh entry in empty table was not new: 3");

  uint64_t index = 0;
  uint64_t key = 0;
  void *value = NULL;

  while (obtbl_iterate(&tbl, &index, &key, &value)) {
    *(int *)value = (int)key;
  }

  ASSERT(first == 0, "did not iterate though index: 0");
  ASSERT(second == 1, "did not iterate though index: 1");
  ASSERT(third == 2, "did not iterate though index: 2");
  ASSERT(fourth == 3, "did not iterate though index: 3");

  obtbl_free(&tbl);
}

const Test SUITE[] = {
    PASS(can_create),
    PASS(can_add_an_element),
    PASS(can_get_an_element),
    PASS(can_iterate),
    SUITE_END,
};

int main() {
  libc = ql_alloc_create();

  obassert_add_handler(assert_failure);
  test(SUITE);

  return 0;
}
