cmake_minimum_required(VERSION 3.28)
project(mana)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_SCAN_FOR_MODULES FALSE) # prevents gcc expecting cppm files

add_subdirectory(projects/salem)

enable_testing()
add_subdirectory(projects/tests)
