cmake_minimum_required(VERSION 3.19)
project(HW_3_Liad C)

set(CMAKE_C_STANDARD 11)

add_executable(HW_3_Liad server.c segel.c segel.h request.h request.c)