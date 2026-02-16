cmake_minimum_required(VERSION 3.28)
project(salem VERSION 0.0.1)

enable_testing()

set(SALEM_SOURCES
        src/salem/main.cpp
)

set(SALEM_INCLUDES
        ${EXT_LIBS_MANA}
        include/
)

set(SALEM_LIBS
        sigil::sigil
)

add_executable(salem)
target_sources(salem PRIVATE ${SALEM_SOURCES})
target_include_directories(salem PRIVATE ${SALEM_INCLUDES})
target_link_libraries(salem PRIVATE ${SALEM_LIBS})

add_library(salem_api STATIC ${SALEM_SOURCES})
target_include_directories(salem_api PUBLIC ${SALEM_INCLUDES})
target_link_libraries(salem_api PUBLIC ${SALEM_LIBS})
add_library(salem::salem ALIAS salem_api)


target_compile_definitions(salem PRIVATE $<$<CONFIG:Debug>:SALEM_DEBUG>)
target_compile_definitions(salem PRIVATE $<$<CONFIG:Release>:SALEM_RELEASE>)
target_compile_definitions(salem PRIVATE $<$<CONFIG:RelWithDebInfo>:SALEM_DEBUG>)

target_compile_definitions(salem PRIVATE SALEM_NODISCARD=[[nodiscard]])
target_compile_definitions(salem PRIVATE SALEM_VER_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(salem PRIVATE SALEM_VER_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(salem PRIVATE SALEM_VER_PATCH=${PROJECT_VERSION_PATCH})

target_compile_definitions(salem PRIVATE SALEM_VER_STRING="${PROJECT_VERSION}")

add_custom_command(
        TARGET salem POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${PROJECT_BINARY_DIR}/assets/
)

add_subdirectory(tests)