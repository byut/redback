cmake_minimum_required(VERSION 3.5)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

#

find_program(GCC_EXE gcc)
if (GCC_EXE)
    message(STATUS "Found gcc : ${GCC_EXE}.")
elseif(NOT GCC_EXE)
    message(FATAL_ERROR "gcc not found in $PATH.")
endif()

#

project(
    redback
    VERSION 0.1.0
    LANGUAGES C)

add_subdirectory(extern)

#

set(CMAKE_C_COMPILER ${GCC_EXE})
set(CMAKE_C_STANDARD 23)

set(C_COMPILE_OPTIONS
    -Werror
    -Wall
    -Wextra
    -pedantic)

#

set(PROJECT_EXECUTABLE ${PROJECT_NAME})

add_executable(
    ${PROJECT_EXECUTABLE}
    main.c)

target_compile_options(
    ${PROJECT_EXECUTABLE}
    PRIVATE ${C_COMPILE_OPTIONS})
