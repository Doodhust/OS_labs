cmake_minimum_required(VERSION 3.10)
project(TemperatureMonitor)

# Указываем стандарт C++ (C++11 или выше)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Добавляем поддиректорию server/
add_subdirectory(server)