cmake_minimum_required(VERSION 4.1.2)

add_library("ql" OBJECT
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

install(
  DIRECTORY "include/ql"
  TYPE INCLUDE
)

install(
  TARGETS "ql"
  ARCHIVE
)

target_include_directories("ql" PUBLIC "include")

add_library(ql::ql ALIAS "ql")

if(OB_BUILD_TESTS)
  add_subdirectory("tests")
endif()
