add_executable(
  "ob"
  "src/ob.c"
)

target_link_libraries("ob" PRIVATE "ObCore" "ObStd")
