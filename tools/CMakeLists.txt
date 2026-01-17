add_executable(
  "xxdi"
  "src/xxdi.c"
)

add_executable(
  "obc"
  "src/obc.c"
)

target_link_libraries("obc" PRIVATE "ObCore")

add_executable(
  "dis"
  "src/dis.c"
)

target_link_libraries("dis" PRIVATE "ObCore")

function(xxdi INPUT)
  get_filename_component(BASENAME ${INPUT} NAME_WE)

  add_custom_command(
    OUTPUT "${BASENAME}.h"
    COMMAND "xxdi" "${INPUT}" "${BASENAME}.h" "${BASENAME}"
    DEPENDS "${INPUT}"
    VERBATIM
  )

  add_custom_target("${BASENAME}.h" DEPENDS ${INPUT})
endfunction()

function(obc INPUT)
  get_filename_component(BASENAME ${INPUT} NAME_WE)

  add_custom_command(
    OUTPUT "${BASENAME}.obc"
    COMMAND "obc" "${INPUT}" > "${BASENAME}.obc"
    DEPENDS "${INPUT}"
    VERBATIM
  )

  add_custom_target("${BASENAME}.obc" DEPENDS ${INPUT})
endfunction()
