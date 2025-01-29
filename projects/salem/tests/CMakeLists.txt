cmake_minimum_required(VERSION 3.28)
project(salem)

# Catch
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(salem-tests
    tests/lexing.cpp
    # tests/parsing.cpp
    # tests/error_reporting.cpp
)

target_include_directories(salem-tests PRIVATE include/)

target_link_libraries(salem-tests PRIVATE
    Catch2::Catch2WithMain
    salem::salem
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(salem-tests)

add_custom_command(
    TARGET salem-tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_SOURCE_DIR}/assets
    ${PROJECT_BINARY_DIR}/assets/
)