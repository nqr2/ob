add_library("Ql"
  "include/ql/Allocator.h"
  "include/ql/Argparse.h"
  "include/ql/Array.h"
  "include/ql/Assert.h"
  "include/ql/Exn.h"
  "include/ql/Hash.h"
  "include/ql/Log.h"
  "include/ql/Number.h"
  "include/ql/Table.h"
  "include/ql/Tap.h"

  "src/Allocator.c"
  "src/Argparse.c"
  "src/Array.c"
  "src/Assert.c"
  "src/Exn.c"
  "src/Hash.c"
  "src/Log.c"
  "src/Number.c"
  "src/Table.c"
  "src/Tap.c"
)

target_include_directories("Ql" PUBLIC "include")

if(BUILD_TESTS)
  add_subdirectory("tests")
endif()
