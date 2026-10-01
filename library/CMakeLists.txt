# =============================================================================
# TPS55289 Buck-Boost Converter Driver - Library Integration CMakeLists.txt
# =============================================================================
# Usage in parent project:
#   add_subdirectory(path/to/TPS55289_STM32_Driver/library)
#   target_link_libraries(your_target PRIVATE tps55289)
# =============================================================================

cmake_minimum_required(VERSION 3.15)

# =============================================================================
# Library Definition
# =============================================================================

add_library(tps55289 STATIC)

# Add source files
target_sources(tps55289
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../Core/Src/tps55289.cpp
)

# Add include directories
# Core/Inc is at ${CMAKE_CURRENT_SOURCE_DIR}/../Core/Inc
target_include_directories(tps55289
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Core/Inc>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../Core/Inc
)

# Dependencies (HAL library must be provided by parent project)
target_link_libraries(tps55289
    PUBLIC
        $<TARGET_NAME_IF_EXISTS:stm32_hal>
)

# =============================================================================
# Requirements
# =============================================================================

set_target_properties(tps55289
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

# =============================================================================
# Compile Definitions
# =============================================================================

target_compile_definitions(tps55289
    PUBLIC
        TPS55289_I2C_ADDRESS=0x74
        TPS55289_TIMEOUT_MS=1000
)
