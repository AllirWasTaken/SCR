cmake_minimum_required(VERSION 3.27)
project(comms)

set(CMAKE_CXX_STANDARD 23)

add_executable(comms main.cpp
        Comunication.cpp
        Comunication.h)
