add_library("ObCore"
  "include/ob/Bytecode.h"
  "include/ob/Context.h"
  "include/ob/ContextFwd.h"
  "include/ob/Exncodes.h"
  "include/ob/Object.h"
  "include/ob/Parse.h"
  "include/ob/Serial.h"
  "include/ob/String.h"
  "include/ob/Ob.h"

  "src/Bytecode.c"
  "src/Context.c"
  "src/Object.c"
  "src/Parse.c"
  "src/Serial.c"
  "src/String.c"
)

target_include_directories("ObCore" PUBLIC "include")

target_link_libraries("ObCore" PUBLIC "Ql")

if(BUILD_TESTS)
  add_subdirectory("tests")
endif()
