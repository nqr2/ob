#include <Test.h>

extern Suite const SUITE_get_slot;

DEFSUITE(core, SUITES(&SUITE_get_slot), TESTS());
