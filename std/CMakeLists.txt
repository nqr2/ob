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

function(obc INPUT)
  get_filename_component(BASENAME ${INPUT} NAME_WE)

  add_custom_command(
    OUTPUT "${BASENAME}.obc"
    COMMAND "obc" "${INPUT}" > "${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.obc"
    DEPENDS "${INPUT}"
    VERBATIM
  )

  add_custom_target("${BASENAME}.obc" DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.obc")
endfunction()

## Building ObStd itself

obc("lib/Shell.ob")
obc("lib/Boolean.ob")

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
)

add_dependencies("ObStd"
  "Shell.obc"
  "Boolean.obc"
)

install(
  DIRECTORY "include/ob"
  TYPE INCLUDE
)

target_link_libraries("ObStd" PUBLIC "ObCore")

target_include_directories("ObStd" PUBLIC "include")
target_include_directories("ObStd" PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
