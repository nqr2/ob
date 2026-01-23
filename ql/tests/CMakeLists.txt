set(QL_TESTS
  "allocator"
  "argparse"
  "assert"
  "hash"
  "table"
)

foreach(T ${QL_TESTS})
  add_executable(
    "test__ql_${T}"
    "${T}.c"
  )

  target_link_libraries("test__ql_${T}" PRIVATE "Tap" "Ql")

  add_test(
    NAME "ql::${T}"
    COMMAND "test__ql_${T}"
  )
endforeach()
