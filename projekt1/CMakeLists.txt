cmake_minimum_required(VERSION 3.26)
project(projekt1)

set(CMAKE_CXX_STANDARD 23)

add_executable(projekt1 main.cpp
        Logger.cpp
        Logger.h)
