add_library("ob" STATIC
  $<TARGET_OBJECTS:ObCore>
  $<TARGET_OBJECTS:ObStd>
  $<TARGET_OBJECTS:ql::ql>
)

target_include_directories("ob" PUBLIC
  $<TARGET_PROPERTY:ql::ql,INCLUDE_DIRECTORIES>
  $<TARGET_PROPERTY:ObCore,INCLUDE_DIRECTORIES>
  $<TARGET_PROPERTY:ObStd,INCLUDE_DIRECTORIES>
)

add_library(ob::ob ALIAS "ob")

install(TARGETS "ob" ARCHIVE)

if(OB_BUILD_TESTS)
  add_subdirectory("tests")
endif()
