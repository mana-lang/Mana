cmake_minimum_required(VERSION 3.28)
project(hex VERSION 0.0.1)

enable_testing()

set(HEX_SOURCES
    src/hex/core/cli.cpp
)

set(HEX_INCLUDES
    include/
)

set(HEX_LIBS
    spdlog::spdlog
)

set(EXT_INCLUDES
    ext/include/
)

set(EXT_SOURCES

)

add_library(hex_api STATIC ${HEX_SOURCES})
target_include_directories(hex_api PUBLIC ${HEX_INCLUDES})
target_link_libraries(hex_api PUBLIC ${HEX_LIBS})
add_library(hex::hex ALIAS hex_api)


add_compile_definitions($<$<CONFIG:Debug>:HEX_DEBUG>)
add_compile_definitions($<$<CONFIG:Release>:HEX_RELEASE>)
add_compile_definitions($<$<CONFIG:RelWithDebInfo>:HEX_DEBUG>)

add_compile_definitions(HEX_NODISCARD=[[nodiscard]])
add_compile_definitions(HEX_VER_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR})
add_compile_definitions(HEX_VER_MINOR=${CMAKE_PROJECT_VERSION_MINOR})
add_compile_definitions(HEX_VER_PATCH=${CMAKE_PROJECT_VERSION_PATCH})

add_compile_definitions(HEX_VER_STRING="${CMAKE_PROJECT_VERSION}")