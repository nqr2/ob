add_library("ObCore" OBJECT
  "include/ob/bits/Begin.h"
  "include/ob/bits/End.h"

  "include/ob/core/Bytecode.h"
  "include/ob/core/Context.h"
  "include/ob/core/Memory.h"
  "include/ob/core/Object.h"
  "include/ob/core/Parse.h"
  "include/ob/core/Serial.h"
  "include/ob/core/String.h"

  "include/ob/Core.h"

  "src/Bytecode.c"
  "src/Context.c"
  "src/Memory.c"
  "src/Object.c"
  "src/Parse.c"
  "src/Serial.c"
  "src/String.c"
)

target_include_directories("ObCore" PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries("ObCore" PRIVATE $<TARGET_OBJECTS:ql>)
target_include_directories("ObCore" PRIVATE $<TARGET_PROPERTY:ql,INCLUDE_DIRECTORIES>)

target_compile_definitions("ObCore" PRIVATE OB_IMPLEMENTATION)

install(
  DIRECTORY "include/ob"
  TYPE INCLUDE
)


install(
  TARGETS "ObCore"
  EXPORT ob-config
  INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
#  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
#  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
