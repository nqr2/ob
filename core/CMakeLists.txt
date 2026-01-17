add_library("ObCore"
  "include/ob/Allocator.h"
  "include/ob/Argparse.h"
  "include/ob/Array.h"
  "include/ob/Assert.h"
  "include/ob/Bytecode.h"
  "include/ob/Context.h"
  "include/ob/ContextFwd.h"
  "include/ob/Exn.h"
  "include/ob/Exncodes.h"
  "include/ob/Hash.h"
  "include/ob/Log.h"
  "include/ob/Number.h"
  "include/ob/Object.h"
  "include/ob/Parse.h"
  "include/ob/Serial.h"
  "include/ob/String.h"
  "include/ob/Table.h"

  "src/Allocator.c"
  "src/Argparse.c"
  "src/Array.c"
  "src/Assert.c"
  "src/Bytecode.c"
  "src/Context.c"
  "src/Exn.c"
  "src/Hash.c"
  "src/Log.c"
  "src/Number.c"
  "src/Object.c"
  "src/Parse.c"
  "src/Serial.c"
  "src/String.c"
  "src/Table.c"
)

target_include_directories("ObCore" PUBLIC "include")

if(BUILD_TESTS)
  add_subdirectory("tests")
endif()
