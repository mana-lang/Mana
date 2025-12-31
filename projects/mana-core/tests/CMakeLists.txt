cmake_minimum_required(VERSION 3.28)
project(mana-core)

add_executable(mana-core-tests
        tests/values.cpp
)

target_include_directories(mana-core-tests PRIVATE include/)

target_link_libraries(mana-core-tests PRIVATE
        Catch2::Catch2WithMain
        mana::core
        spdlog::spdlog
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(mana-core-tests)

add_custom_command(
        TARGET mana-core-tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/assets
        ${PROJECT_BINARY_DIR}/assets/
)