cmake_minimum_required(VERSION 3.28)
project(hex VERSION 0.2.8)

enable_testing()

set(HEX_SOURCES
        src/core/logger.cpp
        src/core/disassembly.cpp

        src/hex.cpp
)

set(HEX_INCLUDES
        ${EXT_LIBS_MANA}
        include/
)

set(HEX_LIBS
        mana::hexe
)

add_library(hex_api STATIC ${HEX_SOURCES})
target_include_directories(hex_api PUBLIC ${HEX_INCLUDES} ${EXT_INCLUDES})
target_link_libraries(hex_api PUBLIC ${HEX_LIBS})
add_library(mana::hex ALIAS hex_api)

add_executable(hex
        src/main.cpp
        src/core/cli.cpp
)
target_link_libraries(hex PRIVATE mana::hex)

target_compile_definitions(hex_api PUBLIC $<$<CONFIG:Debug>:HEX_DEBUG>)
target_compile_definitions(hex_api PUBLIC $<$<CONFIG:Release>:HEX_RELEASE>)
target_compile_definitions(hex_api PUBLIC $<$<CONFIG:RelWithDebInfo>:HEX_DEBUG>)

target_compile_definitions(hex_api PUBLIC HEX_NODISCARD=[[nodiscard]])

target_compile_definitions(hex_api PUBLIC HEX_VER_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(hex_api PUBLIC HEX_VER_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(hex_api PUBLIC HEX_VER_PATCH=${PROJECT_VERSION_PATCH})

target_compile_definitions(hex_api PUBLIC HEX_VER_STRING="${PROJECT_VERSION}-${MANA_BUILD_REV}")