cmake_minimum_required(VERSION 3.15)
project(producer-consumer-simulation C)

set(CMAKE_C_STANDARD 99)

find_package(Threads)
add_executable(simulator simulator.c)
target_link_libraries(simulator ${CMAKE_THREAD_LIBS_INIT})
