add_library(
  "Tap"
  "src/Tap.c"
)

target_include_directories("Tap" PUBLIC "src")

add_subdirectory("tests")
