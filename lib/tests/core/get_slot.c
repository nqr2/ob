#include <ob/Core.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>

#include <ql/Assert.h>
#include <ql/Hash.h>
#include <ql/Tap.h>

ql_Allocator libc;
ob_Ctx ctx;

void assert__failure() {
  ql_fail_with("assertion failed");
}

void cannot_read_bogus() {
  ob_Obj nil = nullptr;
  ob_Obj slot = nullptr;

  auto str = obstr_create_literal(ctx, ">#<bogus>#<");

  if (ob_get_slot(ctx, &slot, nil, str)) {
    ql_fail_with("expected to not contain a bogus slot");
  }
}

void read_known() {
  ob_Obj slots = ob_create_slots(ctx, nullptr);
  ob_Obj five = ob_create_integer(ctx, 5);

  auto view = ob_cast_slots(slots);

  auto key = ql_hash_start(4, "five");
  auto sel = obstr_create_literal(ctx, "five");

  ql_table_set(&view->slots, key, five);

  ob_Obj also_five = nullptr;

  if (ob_get_slot(ctx, &also_five, slots, sel)) {
    QL_ASSERT(five == also_five, "expected %p, got %p instead", five,
              also_five);
  } else {
    ql_fail_with("expected to contain a slot known to be set");
  }

  if (!ob_get_slot(ctx, nullptr, slots, sel)) {
    ql_fail_with("expected to hold even if not reading the slot");
  }
}

void read_known_after_gc() {
  ob_Obj slots = ob_create_slots(ctx, nullptr);
  ob_Obj five = ob_create_integer(ctx, 5);

  auto view = ob_cast_slots(slots);

  auto key = ql_hash_start(4, "five");
  auto sel = obstr_create_literal(ctx, "five");

  ql_table_set(&view->slots, key, five);

  ob_Obj also_five = nullptr;

  if (ob_get_slot(ctx, &also_five, slots, sel)) {
    QL_ASSERT(five == also_five, "expected %p, got %p instead", five,
              also_five);
  } else {
    ql_fail_with("expected to contain a slot known to be set");
  }

  if (!ob_get_slot(ctx, nullptr, slots, sel)) {
    ql_fail_with("expected to hold even if not reading the slot");
  }

  // we read from this, so avoid GC'ing it
  obstr_mark(sel);
  ob_mark(slots);

  ob_gc(ctx, true);

  if (ob_get_slot(ctx, &also_five, slots, sel)) {
    QL_ASSERT(five == also_five, "expected %p, got %p instead, after GC", five,
              also_five);
  } else {
    ql_fail_with("expected to contain a slot known to be set, after GC");
  }

  if (!ob_get_slot(ctx, nullptr, slots, sel)) {
    ql_fail_with("expected to hold even if not reading the slot, after GC");
  }
}

ql_Test const suite[] = {QL_PASS(cannot_read_bogus), QL_PASS(read_known),
                         QL_PASS(read_known_after_gc), QL_SUITE_END};

int core_get_slot() {
  ql_assert_add_handler(assert__failure);

  libc = ql_alloc_create();
  ctx = ob_create(&libc);

  if (!ql_test(suite)) {
    ob_destroy(ctx);
    return 1;
  }

  ob_destroy(ctx);

  return 0;
}
