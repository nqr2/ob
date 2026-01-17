find_package(Doxygen)

if(DOXYGEN_FOUND)
  set(PROJECT_NAME ${CMAKE_PROJECT_NAME})
  set(PROJECT_VERSION ${CMAKE_PROJECT_VERSION})
  set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/doc)
  set(SOURCE_DIR ${CMAKE_SOURCE_DIR})

  configure_file(Doxyfile.in Doxyfile)

  doxygen_add_docs(
    docs
    CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
  )
else()
endif()
