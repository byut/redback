cmake_minimum_required(VERSION 3.5)

# Save the timestamp of the current build
string(TIMESTAMP REDBACK_BUILD_TIMESTAMP "%Y-%m-%d %H:%M")

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

find_program(VALGRIND_EXE NAMES "valgrind")
if (VALGRIND_EXE)
    message(STATUS "Found valgrind : ${VALGRIND_EXE}.")
    set(VALGRIND_COMMAND ${VALGRIND_EXE})
    set(VALGRIND_COMMAND_OPTIONS --leak-check=full)
elseif (NOT VALGRIND_EXE)
    message(AUTHOR_WARNING "valgrind not found in $PATH.")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if (CLANG_TIDY_EXE)
        message(STATUS "Found clang-tidy : ${CLANG_TIDY_EXE}.")
        set(CLANG_TIDY_COMMAND ${CLANG_TIDY_EXE}
            --config-file ${CMAKE_SOURCE_DIR}/.clang-tidy
            -p ${CMAKE_BINARY_DIR}
            --quiet)
    elseif(NOT CLANG_TIDY_EXE)
        message(AUTHOR_WARNING "clang-tidy not found in $PATH.")
    endif()
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

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test)

#

set(PROJECT_EXECUTABLE ${PROJECT_NAME})

add_executable(
    ${PROJECT_EXECUTABLE}
    main.c)

target_compile_options(
    ${PROJECT_EXECUTABLE}
    PRIVATE ${C_COMPILE_OPTIONS})

target_compile_definitions(
    ${PROJECT_EXECUTABLE}
    PRIVATE -DREDBACK_PROJECT_NAME="${CMAKE_PROJECT_NAME}"
    PRIVATE -DREDBACK_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}"
    PRIVATE -DREDBACK_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
    PRIVATE -DREDBACK_BUILD_TIMESTAMP="${REDBACK_BUILD_TIMESTAMP}")

target_link_libraries(
    ${PROJECT_EXECUTABLE}
    PRIVATE ${PROJECT_LOGLIB}
    PRIVATE ${PROJECT_BASELIB}
    PRIVATE ${PROJECT_NUNITLIB}
    PRIVATE ${PROJECT_PUNITLIB}
    PRIVATE ${PROJECT_GUNITLIB})

target_include_directories(
    ${PROJECT_EXECUTABLE}
    PRIVATE ${PROJECT_SOURCE_DIR}/include)

if (DEFINED PROJECT_HEADERS)
    add_dependencies(${PROJECT_EXECUTABLE} ${PROJECT_HEADERS})
endif()

if(DEFINED CLANG_TIDY_COMMAND)
    set_property(
        TARGET ${PROJECT_EXECUTABLE}
        PROPERTY CXX_CLANG_TIDY ${CLANG_TIDY_COMMAND})
endif()
