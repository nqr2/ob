set(QL_TESTS
  "allocator"
  "argparse"
  "assert"
  "hash"
  "table"
)

set(TAP_TESTS
  "empty_suite"
  "passing_test"
  "failing_test"
  "skipped_test"
)

foreach(T ${QL_TESTS})
  add_executable(
    "test__ql_${T}"
    "${T}.c"
    $<TARGET_OBJECTS:ql>
  )

  target_link_libraries("test__ql_${T}" PRIVATE ql::ql)

  add_test(
    NAME "ql::${T}"
    COMMAND "test__ql_${T}"
  )
endforeach()

foreach(T ${TAP_TESTS})
  add_executable(
    "test__ql_tap_${T}"
    "${T}.c"
    $<TARGET_OBJECTS:ql>
  )

  target_link_libraries("test__ql_tap_${T}" PRIVATE ql::ql)

  add_test(
    NAME "ql_tap::${T}"
    COMMAND "test__ql_tap_${T}"
  )
endforeach()
