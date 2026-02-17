cmake_minimum_required(VERSION 3.28)
project(mana)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_SCAN_FOR_MODULES FALSE) # prevents gcc expecting cppm files

set(GIT_SHALLOW TRUE)
include(FetchContent)

# spdlog
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(spdlog)

# Catch
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.0
)
FetchContent_MakeAvailable(Catch2)

find_package(Git QUIET)

set(MANA_BUILD_REV "0")
if (GIT_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} -C ${CMAKE_SOURCE_DIR} rev-list --count HEAD
            OUTPUT_VARIABLE MANA_BUILD_REV
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif ()

set(EXT_LIBS_MANA ${CMAKE_SOURCE_DIR}/extlib)

function(copy_post_build target)
    add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/mana/samples
            ${PROJECT_BINARY_DIR}/mana/samples/
    )
endfunction()

add_subdirectory(salem)
add_subdirectory(sigil)
add_subdirectory(hexec)

add_subdirectory(hex)
add_subdirectory(circe)
add_dependencies(hex circe)

add_subdirectory(tests)
