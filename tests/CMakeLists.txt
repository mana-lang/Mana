cmake_minimum_required(VERSION 3.28)
project(mana-tests)

enable_testing()

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

set(TEST_COMMON ${CMAKE_CURRENT_SOURCE_DIR}/common)

## Unit Tests
add_executable(unit-test
        unit/values.cpp
)

target_include_directories(unit-test PUBLIC
        ${TEST_COMMON}
        ${CMAKE_SOURCE_DIR}/hexec/include
)

target_link_libraries(unit-test PUBLIC
        Catch2::Catch2WithMain
        mana::hexec
        spdlog::spdlog
)

catch_discover_tests(unit-test)