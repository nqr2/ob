#include <Test.h>

#include <ob/Core.h>
#include <ob/base/Assert.h>
#include <ob/base/Hash.h>
#include <ob/core/Object.h>
#include <ob/core/String.h>

void assert__failure() {
  fail_with("assertion failed");
}

DEFTEST(cannot_read_bogus) {
  auto ctx = context();

  ob_Obj nil = nullptr;
  ob_Obj slot = nullptr;

  auto str = obstr_create_literal(ctx, ">#<bogus>#<");

  if (ob_get_slot(ctx, &slot, nil, str)) {
    fail_with("expected to not contain a bogus slot");
  }
}

DEFTEST(read_known) {
  auto ctx = context();

  ob_Obj slots = ob_create_slots(ctx, nullptr);
  ob_Obj five = ob_create_integer(ctx, 5);

  auto view = ob_cast_slots(slots);

  auto key = ql_hash_start(4, "five");
  auto sel = obstr_create_literal(ctx, "five");

  obslot_add(&view->slots, sel, five);

  ob_Obj also_five = nullptr;

  if (ob_get_slot(ctx, &also_five, slots, sel)) {
    QL_ASSERT(five == also_five, "expected %p, got %p instead", five,
              also_five);
  } else {
    fail_with("expected to contain a slot known to be set");
  }

  if (!ob_get_slot(ctx, nullptr, slots, sel)) {
    fail_with("expected to hold even if not reading the slot");
  }
}

DEFTEST(read_known_after_gc) {
  auto ctx = context();

  ob_Obj slots = ob_create_slots(ctx, nullptr);
  ob_Obj five = ob_create_integer(ctx, 5);

  auto view = ob_cast_slots(slots);

  auto key = ql_hash_start(4, "five");
  auto sel = obstr_create_literal(ctx, "five");

  obslot_add(&view->slots, sel, five);

  ob_Obj also_five = nullptr;

  if (ob_get_slot(ctx, &also_five, slots, sel)) {
    QL_ASSERT(five == also_five, "expected %p, got %p instead", five,
              also_five);
  } else {
    fail_with("expected to contain a slot known to be set");
  }

  if (!ob_get_slot(ctx, nullptr, slots, sel)) {
    fail_with("expected to hold even if not reading the slot");
  }

  // we read from this, so avoid GC'ing it
  obstr_mark(sel);
  ob_mark(slots);

  ob_gc(ctx, true);

  if (ob_get_slot(ctx, &also_five, slots, sel)) {
    QL_ASSERT(five == also_five, "expected %p, got %p instead, after GC", five,
              also_five);
  } else {
    fail_with("expected to contain a slot known to be set, after GC");
  }

  if (!ob_get_slot(ctx, nullptr, slots, sel)) {
    fail_with("expected to hold even if not reading the slot, after GC");
  }
}

DEFSUITE(get_slot, SUITES(),
         TESTS(TEST(cannot_read_bogus), TEST(read_known),
               TEST(read_known_after_gc)),
         .request_context = true);
