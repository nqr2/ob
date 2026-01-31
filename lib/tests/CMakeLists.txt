set(OB_TESTS
  "etc/always_pass.c"
)

create_test_sourcelist(
  OB_TEST_DRIVER
  test_driver.c
  ${OB_TESTS}
)

add_executable(
  test_driver
  ${OB_TEST_DRIVER}
)

foreach(test ${OB_TESTS})
  get_filename_component(basename ${test} NAME_WE)
  get_filename_component(dirname ${test} DIRECTORY)
  set(name "${dirname}/${basename}")

  add_test(
    NAME "${name}"
    COMMAND test_driver ${name}
  )
endforeach()
