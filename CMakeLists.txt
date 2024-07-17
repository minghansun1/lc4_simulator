cmake_minimum_required(VERSION 3.27)
project(project_lc4_simulator)

set(CMAKE_CXX_STANDARD 17)

add_executable(project_lc4_simulator
        loader.c
        trace.c
        LC4.c
)
