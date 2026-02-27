add_library("static" STATIC)

target_link_libraries("static" PUBLIC ob::base ob::core ob::std)
set_target_properties("static" PROPERTIES OUTPUT_NAME "ob")

target_include_directories("static" PUBLIC
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_library(ob::static ALIAS "static")

if(OB_INSTALL)
  install(
    TARGETS "static"
    EXPORT ob
    ARCHIVE
  )
endif()
