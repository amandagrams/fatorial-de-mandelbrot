cmake_minimum_required(VERSION 3.7)
project(fractaldemondelbro)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c main.h)
add_library(fractaldemondelbro ${SOURCE_FILES})