cmake_minimum_required(VERSION 3.28)
project(hexe VERSION 0.2.17)

enable_testing()

set(HEXE_SOURCES
        src/logger.cpp

        src/hexe/bytecode.cpp
        src/hexe/value.cpp
        src/hexe/logger.cpp
)

set(HEXE_INCLUDES
        ${EXT_LIBS_MANA}
        include/
)

set(HEXE_LIBS
        spdlog::spdlog
)

add_library(hexe STATIC ${HEXE_SOURCES})

target_include_directories(hexe PUBLIC ${HEXE_INCLUDES})
target_link_libraries(hexe PUBLIC ${HEXE_LIBS})
add_library(mana::hexe ALIAS hexe)

target_compile_definitions(hexe PUBLIC $<$<CONFIG:Debug>:HEXE_DEBUG>)
target_compile_definitions(hexe PUBLIC $<$<CONFIG:Release>:HEXE_RELEASE>)
target_compile_definitions(hexe PUBLIC $<$<CONFIG:RelWithDebInfo>:HEXE_DEBUG>)

target_compile_definitions(hexe PUBLIC HEXE_NODISCARD=[[nodiscard]])
target_compile_definitions(hexe PUBLIC HEXE_VER_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(hexe PUBLIC HEXE_VER_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(hexe PUBLIC HEXE_VER_PATCH=${PROJECT_VERSION_PATCH})

target_compile_definitions(hexe PUBLIC HEXE_VER_STRING="${PROJECT_VERSION}-${HEXE_BUILD_REV}")
target_compile_definitions(hexe PUBLIC HEXE_NAME="Mana")