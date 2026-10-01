/**
 * @file test_tps55289_simple.cpp
 * @brief Simple unit tests for the TPS55289 driver (no external dependencies)
 *
 * This file contains unit tests for the TPS55289 driver without using Google Test.
 * It uses simple assertion macros for testing.
 */

#include "tps55289.hpp"
#include <cmath>
#include <iostream>
#include <string>

// Simple assertion macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "FAIL: " << message << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1; \
        } else { \
            std::cout << "PASS: " << message << std::endl; \
        } \
    } while(0)

#define TEST_ASSERT_FLOAT_EQ(expected, actual, tolerance) \
    do { \
        float diff = std::abs((expected) - (actual)); \
        if (diff > (tolerance)) { \
            std::cerr << "FAIL: " << #expected " == " #actual " (expected=" << (expected) \
                      << ", actual=" << (actual) << ", diff=" << diff << ") at " \
                      << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1; \
        } else { \
            std::cout << "PASS: " << #expected " == " #actual " (within " << tolerance << ")" << std::endl; \
        } \
    } while(0)

#define TEST_ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            std::cerr << "FAIL: " << #expected " == " #actual " (expected=" << (expected) \
                      << ", actual=" << (actual) << ") at " \
                      << __FILE__ << ":" << __LINE__ << std::endl; \
            return 1; \
        } else { \
            std::cout << "PASS: " << #expected " == " #actual << std::endl; \
        } \
    } while(0)

#define TEST_ASSERT_TRUE(condition) TEST_ASSERT((condition), #condition)
#define TEST_ASSERT_FALSE(condition) TEST_ASSERT(!(condition), "!(" #condition ")")

// Test static configuration values
int test_static_values() {
    std::cout << "\n=== Test: Static Values ===" << std::endl;
    
    TEST_ASSERT_FLOAT_EQ(45.0f, tps55289::TPS55289::get_vref_min_mv(), 0.001f);
    TEST_ASSERT_FLOAT_EQ(0.5645f, tps55289::TPS55289::get_vref_lsb_mv(), 0.0001f);
    TEST_ASSERT_EQ(0x7FE, tps55289::TPS55289::get_vref_max_code());
    
    return 0;
}

// Test VREF code conversion
int test_vref_conversion() {
    std::cout << "\n=== Test: VREF Conversion ===" << std::endl;
    
    I2C_HandleTypeDef hi2c;
    tps55289::TPS55289 tps55289(&hi2c);
    
    // Test vref_mv_to_code
    // Formula: (vref_mv - 45.0f) / 0.5645f
    // For 500mV: (500 - 45) / 0.5645 = 806
    uint16_t code = tps55289.vref_mv_to_code(500.0f);
    TEST_ASSERT_FLOAT_EQ(806.0f, static_cast<float>(code), 2.0f);

    // Test ref_code_to_vref_mv
    float vref = tps55289.ref_code_to_vref_mv(code);
    TEST_ASSERT_FLOAT_EQ(500.0f, vref, 0.6f);
    
    // Test boundary conditions
    TEST_ASSERT_EQ(0, tps55289.vref_mv_to_code(0.0f));  // Below minimum
    TEST_ASSERT_EQ(0x7FE, tps55289.vref_mv_to_code(2000.0f));  // Above maximum
    
    return 0;
}

// Test error code conversion
int test_error_codes() {
    std::cout << "\n=== Test: Error Codes ===" << std::endl;
    
    auto ec = tps55289::make_error_code(tps55289::ErrorCode::HALError);
    TEST_ASSERT_EQ(std::string("tps55289"), std::string(ec.category().name()));
    TEST_ASSERT_EQ(std::string("HAL error"), std::string(ec.message()));
    
    ec = tps55289::make_error_code(tps55289::ErrorCode::Timeout);
    TEST_ASSERT_EQ(std::string("Operation timeout"), std::string(ec.message()));
    
    ec = tps55289::make_error_code(tps55289::ErrorCode::None);
    TEST_ASSERT_EQ(std::string("No error"), std::string(ec.message()));
    
    return 0;
}

// Test register write
int test_write_register() {
    std::cout << "\n=== Test: Write Register ===" << std::endl;
    
    I2C_HandleTypeDef hi2c;
    tps55289::TPS55289 tps55289(&hi2c);
    
    auto result = tps55289.write_register(0x00, 0x12);
    TEST_ASSERT_TRUE(result.has_value());
    
    return 0;
}

// Test register read
int test_read_register() {
    std::cout << "\n=== Test: Read Register ===" << std::endl;
    
    I2C_HandleTypeDef hi2c;
    tps55289::TPS55289 tps55289(&hi2c);
    
    auto result = tps55289.read_register(0x00);
    TEST_ASSERT_TRUE(result.has_value());
    // After write_register, the mock register should contain the written value
    // For read_register, we're reading from a new address (0x00) which was never written
    // The mock returns the first byte of mock_registers[0] for all reads due to memset
    // This is expected behavior - the mock register map starts at 0
    
    return 0;
}

// Test current limit
int test_current_limit() {
    std::cout << "\n=== Test: Current Limit ===" << std::endl;
    
    I2C_HandleTypeDef hi2c;
    tps55289::TPS55289 tps55289(&hi2c);
    
    auto result = tps55289.set_current_limit_voltage_mv(50.0f);
    TEST_ASSERT_TRUE(result.has_value());
    
    auto current = tps55289.get_current_limit_voltage_mv();
    TEST_ASSERT_TRUE(current.has_value());
    TEST_ASSERT_FLOAT_EQ(50.0f, current.value(), 0.6f);
    
    return 0;
}

// Test output voltage
int test_output_voltage() {
    std::cout << "\n=== Test: Output Voltage ===" << std::endl;
    
    I2C_HandleTypeDef hi2c;
    tps55289::TPS55289 tps55289(&hi2c);
    
    // Test with internal feedback (0.1128 ratio)
    auto result = tps55289.set_output_voltage(5.0f);
    TEST_ASSERT_TRUE(result.has_value());
    
    auto voltage = tps55289.get_output_voltage();
    TEST_ASSERT_TRUE(voltage.has_value());
    TEST_ASSERT_FLOAT_EQ(5.0f, voltage.value(), 0.1f);
    
    return 0;
}

// Test feedback mode
int test_feedback_mode() {
    std::cout << "\n=== Test: Feedback Mode ===" << std::endl;
    
    I2C_HandleTypeDef hi2c;
    tps55289::TPS55289 tps55289(&hi2c);
    
    auto result = tps55289.set_feedback_mode(tps55289::FeedbackMode::External);
    TEST_ASSERT_TRUE(result.has_value());
    
    auto mode = tps55289.get_feedback_mode();
    TEST_ASSERT_EQ(static_cast<int>(tps55289::FeedbackMode::External), static_cast<int>(mode));
    
    return 0;
}

// Test slew rate
int test_slew_rate() {
    std::cout << "\n=== Test: Slew Rate ===" << std::endl;
    
    I2C_HandleTypeDef hi2c;
    tps55289::TPS55289 tps55289(&hi2c);
    
    auto result = tps55289.set_slew_rate_mv_us(5.0f);
    TEST_ASSERT_TRUE(result.has_value());
    
    auto rate = tps55289.get_slew_rate_mv_us();
    TEST_ASSERT_TRUE(rate.has_value());
    TEST_ASSERT_FLOAT_EQ(5.0f, rate.value(), 0.1f);
    
    return 0;
}

// Test OCP delay
int test_ocp_delay() {
    std::cout << "\n=== Test: OCP Delay ===" << std::endl;
    
    I2C_HandleTypeDef hi2c;
    tps55289::TPS55289 tps55289(&hi2c);
    
    auto result = tps55289.set_ocp_delay_us(3072);
    TEST_ASSERT_TRUE(result.has_value());
    
    auto delay = tps55289.get_ocp_delay_us();
    TEST_ASSERT_TRUE(delay.has_value());
    TEST_ASSERT_EQ(3072, delay.value());
    
    return 0;
}

// Test CDC compensation
int test_cdc_compensation() {
    std::cout << "\n=== Test: CDC Compensation ===" << std::endl;
    
    I2C_HandleTypeDef hi2c;
    tps55289::TPS55289 tps55289(&hi2c);
    
    auto result = tps55289.set_cdc_compensation_v(0.35f);
    TEST_ASSERT_TRUE(result.has_value());
    
    auto cdc = tps55289.get_cdc_compensation_v();
    TEST_ASSERT_TRUE(cdc.has_value());
    TEST_ASSERT_FLOAT_EQ(0.35f, cdc.value(), 0.1f);
    
    return 0;
}

// Test fault indication
int test_fault_indication() {
    std::cout << "\n=== Test: Fault Indication ===" << std::endl;
    
    I2C_HandleTypeDef hi2c;
    tps55289::TPS55289 tps55289(&hi2c);
    
    auto result = tps55289.enable_fault_indication("ocp");
    TEST_ASSERT_TRUE(result.has_value());
    
    result = tps55289.disable_fault_indication("ocp");
    TEST_ASSERT_TRUE(result.has_value());
    
    return 0;
}

// Test mode configuration
int test_mode_configuration() {
    std::cout << "\n=== Test: Mode Configuration ===" << std::endl;
    
    I2C_HandleTypeDef hi2c;
    tps55289::TPS55289 tps55289(&hi2c);
    
    auto result = tps55289.set_fsw_doubling(true);
    TEST_ASSERT_TRUE(result.has_value());
    
    result = tps55289.set_hiccup_mode(true);
    TEST_ASSERT_TRUE(result.has_value());
    
    result = tps55289.set_vout_discharge(true);
    TEST_ASSERT_TRUE(result.has_value());
    
    result = tps55289.set_light_load_mode(true);
    TEST_ASSERT_TRUE(result.has_value());
    
    return 0;
}

// Main test runner
int main() {
    std::cout << "TPS55289 Driver Unit Tests (Simple Framework)" << std::endl;
    std::cout << "================================================" << std::endl;
    
    int failures = 0;
    
    // Run all tests
    failures += test_static_values();
    failures += test_vref_conversion();
    failures += test_error_codes();
    failures += test_write_register();
    failures += test_read_register();
    failures += test_current_limit();
    failures += test_output_voltage();
    failures += test_feedback_mode();
    failures += test_slew_rate();
    failures += test_ocp_delay();
    failures += test_cdc_compensation();
    failures += test_fault_indication();
    failures += test_mode_configuration();
    
    std::cout << "\n================================================" << std::endl;
    if (failures == 0) {
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << failures << " test(s) failed!" << std::endl;
        return 1;
    }
}
