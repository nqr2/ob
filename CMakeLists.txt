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

option(BUILD_DOCS "Build the documentation with Doxygen" ON)
option(BUILD_TESTS "Build the tests" ON)

if(BUILD_DOCS)
  add_subdirectory("doc")
endif()

if(BUILD_TESTS)
  include(CTest)
  enable_testing()
endif()

include(FetchContent)

FetchContent_Declare(
  "ql"
  GIT_REPOSITORY         "https://github.com/nqr2/ql"
  GIT_TAG                "main"
  GIT_SUBMODULES_RECURSE NO
)

FetchContent_MakeAvailable("ql")

add_subdirectory("core")
add_subdirectory("std")
add_subdirectory("lib")
add_subdirectory("bin")
