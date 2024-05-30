cmake_minimum_required(VERSION 3.27)
project(SimGame)

set(CMAKE_CXX_STANDARD 20)

add_executable(SimGame main.cpp
        WorldMap.h
        WorldMap.cpp
        TaskHandler.cpp
        TaskHandler.h
        Task.cpp
        Task.h)