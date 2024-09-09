cmake_minimum_required(VERSION 3.5)

#

if(NOT TARGET unity)
    message(AUTHOR_WARNING "Unity target not found. Skipping unit test generation ...")
    return()
endif()

include(CTest)
enable_testing()

#

if (DEFINED VALGRIND_COMMAND)
    set(DART_CONFIGURATION ${CMAKE_CURRENT_BINARY_DIR}/DartConfiguration.tcl)
    write_file(${DART_CONFIGURATION} "MemoryCheckCommand: ${VALGRIND_COMMAND}")
    write_file(${DART_CONFIGURATION} "MemoryCheckCommandOptions: ${VALGRIND_COMMAND_OPTIONS}" APPEND)
endif()

#

set(PROJECT_UNITY_TESTS ${PROJECT_NAME}-unity-tests)
set(PROJECT_UNITY_LIBTARGET unity)

add_executable(
    ${PROJECT_UNITY_TESTS}
    test.c)

target_compile_options(
    ${PROJECT_UNITY_TESTS}
    PRIVATE ${C_COMPILE_OPTIONS})

target_include_directories(
    ${PROJECT_UNITY_TESTS}
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    PRIVATE $<TARGET_PROPERTY:${PROJECT_UNITY_LIBTARGET},INCLUDE_DIRECTORIES>)

target_link_libraries(
    ${PROJECT_UNITY_TESTS}
    PRIVATE ${PROJECT_UNITY_LIBTARGET})

if (DEFINED PROJECT_HEADERS)
    add_dependencies(${PROJECT_UNITY_TESTS} ${PROJECT_HEADERS})
endif()

if(DEFINED CLANG_TIDY_COMMAND)
    set_property(
        TARGET ${PROJECT_UNITY_TESTS}
        PROPERTY CXX_CLANG_TIDY ${CLANG_TIDY_COMMAND})
endif()

#

add_test(${PROJECT_UNITY_TESTS} ${PROJECT_UNITY_TESTS})

#
