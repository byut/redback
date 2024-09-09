cmake_minimum_required(VERSION 3.5)

#

if(NOT TARGET unity)
    message(AUTHOR_WARNING "Unity target not found. Skipping unit test generation ...")
    return()
endif()

include(CTest)
enable_testing()

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

#

add_test(${PROJECT_UNITY_TESTS} ${PROJECT_UNITY_TESTS})

#
