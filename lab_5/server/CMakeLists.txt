cmake_minimum_required(VERSION 3.10)
project(TemperatureServer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(temperature_server server.cpp)

find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(nlohmann_json 3.11.3 REQUIRED)

target_link_libraries(temperature_server
    PRIVATE
    Threads::Threads
    SQLite::SQLite3
    nlohmann_json::nlohmann_json
)

target_include_directories(temperature_server
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)