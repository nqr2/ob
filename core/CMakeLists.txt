add_library("ObCore" STATIC
  "include/ob/core/Bytecode.h"
  "include/ob/core/Context.h"
  "include/ob/core/Object.h"
  "include/ob/core/Serial.h"
  "include/ob/core/String.h"

  "include/ob/Core.h"

  "src/Bytecode.c"
  "src/Context.c"
  "src/Object.c"
  "src/Parse.c"
  "src/Serial.c"
  "src/String.c"
)

target_include_directories("ObCore" PUBLIC "include")

target_link_libraries("ObCore" PUBLIC ql::ql)

#if(BUILD_TESTS)
#  add_subdirectory("tests")
#endif()
