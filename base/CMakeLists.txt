add_library("base" OBJECT)

target_sources("base" PUBLIC
  FILE_SET HEADERS
  BASE_DIRS "include"
  FILES
    "include/ob/base/Allocator.h"
    "include/ob/base/Argparse.h"
    "include/ob/base/Array.h"
    "include/ob/base/Assert.h"
    "include/ob/base/Exn.h"
    "include/ob/base/Hash.h"
    "include/ob/base/Log.h"
    "include/ob/base/Number.h"
    "include/ob/base/Table.h"
    "include/ob/base/Tap.h"
)

target_sources("base" PRIVATE
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

target_include_directories("base" PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_library(ob::base ALIAS "base")

if(OB_BUILD_TESTS)
  add_subdirectory("tests")
endif()

if(OB_INSTALL)
  install(
    TARGETS "base"
    EXPORT ob
    FILE_SET HEADERS
      DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  )
endif()
