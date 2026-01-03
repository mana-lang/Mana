cmake_minimum_required(VERSION 3.28)
project(circe)

#add_executable(circe-tests
#)
#
#target_include_directories(circe-tests PRIVATE include/)
#
#target_link_libraries(circe-tests PRIVATE
#        Catch2::Catch2WithMain
#        circe::circe
#        spdlog::spdlog
#)
#
#list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
#include(CTest)
#include(Catch)
#catch_discover_tests(circe-tests)
#
#add_custom_command(
#        TARGET circe-tests POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        ${PROJECT_SOURCE_DIR}/assets
#        ${PROJECT_BINARY_DIR}/assets/
#)