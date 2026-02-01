file(GLOB HOSTED_TESTS
  "entries/*.ob"
)

foreach(test ${HOSTED_TESTS})
  get_filename_component(basename ${test} NAME)
  get_filename_component(dirname ${test} DIRECTORY)

  message("${basename} :: ${dirname} :: ${CMAKE_CURRENT_BINARY_DIR}/${basename}.stdout")

  add_test(
    NAME "capture/${basename}"
    COMMAND
       exe_ob
       --capture-stdout "${CMAKE_CURRENT_BINARY_DIR}/${basename}.stdout"
       --capture-stderr "${CMAKE_CURRENT_BINARY_DIR}/${basename}.stderr"
       "${test}"
  )

  add_test(
    NAME "expect/${basename}.stdout"
    COMMAND ${CMAKE_COMMAND}
      -E compare_files
      --ignore-eol
      "${test}.stdout"
      "${CMAKE_CURRENT_BINARY_DIR}/${basename}.stdout"
  )

  add_test(
    NAME "expect/${basename}.stderr"
    COMMAND ${CMAKE_COMMAND}
      -E compare_files
      --ignore-eol
      "${test}.stderr"
      "${CMAKE_CURRENT_BINARY_DIR}/${basename}.stderr"
  )

  set_tests_properties(
    "capture/${basename}"
    PROPERTIES
      FIXTURES_SETUP "${basename}"
  )

  set_tests_properties(
    "expect/${basename}.stdout"
     PROPERTIES
       DEPENDS "capture/${basename}"
       FIXTURES_REQUIRED "${basename}"
       REQUIRED_FILES "${basename}.stdout"
  )

  set_tests_properties(
    "expect/${basename}.stderr"
     PROPERTIES
       DEPENDS "capture/${basename}"
       FIXTURES_REQUIRED "${basename}"
       REQUIRED_FILES "${basename}.stderr"
  )
endforeach()
