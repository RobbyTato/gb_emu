cmake_minimum_required(VERSION 3.15)

project(GBEMU LANGUAGES C)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.c")

add_executable(main ${SOURCES})