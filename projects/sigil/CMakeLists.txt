cmake_minimum_required(VERSION 3.28)
project(sigil VERSION 0.1.0)

enable_testing()

set(SIGIL_SOURCES
        src/sigil/ast/lexer.cpp
        src/sigil/ast/parser.cpp
        src/sigil/ast/parse-tree.cpp
        src/sigil/ast/nodes.cpp
)

set(SIGIL_INCLUDES
        include/
)

set(SIGIL_LIBS
        spdlog::spdlog
)

set(EXT_INCLUDES
        ../.ext/include/
)

set(EXT_SOURCES

)

add_library(sigil STATIC ${SIGIL_SOURCES})

target_include_directories(sigil PUBLIC
        ${SIGIL_INCLUDES}
        ${EXT_INCLUDES}
)
target_link_libraries(sigil PUBLIC ${SIGIL_LIBS})
add_library(sigil::sigil ALIAS sigil)

target_compile_definitions(sigil PUBLIC $<$<CONFIG:Debug>:SIGIL_DEBUG>)
target_compile_definitions(sigil PUBLIC $<$<CONFIG:Release>:SIGIL_RELEASE>)
target_compile_definitions(sigil PUBLIC $<$<CONFIG:RelWithDebInfo>:SIGIL_DEBUG>)

target_compile_definitions(sigil PUBLIC SIGIL_NODISCARD=[[nodiscard]])
target_compile_definitions(sigil PUBLIC SIGIL_VER_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(sigil PUBLIC SIGIL_VER_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(sigil PUBLIC SIGIL_VER_PATCH=${PROJECT_VERSION_PATCH})

target_compile_definitions(sigil PUBLIC SIGIL_VER_STRING="${PROJECT_VERSION}")

add_subdirectory(tests)