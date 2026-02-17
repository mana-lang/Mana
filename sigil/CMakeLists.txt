cmake_minimum_required(VERSION 3.28)
project(sigil VERSION 0.2.4)

enable_testing()

set(SIGIL_SOURCES
        src/ast/lexer.cpp
        src/ast/parser.cpp
        src/ast/parse-tree.cpp
        src/ast/syntax-tree.cpp
        src/ast/source-file.cpp
        src/ast/semantic-analyzer.cpp

        src/core/logger.cpp

        src/error/error.cpp
        src/error/error-sink.cpp
)

set(SIGIL_INCLUDES
        ${EXT_LIBS_MANA}
        include/
)

set(SIGIL_LIBS
        mana::hexec
)

add_library(sigil STATIC ${SIGIL_SOURCES})

target_include_directories(sigil PUBLIC ${SIGIL_INCLUDES})
target_link_libraries(sigil PUBLIC ${SIGIL_LIBS})
add_library(mana::sigil ALIAS sigil)

target_compile_definitions(sigil PUBLIC $<$<CONFIG:Debug>:SIGIL_DEBUG>)
target_compile_definitions(sigil PUBLIC $<$<CONFIG:Release>:SIGIL_RELEASE>)
target_compile_definitions(sigil PUBLIC $<$<CONFIG:RelWithDebInfo>:SIGIL_DEBUG>)

target_compile_definitions(sigil PUBLIC SIGIL_NODISCARD=[[nodiscard]])
target_compile_definitions(sigil PUBLIC SIGIL_VER_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(sigil PUBLIC SIGIL_VER_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(sigil PUBLIC SIGIL_VER_PATCH=${PROJECT_VERSION_PATCH})

target_compile_definitions(sigil PUBLIC SIGIL_VER_STRING="${PROJECT_VERSION}-${MANA_BUILD_REV}")