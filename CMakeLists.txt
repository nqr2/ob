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
option(ENABLE_ASSERTS "Enable the ASSERT macro" ON)
option(ENABLE_LOG "Enable the LOG macro" ON)

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
  GIT_TAG                "da8d6e90c4b2ccf3f98299521886a18b6332a59f"
  GIT_SUBMODULES_RECURSE NO
)

FetchContent_MakeAvailable("ql")

add_subdirectory("core")
add_subdirectory("tools")
add_subdirectory("std")
add_subdirectory("bin")
