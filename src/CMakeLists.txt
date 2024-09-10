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

#

if (NOT DEFINED PROJECT_SOURCELIB)
    set(PROJECT_SOURCELIB ${PROJECT_NAME}-src)
    set(PROJECT_SOURCELIB ${PROJECT_SOURCELIB} PARENT_SCOPE)
endif()

add_library(
    ${PROJECT_SOURCELIB} STATIC
    redback/error.c
    lib.c)

target_compile_options(
    ${PROJECT_SOURCELIB}
    PRIVATE ${C_COMPILE_OPTIONS})

target_include_directories(
    ${PROJECT_SOURCELIB}
    PRIVATE $<TARGET_PROPERTY:event_core,INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:event_extra,INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:event_pthreads,INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:event_openssl,INCLUDE_DIRECTORIES>
    PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(
    ${PROJECT_SOURCELIB}
    PRIVATE event_core
    PRIVATE event_extra
    PRIVATE event_pthreads
    PRIVATE event_openssl)

if (DEFINED PROJECT_HEADERS)
    add_dependencies(${PROJECT_SOURCELIB} ${PROJECT_HEADERS})
endif()


if(DEFINED CLANG_TIDY_COMMAND)
    set_property(
        TARGET ${PROJECT_SOURCELIB}
        PROPERTY CXX_CLANG_TIDY ${CLANG_TIDY_COMMAND})
endif()

#
