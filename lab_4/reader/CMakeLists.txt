# reader/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(temperature_reader)

set(CMAKE_CXX_STANDARD 14)

add_executable(temperature_reader temperature_reader.cpp)