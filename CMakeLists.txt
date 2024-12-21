cmake_minimum_required(VERSION 3.15)

project(GBEMU LANGUAGES C)

include(CheckCCompilerFlag)

function(enable_c_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_C_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_c_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
        unset(flag_supported CACHE)
    endif()
endfunction()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.c")

enable_c_compiler_flag_if_supported("-Wall")
enable_c_compiler_flag_if_supported("-Wextra")
enable_c_compiler_flag_if_supported("-Werror")
enable_c_compiler_flag_if_supported("-Wpedantic")
enable_c_compiler_flag_if_supported("-Wconversion")
enable_c_compiler_flag_if_supported("-Wfloat-equal")
enable_c_compiler_flag_if_supported("-Wundef")
enable_c_compiler_flag_if_supported("-Wshadow")
enable_c_compiler_flag_if_supported("-Wpointer-arith")
enable_c_compiler_flag_if_supported("-Wcast-align")
enable_c_compiler_flag_if_supported("-Wstrict-prototypes")
enable_c_compiler_flag_if_supported("-Wwrite-strings")
enable_c_compiler_flag_if_supported("-Wno-unused-function")
enable_c_compiler_flag_if_supported("-Wno-unused-parameter")
enable_c_compiler_flag_if_supported("-Waggregate-return")

enable_c_compiler_flag_if_supported("-O3")

add_executable(gbemu ${SOURCES})
target_include_directories(gbemu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)