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
    PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(
    ${PROJECT_BASELIB})

if (DEFINED PROJECT_HEADERS)
    add_dependencies(${PROJECT_BASELIB} ${PROJECT_HEADERS})
endif()


if(DEFINED CLANG_TIDY_COMMAND)
    set_property(
        TARGET ${PROJECT_BASELIB}
        PROPERTY CXX_CLANG_TIDY ${CLANG_TIDY_COMMAND})
endif()

##### Network Unit ############################################################

if (NOT DEFINED PROJECT_NUNITLIB)
    set(PROJECT_NUNITLIB ${PROJECT_NAME}-nunit)
    set(PROJECT_NUNITLIB ${PROJECT_NUNITLIB} PARENT_SCOPE)
endif()

add_library(
    ${PROJECT_NUNITLIB} STATIC
    lib.c)

target_compile_options(
    ${PROJECT_NUNITLIB}
    PRIVATE ${C_COMPILE_OPTIONS})

target_include_directories(
    ${PROJECT_NUNITLIB}
    PUBLIC $<TARGET_PROPERTY:event_core,INCLUDE_DIRECTORIES>
    PUBLIC $<TARGET_PROPERTY:event_extra,INCLUDE_DIRECTORIES>
    PUBLIC $<TARGET_PROPERTY:event_pthreads,INCLUDE_DIRECTORIES>
    PUBLIC $<TARGET_PROPERTY:event_openssl,INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:${PROJECT_BASELIB},INCLUDE_DIRECTORIES>
    PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(
    ${PROJECT_NUNITLIB}
    PRIVATE event_core
    PRIVATE event_extra
    PRIVATE event_pthreads
    PRIVATE event_openssl
    PRIVATE ${PROJECT_BASELIB})

if (DEFINED PROJECT_HEADERS)
    add_dependencies(${PROJECT_NUNITLIB} ${PROJECT_HEADERS})
endif()


if(DEFINED CLANG_TIDY_COMMAND)
    set_property(
        TARGET ${PROJECT_NUNITLIB}
        PROPERTY CXX_CLANG_TIDY ${CLANG_TIDY_COMMAND})
endif()

##### Processing Unit #########################################################

if (NOT DEFINED PROJECT_PUNITLIB)
    set(PROJECT_PUNITLIB ${PROJECT_NAME}-punit)
    set(PROJECT_PUNITLIB ${PROJECT_PUNITLIB} PARENT_SCOPE)
endif()

add_library(
    ${PROJECT_PUNITLIB} STATIC
    lib.c)

target_compile_options(
    ${PROJECT_PUNITLIB}
    PRIVATE ${C_COMPILE_OPTIONS})

target_include_directories(
    ${PROJECT_PUNITLIB}
    PRIVATE $<TARGET_PROPERTY:${PROJECT_BASELIB},INCLUDE_DIRECTORIES>
    PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(
    ${PROJECT_PUNITLIB}
    PRIVATE ${PROJECT_BASELIB})

if (DEFINED PROJECT_HEADERS)
    add_dependencies(${PROJECT_PUNITLIB} ${PROJECT_HEADERS})
endif()


if(DEFINED CLANG_TIDY_COMMAND)
    set_property(
        TARGET ${PROJECT_PUNITLIB}
        PROPERTY CXX_CLANG_TIDY ${CLANG_TIDY_COMMAND})
endif()

##### Graphical Unit ##########################################################

find_package(Curses REQUIRED)

if (NOT DEFINED PROJECT_GUNITLIB)
    set(PROJECT_GUNITLIB ${PROJECT_NAME}-gunit)
    set(PROJECT_GUNITLIB ${PROJECT_GUNITLIB} PARENT_SCOPE)
endif()

add_library(
    ${PROJECT_GUNITLIB} STATIC
    redback/gunit.c
    lib.c)

target_compile_options(
    ${PROJECT_GUNITLIB}
    PRIVATE ${C_COMPILE_OPTIONS})

target_include_directories(
    ${PROJECT_GUNITLIB}
    PUBLIC $<TARGET_PROPERTY:event_core,INCLUDE_DIRECTORIES>
    PUBLIC $<TARGET_PROPERTY:event_extra,INCLUDE_DIRECTORIES>
    PUBLIC $<TARGET_PROPERTY:event_pthreads,INCLUDE_DIRECTORIES>
    PUBLIC $<TARGET_PROPERTY:event_openssl,INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:${PROJECT_BASELIB},INCLUDE_DIRECTORIES>
    PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(
    ${PROJECT_GUNITLIB}
    PRIVATE event_core
    PRIVATE event_extra
    PRIVATE event_pthreads
    PRIVATE event_openssl
    PRIVATE ${CURSES_LIBRARIES}
    PRIVATE ${PROJECT_BASELIB})

if (DEFINED PROJECT_HEADERS)
    add_dependencies(${PROJECT_GUNITLIB} ${PROJECT_HEADERS})
endif()


if(DEFINED CLANG_TIDY_COMMAND)
    set_property(
        TARGET ${PROJECT_GUNITLIB}
        PROPERTY CXX_CLANG_TIDY ${CLANG_TIDY_COMMAND})
endif()
