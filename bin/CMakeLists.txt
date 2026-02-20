add_executable(
  "exe_ob"
  "src/ob.c"
)

set_target_properties("exe_ob" PROPERTIES OUTPUT_NAME "ob")
target_link_libraries("exe_ob" PRIVATE ob::ob)
target_include_directories(exe_ob PRIVATE
$<TARGET_PROPERTY:ObCore,INCLUDE_DIRECTORIES>
$<TARGET_PROPERTY:ObStd,INCLUDE_DIRECTORIES>
)

install(TARGETS "exe_ob" RUNTIME)
