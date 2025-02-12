cmake_minimum_required(VERSION 3.28)
project(sigil)

add_executable(sigil-tests
        tests/ptree.cpp
        tests/lexing.cpp
        tests/ast.cpp
        # tests/error_reporting.cpp
)

target_include_directories(sigil-tests PRIVATE include/)

target_link_libraries(sigil-tests PRIVATE
        Catch2::Catch2WithMain
        sigil::sigil
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(sigil-tests)

add_custom_command(
        TARGET sigil-tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/assets
        ${PROJECT_BINARY_DIR}/assets/
)

#add_custom_command(
#        TARGET sigil-tests PRE_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy
#        ${PROJECT_BINARY_DIR}/../libsigil.dll
#        ${PROJECT_BINARY_DIR}/
#)
