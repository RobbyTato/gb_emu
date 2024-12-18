cmake_minimum_required(VERSION 3.15)

project(GBEMU LANGUAGES C)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.c")

add_subdirectory(json-c)

add_executable(gbemu ${SOURCES})
target_link_libraries(gbemu PRIVATE json-c)
target_include_directories(gbemu PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/MathFunctions"
                          )