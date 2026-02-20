cmake_minimum_required(VERSION 4.1.2)

project(
  "ob"
  LANGUAGES C
  VERSION 0.0.0
  HOMEPAGE_URL "https://github.com/nqr2/ob"
  DESCRIPTION "A pure object-oriented language"
)

set(CMAKE_C_STANDARD 23 REQUIRED)
set(CMAKE_C_EXTENSIONS NO)

option(OB_BUILD_DOCS     "Build the documentation with Doxygen" ON)
option(OB_BUILD_TESTS    "Build the tests"                      ON)
option(OB_ENABLE_ASSERTS "Enable the OB_ASSERT macro"           ON)
option(OB_ENABLE_LOG     "Enable the OB_LOG macro"              ON)

if(OB_BUILD_DOCS)
  add_subdirectory("doc")
endif()

if(OB_BUILD_TESTS)
  include(CTest)
  enable_testing()
endif()

add_subdirectory("base")
add_subdirectory("core")
add_subdirectory("std")
add_subdirectory("lib")
add_subdirectory("bin")

include(GNUInstallDirs)
export(
  TARGETS ob
  NAMESPACE ob::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/ob-config.cmake"
)

install(
  EXPORT ob-config
  DESTINATION "${CMAKE_INSTALL_DATADIR}/"
  NAMESPACE ob::
)

if(OB_BUILD_TESTS)
  add_subdirectory("tests")
endif()
