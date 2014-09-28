cmake_minimum_required(VERSION 2.8.5)
project(mkpkg C)
set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
add_definitions("-Wall")

add_executable(kpack
    src/main.c
    src/packager.c
)

install(PROGRAMS bin/kpack
    DESTINATION bin)

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
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/kpack.1 ${CMAKE_CURRENT_SOURCE_DIR}/kpack.5
  DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1
)
