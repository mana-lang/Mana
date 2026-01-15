cmake_minimum_required(VERSION 3.28)
project(circe VERSION 0.1.0)

enable_testing()

set(CIRCE_SOURCES
        src/main.cpp
        src/bytecode-generator.cpp

        src/core/logger.cpp
        src/core/cli.cpp
)

set(CIRCE_INCLUDES
        include/
)

set(CIRCE_LIBS
        sigil::sigil
)

set(EXT_INCLUDES
        ../.ext/include/
)

set(EXT_SOURCES

)

add_executable(circe)
target_sources(circe PRIVATE ${CIRCE_SOURCES})
target_include_directories(circe PRIVATE ${CIRCE_INCLUDES})
target_link_libraries(circe PRIVATE ${CIRCE_LIBS})

add_library(circe_api STATIC ${CIRCE_SOURCES})
target_include_directories(circe_api PUBLIC ${CIRCE_INCLUDES})
target_link_libraries(circe_api PUBLIC ${CIRCE_LIBS})
add_library(circe::circe ALIAS circe_api)


target_compile_definitions(circe PRIVATE $<$<CONFIG:Debug>:CIRCE_DEBUG>)
target_compile_definitions(circe PRIVATE $<$<CONFIG:Release>:CIRCE_RELEASE>)
target_compile_definitions(circe PRIVATE $<$<CONFIG:RelWithDebInfo>:CIRCE_DEBUG>)

target_compile_definitions(circe PRIVATE CIRCE_NODISCARD=[[nodiscard]])
target_compile_definitions(circe PRIVATE CIRCE_VER_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(circe PRIVATE CIRCE_VER_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(circe PRIVATE CIRCE_VER_PATCH=${PROJECT_VERSION_PATCH})

target_compile_definitions(circe PRIVATE CIRCE_VER_STRING="${PROJECT_VERSION}")

add_custom_command(
        TARGET circe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${PROJECT_BINARY_DIR}/assets/
)

add_subdirectory(tests)