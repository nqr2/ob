add_executable(
  "exe_ob"
  "src/ob.c"
)

set_target_properties("exe_ob" PROPERTIES OUTPUT_NAME "ob")
target_link_libraries("exe_ob" PRIVATE ob::ob)

install(TARGETS "exe_ob" RUNTIME)
