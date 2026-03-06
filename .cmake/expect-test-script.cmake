set(input "${NAME}.input")

execute_process(
  COMMAND ${OB_COMMAND} --verbose 0 ${PROGRAM}
  OUTPUT_VARIABLE stdout
  ERROR_VARIABLE stderr
)

file(READ "${PROGRAM}" program)

file(WRITE "${input}"
  "-- PROGRAM ${FILENAME} --\n"
  "${program}"
  "-- STDOUT --\n"
  "${stdout}"
  "-- STDERR --\n"
  "${stderr}"
)

execute_process(
  COMMAND "${CMAKE_CURRENT_BINARY_DIR}/xd" -e "${EXPECT}" -i "${input}"
  COMMAND_ERROR_IS_FATAL ANY
)

file(REMOVE "${input}")
