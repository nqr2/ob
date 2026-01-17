set(TAP_TESTS
  "empty_suite"
  "passing_test"
  "failing_test"
  "skipped_test"
)

foreach(T ${TAP_TESTS})
  add_executable(
    "test__tap_${T}"
    "${T}.c"
  )

  target_link_libraries("test__tap_${T}" PRIVATE "Tap")

  add_test(
    NAME ${T}
    COMMAND "test__tap_${T}"
  )
endforeach()
