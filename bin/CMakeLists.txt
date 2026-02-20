add_executable(
  "ob"
  "src/ob.c"
)

target_link_libraries("ob" PUBLIC ob::static)

if(OB_INSTALL)
  install(
    TARGETS "ob"
    RUNTIME
  )
endif()
