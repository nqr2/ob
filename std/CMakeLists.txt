function(obc_header PATH)
  get_filename_component(BASENAME ${PATH} NAME_WE)

  obc(${PATH})
  xxdi("${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.obc")
endfunction()

set(STD_HEADERS
  "Shell.h"
  "Boolean.h"
)

obc_header("lib/Shell.ob")
obc_header("lib/Boolean.ob")

add_library("ObStd" STATIC
  "src/Std.c"
  "src/AddMethods.c"
  "src/Activation.c"
  "src/String.c"
  "src/Object.c"
  "src/Method.c"
  "src/Slots.c"
  "src/Shell.c"
  "src/World.c"

  ${STD_HEADERS}
)

install(
  DIRECTORY "include/ob"
  TYPE INCLUDE
)

target_link_libraries("ObStd" PUBLIC "ObCore")

target_include_directories("ObStd" PUBLIC "include")
target_include_directories("ObStd" PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
