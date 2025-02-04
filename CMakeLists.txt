cmake_minimum_required(VERSION 3.28)
project(mana)

enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_SCAN_FOR_MODULES FALSE) # prevents gcc expecting cppm files

set(GIT_SHALLOW TRUE)
include(FetchContent)

# spdlog
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git 
  GIT_TAG v1.15.0
)
FetchContent_MakeAvailable(spdlog)

# Catch
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.0
)
FetchContent_MakeAvailable(Catch2)

add_subdirectory(projects/salem)
add_subdirectory(projects/sigil)
