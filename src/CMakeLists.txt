cmake_minimum_required(VERSION 3.5)

#

if (NOT DEFINED PROJECT_LOGLIB)
    set(PROJECT_LOGLIB ${PROJECT_NAME}-log)
    set(PROJECT_LOGLIB ${PROJECT_LOGLIB} PARENT_SCOPE)
endif()

add_library(
    ${PROJECT_LOGLIB} STATIC
    log.c)

target_compile_definitions(
    ${PROJECT_LOGLIB}
    PRIVATE -DLOG_USE_COLOR)

target_include_directories(
    ${PROJECT_LOGLIB}
    PUBLIC ${CMAKE_SOURCE_DIR}/include)

##### Base Lib ################################################################

if (NOT DEFINED PROJECT_BASELIB)
    set(PROJECT_BASELIB ${PROJECT_NAME}-base)
    set(PROJECT_BASELIB ${PROJECT_BASELIB} PARENT_SCOPE)
endif()

add_library(
    ${PROJECT_BASELIB} STATIC
    redback/error.c
    redback/memory_pool.c
    lib.c)

target_compile_options(
    ${PROJECT_BASELIB}
    PRIVATE ${C_COMPILE_OPTIONS})

target_include_directories(
    ${PROJECT_BASELIB}
    PRIVATE $<TARGET_PROPERTY:event_core,INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:event_extra,INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:event_pthreads,INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:event_openssl,INCLUDE_DIRECTORIES>
    PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(
    ${PROJECT_BASELIB}
    PRIVATE event_core
    PRIVATE event_extra
    PRIVATE event_pthreads
    PRIVATE event_openssl)

if (DEFINED PROJECT_HEADERS)
    add_dependencies(${PROJECT_BASELIB} ${PROJECT_HEADERS})
endif()


if(DEFINED CLANG_TIDY_COMMAND)
    set_property(
        TARGET ${PROJECT_BASELIB}
        PROPERTY CXX_CLANG_TIDY ${CLANG_TIDY_COMMAND})
endif()

#
