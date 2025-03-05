cmake_minimum_required(VERSION 3.19)


# CMake runtime

if(NOT CMAKE_BINARY_DIR STREQUAL ${CMAKE_SOURCE_DIR}/.cmake)
  message(FATAL_ERROR "Run \"cmake -B .cmake\"")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../.build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../.build)


# Setting up project

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


# Setting up C++

# <thread> support
set(CMAKE_CXX_FLAGS "-pthread")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Collect utils into variable

file(GLOB
  utils_SRC
  CONFIGURE_DEPENDS
  "utils/npipe/npipe.h"
  "utils/npipe/npipe.cc"
  "utils/measur.h"
)


# random measuring device - random_temp_pipe
add_executable(device
  "device.cc"
  ${utils_SRC}
)
target_include_directories(device PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


# logger
add_executable(logger
  "logger.cc"
  ${utils_SRC}
)
target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
