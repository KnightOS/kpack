cmake_minimum_required(VERSION 2.8.5)
project(kpack C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Werror")

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")
endif()

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
target_include_directories(kpack PRIVATE ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Git)

if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --dirty --always --abbrev=0
        OUTPUT_VARIABLE GIT_SHORT_VERSION
        ERROR_QUIET
        )
    if(NOT ${GIT_SHORT_VERSION} STREQUAL "")
        string(STRIP ${GIT_SHORT_VERSION} GIT_SHORT_VERSION)
    endif()
    message(STATUS "Current git revision is ${GIT_SHORT_VERSION}")
endif()

configure_file(${CMAKE_SOURCE_DIR}/src/version.h.in ${CMAKE_BINARY_DIR}/version.h)

install(TARGETS kpack DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_BINARY_DIR}/kpack.1
  COMMAND a2x --no-xmllint --doctype manpage --format manpage ${CMAKE_SOURCE_DIR}/kpack.1.txt -D ${CMAKE_BINARY_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kpack.1.txt
)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_BINARY_DIR}/kpack.5
  COMMAND a2x --no-xmllint --doctype manpage --format manpage ${CMAKE_SOURCE_DIR}/kpack.5.txt -D ${CMAKE_BINARY_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kpack.5.txt
)

ADD_CUSTOM_TARGET(man ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kpack.1
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kpack.5)

INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kpack.1 ${CMAKE_CURRENT_BINARY_DIR}/kpack.5
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1
)
