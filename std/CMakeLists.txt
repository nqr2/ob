## Building the tools to build ObStd

add_executable(
  "obc"
  "bin/obc.c"
)

target_link_libraries("obc" PRIVATE "ObCore")

add_executable(
  "dis"
  "bin/dis.c"
)

target_link_libraries("dis" PRIVATE "ObCore")

function(obc)
  cmake_parse_arguments(PARSE_ARGV 0 arg "" OUTPUT INPUTS)

  add_custom_target(
    "${arg_OUTPUT}"
    COMMAND "obc" ${arg_INPUTS} > "${CMAKE_CURRENT_BINARY_DIR}/${arg_OUTPUT}"
    SOURCES ${arg_INPUTS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
endfunction()

## Building ObStd itself

obc(
  OUTPUT "World.obc"
  INPUTS
    "World.ob"
)

add_library("ObStd" OBJECT
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

add_dependencies("ObStd" "World.obc")

install(
  DIRECTORY "include/ob"
  TYPE INCLUDE
)

target_link_libraries("ObStd" PUBLIC "ObCore")

target_include_directories("ObStd" PUBLIC "include")

# TODO: wait until cmake issue #26584 is solved
# https://gitlab.kitware.com/cmake/cmake/-/issues/26584
# it should be OK for GCC and clang
target_compile_options("ObStd" PRIVATE "--embed-dir=${CMAKE_CURRENT_BINARY_DIR}")

target_compile_definitions("ObStd" PRIVATE OB_IMPLEMENTATION)
