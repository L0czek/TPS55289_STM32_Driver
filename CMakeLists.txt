# =============================================================================
# TPS55289 Buck-Boost Converter Driver CMake Library
# =============================================================================
# This library provides an I2C driver for the TI TPS55289 buck-boost converter
# for use with STM32 microcontrollers using the HAL driver.
#
# Usage in parent project:
#   add_subdirectory(TPS55289_STM32_Driver)
#   target_link_libraries(your_target PRIVATE tps55289)
# =============================================================================

cmake_minimum_required(VERSION 3.15)

# =============================================================================
# Configuration
# =============================================================================

# Generate compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(tps55289_driver
    LANGUAGES CXX C
    VERSION 1.0.0
    DESCRIPTION "TPS55289 Buck-Boost Converter Driver for STM32 HAL"
)

# =============================================================================
# Configuration Options
# =============================================================================

# Optional features
option(TPS55289_ENABLE_LOGGING "Enable verbose logging" OFF)
option(TPS55289_ENABLE_ASSERTS "Enable debug assertions" OFF)
option(TPS55289_BUILD_TESTS "Build STM32 test application" OFF)
option(TPS55289_BUILD_UNIT_TESTS "Build unit tests with mock HAL" ON)
option(TPS55289_INSTALL "Install TPS55289 driver" OFF)

# =============================================================================
# STM32 HAL Library (mock for unit testing)
# =============================================================================

add_library(stm32_hal STATIC)
target_sources(stm32_hal
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/mock_hal.cpp
)

target_include_directories(stm32_hal
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
        ${CMAKE_CURRENT_SOURCE_DIR}/library
)

# Always compile for unit test mode
target_compile_definitions(stm32_hal
    PUBLIC
        TPS55289_UNIT_TEST=1
)

# =============================================================================
# Library Definition
# =============================================================================

add_library(${PROJECT_NAME} STATIC)

# Add source files
target_sources(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/tps55289.cpp
)

# Add include directories
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
)

# Dependencies
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        stm32_hal
)

# =============================================================================
# Requirements
# =============================================================================

# Require C++17 standard (for std::expected)
set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

# =============================================================================
# Compile Definitions
# =============================================================================

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        # TPS55289 configuration
        TPS55289_I2C_ADDRESS=0x74
        TPS55289_TIMEOUT_MS=1000
)

# =============================================================================
# Optional Features
# =============================================================================

if(TPS55289_ENABLE_LOGGING)
    target_compile_definitions(${PROJECT_NAME}
        PUBLIC
            TPS55289_ENABLE_LOGGING=1
    )
endif()

if(TPS55289_ENABLE_ASSERTS)
    target_compile_definitions(${PROJECT_NAME}
        PUBLIC
            TPS55289_ENABLE_ASSERTS=1
    )
endif()

# =============================================================================
# Test Application (STM32 - requires STM32CubeG4)
# =============================================================================

if(TPS55289_BUILD_TESTS)
    message(STATUS "STM32 test application requires STM32CubeG4 path")
    message(STATUS "Enable with: -DTPS55289_BUILD_TESTS=ON -DSTM32_CUBE_PATH=/path/to/STM32CubeG4/Drivers")
endif()

# =============================================================================
# Unit Tests with Mocks (Host-based testing)
# =============================================================================

if(TPS55289_BUILD_UNIT_TESTS)
    enable_testing()

    # Create test executable (using simple test framework, no external dependencies)
    add_executable(test_tps55289
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_tps55289_simple.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/tps55289.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/mock_hal.cpp
    )

    target_include_directories(test_tps55289
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    target_compile_definitions(test_tps55289
        PUBLIC
            TPS55289_I2C_ADDRESS=0x74
            TPS55289_TIMEOUT_MS=1000
    )

    # Use host C++17 compiler
    set_target_properties(test_tps55289
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )

    # Link with standard C++ library
    target_link_libraries(test_tps55289
        PRIVATE
            stdc++
            stm32_hal
    )

    # Add test (use full path to ensure CTest can find it)
    add_test(NAME test_tps55289 COMMAND $<TARGET_FILE:test_tps55289>)

    message(STATUS "Unit tests enabled - build with 'make test_tps55289'")
endif()

# =============================================================================
# Installation
# =============================================================================

if(TPS55289_INSTALL)
    include(GNUInstallDirs)

    install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(EXPORT ${PROJECT_NAME}-targets
        FILE
            ${PROJECT_NAME}-targets.cmake
        NAMESPACE
            tps55289::
        DESTINATION
            lib/cmake/tps55289
    )

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
        DESTINATION include
    )

    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/README.md
        DESTINATION share/doc/tps55289
    )
endif()

# =============================================================================
# Summary
# =============================================================================

message(STATUS "")
message(STATUS "TPS55289 Buck-Boost Converter Driver Configuration:")
message(STATUS "  Version:           ${PROJECT_VERSION}")
message(STATUS "  Logging:           ${TPS55289_ENABLE_LOGGING}")
message(STATUS "  Assertions:        ${TPS55289_ENABLE_ASSERTS}")
message(STATUS "  Build Tests:       ${TPS55289_BUILD_TESTS}")
message(STATUS "  Build Unit Tests:  ${TPS55289_BUILD_UNIT_TESTS}")
message(STATUS "  Install:           ${TPS55289_INSTALL}")
message(STATUS "")
message(STATUS "Available targets:")
message(STATUS "  tps55289          - Build TPS55289 library")
if(TPS55289_BUILD_TESTS)
    message(STATUS "  tps55289_test     - Build test application (requires STM32CubeG4)")
endif()
if(TPS55289_BUILD_UNIT_TESTS)
    message(STATUS "  test_tps55289     - Build unit tests (host)")
endif()
message(STATUS "")
message(STATUS "To build:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  make")
if(TPS55289_BUILD_TESTS)
    message(STATUS "  make tps55289_test - Build STM32 test application")
endif()
if(TPS55289_BUILD_UNIT_TESTS)
    message(STATUS "  make test_tps55289 - Build unit tests")
    message(STATUS "  ctest              - Run unit tests")
endif()
message(STATUS "")
