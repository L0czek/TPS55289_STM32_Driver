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

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(tps55289
    LANGUAGES CXX C
    VERSION 1.0.0
    DESCRIPTION "TPS55289 Buck-Boost Converter Driver for STM32 HAL"
)

# =============================================================================
# Configuration Options
# =============================================================================

option(TPS55289_ENABLE_LOGGING "Enable verbose logging" OFF)
option(TPS55289_ENABLE_ASSERTS "Enable debug assertions" OFF)
option(TPS55289_BUILD_TESTS "Build STM32 test application" OFF)
option(TPS55289_BUILD_UNIT_TESTS "Build unit tests with mock HAL" ON)
option(TPS55289_INSTALL "Install TPS55289 driver" OFF)

# Integrated mode: Use project's CMSIS and HAL paths
option(TPS55289_INTEGRATED_MODE "Use project-provided CMSIS/HAL paths" OFF)

# Integrated mode settings (provided by parent project)
set(TPS55289_CMSIS_INC_DIR "" CACHE STRING "Path to CMSIS include directory")
set(TPS55289_HAL_INC_DIR "" CACHE PATH "Path to HAL driver include directory")
set(TPS55289_MCU "" CACHE STRING "MCU name (e.g., STM32G474xx)")

# Standalone mode settings
set(STM32_DEVICE STM32G474 CACHE STRING "STM32 device")
set(STM32_CUBE_PATH "" CACHE PATH "Path to STM32CubeG4 Drivers directory")

# =============================================================================
# Find STM32 HAL
# =============================================================================

if(TPS55289_INTEGRATED_MODE)
    if(NOT TPS55289_CMSIS_INC_DIR OR NOT TPS55289_HAL_INC_DIR OR NOT TPS55289_MCU)
        message(FATAL_ERROR "TPS55289_INTEGRATED_MODE requires TPS55289_CMSIS_INC_DIR, TPS55289_HAL_INC_DIR, and TPS55289_MCU to be set")
    endif()

    message(STATUS "TPS55289: Building in integrated mode with project-provided headers")
    # Only create stm32_hal if it doesn't already exist (from another driver or parent project)
    if(NOT TARGET stm32_hal)
        add_library(stm32_hal INTERFACE)
        target_include_directories(stm32_hal INTERFACE ${TPS55289_CMSIS_INC_DIR} ${TPS55289_HAL_INC_DIR} ${TPS55289_BOARD_INC_DIR})
        target_compile_definitions(stm32_hal INTERFACE ${TPS55289_MCU} USE_HAL_DRIVER)
    endif()
else()
    if(NOT STM32_CUBE_PATH)
        message(FATAL_ERROR "STM32_CUBE_PATH must be set. Example: -DSTM32_CUBE_PATH=/path/to/STM32CubeG4/Drivers")
    endif()

    if(NOT EXISTS ${STM32_CUBE_PATH})
        message(FATAL_ERROR "STM32_CUBE_PATH does not exist: ${STM32_CUBE_PATH}")
    endif()

    message(STATUS "TPS55289: Building in standalone mode with STM32Cube path")

    enable_language(C)

    add_library(stm32_hal STATIC)
    target_sources(stm32_hal
        PRIVATE
            ${STM32_CUBE_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.c
            ${STM32_CUBE_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c.c
            ${STM32_CUBE_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.c
            ${STM32_CUBE_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.c
            ${STM32_CUBE_PATH}/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.c
    )

    target_include_directories(stm32_hal
        PUBLIC
            ${STM32_CUBE_PATH}/STM32G4xx_HAL_Driver/Inc
            ${STM32_CUBE_PATH}/CMSIS/Device/ST/STM32G4xx/Include
    )

    target_compile_definitions(stm32_hal
        PUBLIC
            ${STM32_DEVICE}
            USE_HAL_DRIVER
            HSE_VALUE=16000000
            HSI_VALUE=16000000
    )

    target_compile_options(stm32_hal
        PRIVATE
            -mcpu=cortex-m4
            -mthumb
            -mfloat-abi=hard
            -mfpu=fpv4-sp-d16
            -O2
            -Wall
            -Wextra
    )
endif()

# =============================================================================
# Library Definition
# =============================================================================

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/tps55289.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
)

if(TPS55289_INTEGRATED_MODE)
    target_include_directories(${PROJECT_NAME}
        PRIVATE
            ${TPS55289_CMSIS_INC_DIR}
            ${TPS55289_HAL_INC_DIR}
            ${TPS55289_BOARD_INC_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    )
else()
    target_include_directories(${PROJECT_NAME}
        PRIVATE
            ${STM32_CUBE_PATH}/STM32G4xx_HAL_Driver/Inc
            ${STM32_CUBE_PATH}/CMSIS/Include
            ${STM32_CUBE_PATH}/CMSIS/Device/ST/STM32G4xx/Include
    )
endif()

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        stm32_hal
)

# =============================================================================
# Requirements
# =============================================================================

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

# Exception handling required for expected type
target_compile_options(${PROJECT_NAME}
    PRIVATE
        -fexceptions
)

# =============================================================================
# Compile Definitions
# =============================================================================

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
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
    # Since we cannot change compilers after project() is defined,
    # we create a custom target that builds the test with the host compiler directly

    set(TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_tps55289_simple.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/tps55289.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/mock_hal.cpp
    )

    # Determine include paths based on mode
    if(TPS55289_INTEGRATED_MODE)
        set(HAL_INCLUDE_PATHS "")
        foreach(path IN LISTS TPS55289_CMSIS_INC_DIR)
            list(APPEND HAL_INCLUDE_PATHS -I${path})
        endforeach()
        foreach(path IN LISTS TPS55289_HAL_INC_DIR)
            list(APPEND HAL_INCLUDE_PATHS -I${path})
        endforeach()
    else()
        set(HAL_INCLUDE_PATHS
            -I${STM32_CUBE_PATH}/STM32G4xx_HAL_Driver/Inc
            -I${STM32_CUBE_PATH}/CMSIS/Include
            -I${STM32_CUBE_PATH}/CMSIS/Device/ST/STM32G4xx/Include
        )
    endif()

    # Create custom target that runs g++ directly
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_tps55289.elf
        COMMAND /usr/bin/g++ -std=c++17 -fexceptions
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
            -I${CMAKE_CURRENT_SOURCE_DIR}/tests
            ${HAL_INCLUDE_PATHS}
            -D${TPS55289_MCU}
            -DTPS55289_I2C_ADDRESS=0x74
            -DTPS55289_TIMEOUT_MS=1000
            -DTPS55289_UNIT_TEST
            -o ${CMAKE_CURRENT_BINARY_DIR}/test_tps55289.elf
            ${TEST_SOURCES}
        DEPENDS ${TEST_SOURCES}
        COMMENT "Building TPS55289 unit test with host compiler"
    )

    add_custom_target(test_tps55289
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_tps55289.elf
    )

    # Add test
    add_test(NAME test_tps55289 COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_tps55289.elf)

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
message(STATUS "  Device:            ${STM32_DEVICE}")
message(STATUS "  Integrated Mode:   ${TPS55289_INTEGRATED_MODE}")
if(TPS55289_INTEGRATED_MODE)
    message(STATUS "  CMSIS Include:     ${TPS55289_CMSIS_INC_DIR}")
    message(STATUS "  HAL Include:       ${TPS55289_HAL_INC_DIR}")
else()
    message(STATUS "  Cube Path:         ${STM32_CUBE_PATH}")
endif()
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
message(STATUS "To build (integrated mode):")
message(STATUS "  add_subdirectory(TPS55289_STM32_Driver)")
message(STATUS "  target_link_libraries(your_target PRIVATE tps55289)")
message(STATUS "")
message(STATUS "To build (standalone mode):")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake .. -DSTM32_DEVICE=${STM32_DEVICE} -DSTM32_CUBE_PATH=<path-to-STM32CubeG4>")
message(STATUS "  make")
if(TPS55289_BUILD_TESTS)
    message(STATUS "  make tps55289_test - Build STM32 test application")
endif()
if(TPS55289_BUILD_UNIT_TESTS)
    message(STATUS "  make test_tps55289 - Build unit tests")
    message(STATUS "  ctest              - Run unit tests")
endif()
message(STATUS "")
