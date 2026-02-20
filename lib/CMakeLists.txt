add_library("ob" STATIC "stub.c")

target_sources("ob" PRIVATE
  $<TARGET_OBJECTS:ObCore>
  $<TARGET_OBJECTS:ObStd>
  $<TARGET_OBJECTS:ql>
)

target_include_directories("ob" PRIVATE
  $<BUILD_INTERFACE:
    $<TARGET_PROPERTY:ql,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:ObCore,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:ObStd,INCLUDE_DIRECTORIES>
  >

  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_library(ob::ob ALIAS "ob")

if(OB_BUILD_TESTS)
  add_subdirectory("tests")
endif()

install(
  TARGETS "ob"
  EXPORT ob-config
  ARCHIVE
)
