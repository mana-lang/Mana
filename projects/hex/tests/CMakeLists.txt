cmake_minimum_required(VERSION 3.28)
project(hex)

add_executable(hex-tests
    tests/lexing.cpp
    # tests/parsing.cpp
    # tests/error_reporting.cpp
)

target_include_directories(hex-tests PRIVATE include/)

target_link_libraries(hex-tests PRIVATE
    Catch2::Catch2WithMain
    hex::hex
    spdlog::spdlog
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(hex-tests)

add_custom_command(
    TARGET hex-tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_SOURCE_DIR}/assets
    ${PROJECT_BINARY_DIR}/assets/
)