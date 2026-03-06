# Unit tests

add_library("drv" OBJECT
  "lib/Test.h"
  "lib/Test.c"
)

target_include_directories("drv" PUBLIC "lib")
target_link_libraries("drv" PUBLIC "ob::base" "ob::core")

add_executable("driver"
  "src/drv.c"
  "src/suite.c"

  "src/base/suite.c"
  "src/base/allocator.c"
  "src/base/argparse.c"
  "src/base/assert.c"
  "src/base/hash.c"
  "src/base/table.c"

  "src/core/suite.c"
  "src/core/get_slot.c"
)

target_link_libraries("driver" PUBLIC "drv" "ob::static")

add_custom_command(
  TARGET "driver" POST_BUILD
  COMMAND "driver" "-l" ">" "test_list.txt"
  COMMENT "listing every unit test"
)

configure_file(
  "${CMAKE_SOURCE_DIR}/.cmake/unit-test-script.cmake.in"
  "unit-test-script.cmake"
  @ONLY
)

set_property(
  DIRECTORY APPEND
  PROPERTY TEST_INCLUDE_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/unit-test-script.cmake"
)

# Hosted tests:
# - expect tests on scripts (check outputs)
# - check tests on scripts  (check exit code)

add_executable("xd" "bin/xd.c")
target_link_libraries("xd" PRIVATE ob::base)

file(GLOB EXPECT_TESTS "expect/*.ob")
file(GLOB CHECK_TESTS "check/*.ob")

foreach(test ${EXPECT_TESTS})
  cmake_path(GET test STEM basename)
  cmake_path(GET test FILENAME filename)

  add_test(NAME "expect/${basename}"
   COMMAND ${CMAKE_COMMAND}
     -D "PROGRAM:STRING=${test}"
     -D "NAME:STRING=${basename}"
     -D "FILENAME:STRING=${filename}"
     -D "EXPECT:STRING=${CMAKE_CURRENT_SOURCE_DIR}/expect/${basename}.expect"
     -D "OB_COMMAND:STRING=$<TARGET_PROPERTY:ob,BINARY_DIR>/ob"
     -P "${CMAKE_SOURCE_DIR}/.cmake/expect-test-script.cmake"
  )
endforeach()

foreach(test ${CHECK_TESTS})
  cmake_path(GET test STEM basename)

  add_test(NAME "check/${basename}"
    COMMAND ob --verbose 0 "${test}"
  )

  if(${basename} MATCHES "^fail_")
    set_property(
      TEST "check/${basename}"
      PROPERTY WILL_FAIL YES
    )
  endif()
endforeach()
