cmake_minimum_required(VERSION 3.28)
project(salem)

enable_testing()

set(SALEM_SOURCES
    src/salem/main.cpp
)

set(SALEM_INCLUDES
    include/
)

set(SALEM_LIBS
    hex::hex
)

set(EXT_INCLUDES
    ext/include/
)

set(EXT_SOURCES

)

add_executable(salem)
target_sources(salem PRIVATE ${SALEM_SOURCES})
target_include_directories(salem PRIVATE ${SALEM_INCLUDES})
target_link_libraries(salem PRIVATE ${SALEM_LIBS})

add_library(salem_api STATIC ${SALEM_SOURCES})
target_include_directories(salem_api PUBLIC ${SALEM_INCLUDES})
target_link_libraries(salem_api PUBLIC ${SALEM_LIBS})
add_library(salem::salem ALIAS salem_api)


add_compile_definitions($<$<CONFIG:Debug>:SALEM_DEBUG>)
add_compile_definitions($<$<CONFIG:Release>:SALEM_RELEASE>)
add_compile_definitions($<$<CONFIG:RelWithDebInfo>:SALEM_DEBUG>)

add_compile_definitions(SALEM_NODISCARD=[[nodiscard]])
add_compile_definitions(SALEM_VER_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR})
add_compile_definitions(SALEM_VER_MINOR=${CMAKE_PROJECT_VERSION_MINOR})
add_compile_definitions(SALEM_VER_PATCH=${CMAKE_PROJECT_VERSION_PATCH})

add_compile_definitions(SALEM_VER_STRING="${CMAKE_PROJECT_VERSION}")

add_custom_command(
    TARGET salem POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_SOURCE_DIR}/assets
    ${PROJECT_BINARY_DIR}/assets/
)

add_subdirectory(tests)