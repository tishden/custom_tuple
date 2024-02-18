cmake_minimum_required(VERSION 3.26)
project(custom_tuple)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

add_executable(custom_tuple main.cpp
        core/CustomTuple.h)
