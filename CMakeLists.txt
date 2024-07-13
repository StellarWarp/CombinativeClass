cmake_minimum_required(VERSION 3.24)
project(CombinativeClass)

set(CMAKE_CXX_STANDARD 23)

include_directories(src)

add_executable(CombinativeClass
        src/combinative.h
        src/main.cpp
        src/sample_type_info.h
        src/type_list.h)
