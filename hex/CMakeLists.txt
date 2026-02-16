cmake_minimum_required(VERSION 3.28)
project(hex VERSION 0.1.17)

enable_testing()

set(HEX_SOURCES
        src/main.cpp

        src/core/cli.cpp
        src/core/logger.cpp
        src/core/disassembly.cpp

        src/hex.cpp
)

set(HEX_INCLUDES
        ../ext/include/
        include/
)

set(HEX_LIBS
        mana::core
)

add_executable(hex)
target_sources(hex PRIVATE ${HEX_SOURCES})
target_include_directories(hex PUBLIC ${HEX_INCLUDES})
target_link_libraries(hex PRIVATE ${HEX_LIBS})

add_library(hex_api SHARED ${HEX_SOURCES})
target_include_directories(hex_api PUBLIC ${HEX_INCLUDES} ${EXT_INCLUDES})
target_link_libraries(hex_api PUBLIC ${HEX_LIBS})
add_library(hex::hex ALIAS hex_api)

target_compile_definitions(hex PRIVATE $<$<CONFIG:Debug>:HEX_DEBUG>)
target_compile_definitions(hex PRIVATE $<$<CONFIG:Release>:HEX_RELEASE>)
target_compile_definitions(hex PRIVATE $<$<CONFIG:RelWithDebInfo>:HEX_DEBUG>)

target_compile_definitions(hex PRIVATE HEX_NODISCARD=[[nodiscard]])
target_compile_definitions(hex_api PUBLIC HEX_NODISCARD=[[nodiscard]])

target_compile_definitions(hex PRIVATE HEX_VER_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(hex PRIVATE HEX_VER_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(hex PRIVATE HEX_VER_PATCH=${PROJECT_VERSION_PATCH})

target_compile_definitions(hex PRIVATE HEX_VER_STRING="${PROJECT_VERSION}-${MANA_BUILD_REV}")

add_custom_command(
        TARGET hex POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${PROJECT_BINARY_DIR}/assets/
)

add_subdirectory(tests)