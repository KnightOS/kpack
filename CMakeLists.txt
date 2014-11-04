cmake_minimum_required(VERSION 2.8.5)
project(mkpkg C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
add_definitions("-Wall")

if(EMSCRIPTEN)
  set(CMAKE_EXE_LINKER_FLAGS "--pre-js ${CMAKE_CURRENT_SOURCE_DIR}/module.js")
  set(CMAKE_C_FLAGS "-O2")
endif()

add_executable(kpack
    src/main.c
    src/packager.c
    src/checksums.c
    src/unpack.c
)

if(WIN32 OR CYGWIN)
  install(PROGRAMS bin/kpack.exe
      DESTINATION bin)
else()
  install(PROGRAMS bin/kpack
      DESTINATION bin)
endif()

set(CMAKE_BUILD_TYPE Release)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/kpack.1
  COMMAND a2x --no-xmllint --doctype manpage --format manpage ${CMAKE_CURRENT_SOURCE_DIR}/kpack.1.txt 
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kpack.1.txt
)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/kpack.5
  COMMAND a2x --no-xmllint --doctype manpage --format manpage ${CMAKE_CURRENT_SOURCE_DIR}/kpack.5.txt 
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kpack.5.txt
)

ADD_CUSTOM_TARGET(man ALL
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kpack.1
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kpack.5)

INSTALL(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/kpack.1
  DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1
)

INSTALL(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/kpack.5
  DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man5
)
