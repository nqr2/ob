add_library("core" OBJECT)

target_sources("core" PUBLIC
  FILE_SET HEADERS
  BASE_DIRS "include"
  FILES
    "include/ob/bits/Begin.h"
    "include/ob/bits/End.h"

    "include/ob/core/Bytecode.h"
    "include/ob/core/Context.h"
    "include/ob/core/Memory.h"
    "include/ob/core/Object.h"
    "include/ob/core/Parse.h"
    "include/ob/core/Serial.h"
    "include/ob/core/String.h"

    "include/ob/Core.h"
)

target_sources("core" PRIVATE
  "src/Bytecode.c"
  "src/Context.c"
  "src/Memory.c"
  "src/Object.c"
  "src/Parse.c"
  "src/Serial.c"
  "src/String.c"
)

add_library(ob::core ALIAS "core")

target_include_directories("core" PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries("core" PUBLIC ob::base)

target_compile_definitions("core" PRIVATE OB_IMPLEMENTATION)

if(OB_INSTALL)
  install(
    TARGETS "core"
    EXPORT ob
    FILE_SET HEADERS DESTINATION "${CMAKE_INSTALL_DIR}"
  )
endif()
