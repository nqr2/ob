## Building the tools to build ObStd

add_executable(
  "obc"
  "bin/obc.c"
)

target_link_libraries("obc" PRIVATE ob::base ob::core)

add_executable(
  "dis"
  "bin/dis.c"
)

target_link_libraries("dis" PRIVATE ob::base ob::core)

function(obc)
  cmake_parse_arguments(PARSE_ARGV 0 arg "" OUTPUT INPUTS)

  add_custom_target(
    "${arg_OUTPUT}"
    COMMAND "obc" ${arg_INPUTS} > "${CMAKE_CURRENT_BINARY_DIR}/${arg_OUTPUT}"
    SOURCES ${arg_INPUTS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
endfunction()

## Building std itself

obc(
  OUTPUT "World.obc"
  INPUTS
    "World.ob"
)

add_library("std" OBJECT
  "src/Std.c"
  "src/AddMethods.c"
  "src/Activation.c"
  "src/String.c"
  "src/Object.c"
  "src/Method.c"
  "src/Slots.c"
  "src/Shell.c"
  "src/World.c"
)

add_library(ob::std ALIAS "std")

add_dependencies("std" "World.obc")

target_sources("std" PUBLIC
  FILE_SET HEADERS
  BASE_DIRS "include"
  FILES
    "include/ob/Std.h"
)

target_link_libraries("std" PRIVATE ob::core)

target_include_directories("std" PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

#target_include_directories("ob-std" PRIVATE $<TARGET_PROPERTY:ql,INCLUDE_DIRECTORIES>)

# TODO: wait until cmake issue #26584 is solved
# https://gitlab.kitware.com/cmake/cmake/-/issues/26584
# it should be OK for GCC and clang
target_compile_options("std" PRIVATE "--embed-dir=${CMAKE_CURRENT_BINARY_DIR}")

target_compile_definitions("std" PRIVATE OB_IMPLEMENTATION)

if(OB_INSTALL)
  install(
    TARGETS "std"
    EXPORT ob
    FILE_SET HEADERS
      DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  )
endif()
