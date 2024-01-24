cmake_minimum_required(VERSION 3.27)
project(projekt2)

set(CMAKE_CXX_STANDARD 23)

add_executable(projekt2 main.cpp
Logger.h Logger.cpp
        Scheduler.cpp
        Scheduler.h)
