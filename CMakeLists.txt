cmake_minimum_required(VERSION 3.15)

project(GBEMU LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_compile_definitions(SDL_VIDEO_DRIVER_X11_SUPPORTS_GENERIC_EVENTS=0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.c")

add_subdirectory(vendored/SDL)

add_executable(gbemu ${SOURCES})
target_include_directories(gbemu PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL/include
)
target_link_libraries(gbemu PRIVATE SDL3::SDL3)
target_compile_options(gbemu PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wpointer-arith
    -Wcast-align
    -Wundef
    -Waggregate-return
    -Wimplicit-fallthrough
    -Wno-unused-function
    -Wno-unused-parameter
    -O3
)