cmake_minimum_required(VERSION 3.9)
project(SOI_LAB6C C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-pthread")


add_executable(SOI_LAB6C main.c FileSystem.c FileSystem.h)