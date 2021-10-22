cmake_minimum_required(VERSION 3.20)
project(lab_1_fw C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(lab_1_fw splpv1.c main.c splpv1.h)