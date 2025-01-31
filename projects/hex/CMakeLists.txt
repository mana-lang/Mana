cmake_minimum_required(VERSION 3.28)
project(hex VERSION 0.0.1)

enable_testing()

set(HEX_SOURCES
        src/hex/ast/lexer.cpp
        src/hex/ast/parser.cpp
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

add_library(hex STATIC ${HEX_SOURCES})

target_include_directories(hex PUBLIC
        ${HEX_INCLUDES}
        ${EXT_INCLUDES}
)
target_link_libraries(hex PUBLIC ${HEX_LIBS})
add_library(hex::hex ALIAS hex)

target_compile_definitions(hex PUBLIC $<$<CONFIG:Debug>:HEX_DEBUG>)
target_compile_definitions(hex PUBLIC $<$<CONFIG:Release>:HEX_RELEASE>)
target_compile_definitions(hex PUBLIC $<$<CONFIG:RelWithDebInfo>:HEX_DEBUG>)

target_compile_definitions(hex PUBLIC HEX_NODISCARD=[[nodiscard]])
target_compile_definitions(hex PUBLIC HEX_VER_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(hex PUBLIC HEX_VER_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(hex PUBLIC HEX_VER_PATCH=${PROJECT_VERSION_PATCH})

target_compile_definitions(hex PUBLIC HEX_VER_STRING="${PROJECT_VERSION}")

add_subdirectory(tests)