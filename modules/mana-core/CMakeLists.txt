cmake_minimum_required(VERSION 3.28)
project(mana-core VERSION 0.0.1)

enable_testing()

set(MANA_SOURCES
        src/logger.cpp

        src/vm/hexe.cpp
        src/vm/value.cpp
)

set(MANA_INCLUDES
        include/
)

set(MANA_LIBS
        spdlog::spdlog
)

set(EXT_INCLUDES
        ../.ext/include/
)

add_library(mana-core STATIC ${MANA_SOURCES})

target_include_directories(mana-core PUBLIC
        ${MANA_INCLUDES}
        ${EXT_INCLUDES}
)
target_link_libraries(mana-core PUBLIC ${MANA_LIBS})
add_library(mana::core ALIAS mana-core)

target_compile_definitions(mana-core PUBLIC $<$<CONFIG:Debug>:MANA_DEBUG>)
target_compile_definitions(mana-core PUBLIC $<$<CONFIG:Release>:MANA_RELEASE>)
target_compile_definitions(mana-core PUBLIC $<$<CONFIG:RelWithDebInfo>:MANA_DEBUG>)

target_compile_definitions(mana-core PUBLIC MANA_NODISCARD=[[nodiscard]])
target_compile_definitions(mana-core PUBLIC MANA_VER_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(mana-core PUBLIC MANA_VER_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(mana-core PUBLIC MANA_VER_PATCH=${PROJECT_VERSION_PATCH})

target_compile_definitions(mana-core PUBLIC MANA_VER_STRING="${PROJECT_VERSION}")
target_compile_definitions(mana-core PUBLIC MANA_NAME="Mana")

add_subdirectory(tests)