/**
 * @file test_main.cpp
 * @brief Simple test to verify the TPS55289 driver compiles correctly
 *
 * This file is not meant to be a comprehensive test, just a compilation
 * verification that all headers and implementations work together.
 */

#include "tps55289.hpp"
#include <iostream>

// Mock I2C handle - provided by mock_hal.c
extern "C" {
    I2C_HandleTypeDef hi2c1;
}

int main() {
    // Create TPS55289 instance
    tps55289::TPS55289 tps55289(&hi2c1);

    // Test static methods
    std::cout << "VREF Min: " << tps55289::TPS55289::get_vref_min_mv() << " mV" << std::endl;
    std::cout << "VREF LSB: " << tps55289::TPS55289::get_vref_lsb_mv() << " mV" << std::endl;
    std::cout << "VREF Max Code: " << tps55289::TPS55289::get_vref_max_code() << std::endl;

    // Test method calls (these will fail at runtime without real hardware,
    // but we're just verifying compilation)
    auto result = tps55289.verify_device_present();
    if (!result) {
        std::cerr << "Device verification failed: " << result.error().message() << std::endl;
    }

    // Test error codes
    auto ec = tps55289::make_error_code(tps55289::ErrorCode::HALError);
    std::cout << "Error category: " << ec.category().name() << std::endl;
    std::cout << "Error message: " << ec.message() << std::endl;

    // Test voltage conversion
    uint16_t code = tps55289.vref_mv_to_code(500.0f);
    std::cout << "500mV REF code: 0x" << std::hex << code << std::dec << std::endl;

    float vref = tps55289.ref_code_to_vref_mv(code);
    std::cout << "REF code " << code << " -> " << vref << " mV" << std::endl;

    return 0;
}
