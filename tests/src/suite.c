#include <Test.h>

extern Suite const SUITE_drv, SUITE_core, SUITE_base;

DEFSUITE(, SUITES(&SUITE_drv, &SUITE_core, &SUITE_base), TESTS());
