cmake_minimum_required(VERSION 3.28)
project(salem)

#add_executable(salem-tests
#)
#
#target_include_directories(salem-tests PRIVATE include/)
#
#target_link_libraries(salem-tests PRIVATE
#    Catch2::Catch2WithMain
#    salem::salem
#    spdlog::spdlog
#)
#
#list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
#include(CTest)
#include(Catch)
#catch_discover_tests(salem-tests)
#
#add_custom_command(
#    TARGET salem-tests POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#    ${PROJECT_SOURCE_DIR}/assets
#    ${PROJECT_BINARY_DIR}/assets/
#)