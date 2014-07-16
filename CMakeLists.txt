cmake_minimum_required(VERSION 2.8.5)
project(mkpkg C)
set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")
add_definitions("-Wall")

add_executable(kpack
    main.c
)

install(PROGRAMS bin/kpack
    DESTINATION bin)

set(CMAKE_BUILD_TYPE Release)

ADD_CUSTOM_TARGET(man ALL)

ADD_CUSTOM_COMMAND(
  TARGET man
  SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/kpack.1.txt
  COMMAND a2x --no-xmllint --doctype manpage --format manpage ${CMAKE_CURRENT_SOURCE_DIR}/kpack.1.txt 
  OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/kpack.1
)

ADD_CUSTOM_COMMAND(
  TARGET man
  SOURCE man
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kpack.1
)

INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kpack.1
    DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1
)
