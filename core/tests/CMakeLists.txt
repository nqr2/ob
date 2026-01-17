set(CORE_TESTS
  "allocator"
  "argparse"
  "assert"
  "hash"
  "table"
)

foreach(T ${CORE_TESTS})
  add_executable(
    "test__core_${T}"
    "${T}.c"
  )

  target_link_libraries("test__core_${T}" PRIVATE "Tap" "ObCore")

  add_test(
    NAME ${T}
    COMMAND "test__core_${T}"
  )
endforeach()
