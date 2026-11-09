cmake_minimum_required(VERSION 3.28)
project(circe VERSION 0.1.33)

enable_testing()

set(CIRCE_SOURCES
        src/bytecode-generator.cpp
        src/register.cpp

        src/core/logger.cpp
)

set(CIRCE_INCLUDES
        ${EXT_LIBS_MANA}
        include/
)

set(CIRCE_LIBS
        mana::sigil
)

add_library(circe_api STATIC ${CIRCE_SOURCES})
target_include_directories(circe_api PUBLIC ${CIRCE_INCLUDES})
target_link_libraries(circe_api PUBLIC ${CIRCE_LIBS})
add_library(mana::circe ALIAS circe_api)

add_executable(circe
        src/main.cpp
        src/core/cli.cpp
)
target_link_libraries(circe PRIVATE mana::circe)

target_compile_definitions(circe_api PUBLIC $<$<CONFIG:Debug>:CIRCE_DEBUG>)
target_compile_definitions(circe_api PUBLIC $<$<CONFIG:Release>:CIRCE_RELEASE>)
target_compile_definitions(circe_api PUBLIC $<$<CONFIG:RelWithDebInfo>:CIRCE_DEBUG>)

target_compile_definitions(circe_api PUBLIC CIRCE_NODISCARD=[[nodiscard]])
target_compile_definitions(circe_api PUBLIC CIRCE_VER_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(circe_api PUBLIC CIRCE_VER_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(circe_api PUBLIC CIRCE_VER_PATCH=${PROJECT_VERSION_PATCH})

target_compile_definitions(circe_api PUBLIC CIRCE_VER_STRING="${PROJECT_VERSION}-${MANA_BUILD_REV}")

copy_post_build(circe)