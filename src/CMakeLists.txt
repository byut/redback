cmake_minimum_required(VERSION 3.5)

#

if (NOT DEFINED PROJECT_SOURCELIB)
    set(PROJECT_SOURCELIB ${PROJECT_NAME}-src)
    set(PROJECT_SOURCELIB ${PROJECT_SOURCELIB} PARENT_SCOPE)
endif()

add_library(
    ${PROJECT_SOURCELIB} STATIC
    lib.c)

target_compile_options(
    ${PROJECT_SOURCELIB}
    PRIVATE ${C_COMPILE_OPTIONS})

target_include_directories(
    ${PROJECT_SOURCELIB}
    PUBLIC ${CMAKE_SOURCE_DIR}/include)

if (DEFINED PROJECT_HEADERS)
    add_dependencies(${PROJECT_SOURCELIB} ${PROJECT_HEADERS})
endif()


if(DEFINED CLANG_TIDY_COMMAND)
    set_property(
        TARGET ${PROJECT_SOURCELIB}
        PROPERTY CXX_CLANG_TIDY ${CLANG_TIDY_COMMAND})
endif()

#
