cmake_minimum_required(VERSION 3.28)
project(hexec VERSION 0.2.17)

enable_testing()

set(HEXEC_SOURCES
        src/logger.cpp

        src/hexec/bytecode.cpp
        src/hexec/value.cpp
        src/hexec/logger.cpp
)

set(HEXEC_INCLUDES
        ${EXT_LIBS_MANA}
        include/
)

set(HEXEC_LIBS
        spdlog::spdlog
)

add_library(hexec STATIC ${HEXEC_SOURCES})

target_include_directories(hexec PUBLIC ${HEXEC_INCLUDES})
target_link_libraries(hexec PUBLIC ${HEXEC_LIBS})
add_library(mana::hexec ALIAS hexec)

target_compile_definitions(hexec PUBLIC $<$<CONFIG:Debug>:HEXEC_DEBUG>)
target_compile_definitions(hexec PUBLIC $<$<CONFIG:Release>:HEXEC_RELEASE>)
target_compile_definitions(hexec PUBLIC $<$<CONFIG:RelWithDebInfo>:HEXEC_DEBUG>)

target_compile_definitions(hexec PUBLIC HEXEC_NODISCARD=[[nodiscard]])
target_compile_definitions(hexec PUBLIC HEXEC_VER_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(hexec PUBLIC HEXEC_VER_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(hexec PUBLIC HEXEC_VER_PATCH=${PROJECT_VERSION_PATCH})

target_compile_definitions(hexec PUBLIC HEXEC_VER_STRING="${PROJECT_VERSION}-${HEXEC_BUILD_REV}")
target_compile_definitions(hexec PUBLIC HEXEC_NAME="Mana")