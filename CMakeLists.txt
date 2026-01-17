cmake_minimum_required(VERSION 4.1.2)

project(
  "ob"
  LANGUAGES C
  VERSION 0.0.0
  HOMEPAGE_URL "https://github.com/nqr2/ob"
  DESCRIPTION "A pure object-oriented language"
)

add_subdirectory("doc")
add_subdirectory("tap")
add_subdirectory("core")
add_subdirectory("tools")
add_subdirectory("std")
add_subdirectory("bin")
