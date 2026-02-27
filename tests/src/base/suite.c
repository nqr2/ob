#include <Test.h>

extern Suite const SUITE_allocator, SUITE_argparse, SUITE_assert, SUITE_hash,
    SUITE_table;

DEFSUITE(base,
         SUITES(&SUITE_allocator, &SUITE_argparse, &SUITE_assert, &SUITE_hash,
                &SUITE_table),
         TESTS());
