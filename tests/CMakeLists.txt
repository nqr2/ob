add_library(harness2 OBJECT
  "lib/Test.h"
  "lib/Test.c"
)

target_include_directories("harness2" PUBLIC "lib")
target_link_libraries("harness2" PUBLIC "ob::base")

add_executable("driver"
  "src/suite.c"
)

target_link_libraries("driver" PUBLIC "harness2" "ob::base")

file(GLOB HOSTED_TESTS
  "entries/*.ob"
)

foreach(test ${HOSTED_TESTS})
  get_filename_component(basename ${test} NAME_WE)
  get_filename_component(dirname ${test} DIRECTORY)

  cmake_path(RELATIVE_PATH test BASE_DIRECTORY ${CMAKE_SOURCE_DIR})

  set(fails FALSE)

  if(basename MATCHES "^fail_")
    set(fails TRUE)
  endif()

  add_test(
    NAME "capture/${basename}"
    COMMAND
       ob
       --capture-stdout "${CMAKE_CURRENT_BINARY_DIR}/${basename}.stdout"
       --capture-stderr "${CMAKE_CURRENT_BINARY_DIR}/${basename}.stderr"
       "${test}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )

  add_test(
    NAME "expect/${basename}.stdout"
    COMMAND ${CMAKE_COMMAND}
      -E compare_files
      --ignore-eol
      "${CMAKE_CURRENT_SOURCE_DIR}/stdout/${basename}"
      "${CMAKE_CURRENT_BINARY_DIR}/${basename}.stdout"
  )

  add_test(
    NAME "expect/${basename}.stderr"
    COMMAND ${CMAKE_COMMAND}
      -E compare_files
      --ignore-eol
      "${CMAKE_CURRENT_SOURCE_DIR}/stderr/${basename}"
      "${CMAKE_CURRENT_BINARY_DIR}/${basename}.stderr"
  )

  set_tests_properties(
    "capture/${basename}"
    PROPERTIES
      FIXTURES_SETUP "${basename}"
      WILL_FAIL ${fails}
  )

  set_tests_properties(
    "expect/${basename}.stdout"
     PROPERTIES
       DEPENDS "capture/${basename}"
       FIXTURES_REQUIRED "${basename}"
       REQUIRED_FILES "${CMAKE_CURRENT_SOURCE_DIR}/stdout/${basename}"
  )

  set_tests_properties(
    "expect/${basename}.stderr"
     PROPERTIES
       DEPENDS "capture/${basename}"
       FIXTURES_REQUIRED "${basename}"
       REQUIRED_FILES "${CMAKE_CURRENT_SOURCE_DIR}/stderr/${basename}"
  )
endforeach()
