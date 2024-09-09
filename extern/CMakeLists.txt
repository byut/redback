cmake_minimum_required(VERSION 3.5)

#

find_package(Git QUIET)

if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        RESULT_VARIABLE GIT_SUBMODULE_RESULT)
    if(NOT GIT_SUBMODULE_RESULT EQUAL "0")
        message(
            FATAL_ERROR
                "Couldn't initialize submodules. (${GIT_SUBMODULE_RESULT})")
    endif()
endif()

#

add_subdirectory(Unity EXCLUDE_FROM_ALL)

#
