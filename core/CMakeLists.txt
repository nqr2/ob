add_library("ObCore" OBJECT
  "include/ob/bits/Begin.h"
  "include/ob/bits/End.h"

  "include/ob/core/Bytecode.h"
  "include/ob/core/Context.h"
  "include/ob/core/Object.h"
  "include/ob/core/Parse.h"
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

install(
  DIRECTORY "include/ob"
  TYPE INCLUDE
)

target_include_directories("ObCore" PUBLIC "include")

target_link_libraries("ObCore" PUBLIC ql::ql)

target_compile_definitions("ObCore" PRIVATE OB_IMPLEMENTATION)
