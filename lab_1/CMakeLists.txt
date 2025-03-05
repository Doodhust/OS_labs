# this is the example CMakeLists.txt

cmake_minimum_required(VERSION 3.19)


# Require CMake work in .cmake directory

if(NOT CMAKE_BINARY_DIR STREQUAL ${CMAKE_SOURCE_DIR}/.cmake)
  message(FATAL_ERROR "Run \"cmake -B .cmake\"")
endif()


# Set CMake output directory to .build

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../.build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../.build)


# Describe project

project(base
  VERSION 0.1.0
  DESCRIPTION "base"
  LANGUAGES CXX
)


# Set build type

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


# Setting up C++:

# <thread> support
set(CMAKE_CXX_FLAGS "-pthread")

# requiring standard
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Setting up Qt

find_package(Qt5 COMPONENTS Core Gui Network Widgets Charts REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)


# Other external libraries: Boost, CURL, SQLite3

find_package(Boost COMPONENTS json REQUIRED)
find_package(CURL REQUIRED)
find_package(SQLite3 REQUIRED)


# Collect filenames into variable

file(GLOB
  base_SRC
  CONFIGURE_DEPENDS
  "src/h/*.h"
  "src/cc/*.cc"
)


# base library target

add_library(base
  SHARED
  src/api.h
  ${base_SRC}
)


# base executable target

add_executable(base
  src/base.cc
  ${base_SRC}
)
target_link_libraries(base
  PRIVATE 
  ${Qt5_LIBRARIES}
  ${Boost_JSON_LIBRARY}
  ${CURL_LIBRARIES}
  ${SQLite3_LIBRARIES}
)
# make absolute header names visible
target_include_directories(base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
