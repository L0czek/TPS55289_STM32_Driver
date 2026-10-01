/**
 * @file test_tps55289.cpp
 * @brief Unit tests for the TPS55289 driver
 *
 * This file contains unit tests for the TPS55289 driver using Google Test.
 * It tests the register operations, voltage calculations, and error handling.
 */

#include "tps55289.hpp"
#include <gtest/gtest.h>
#include <cmath>

// Include the mock HAL which provides I2C_HandleTypeDef and HAL function definitions
// The mock HAL is in a separate .c file to provide C linkage

// Test fixture
class TPS55289Test : public ::testing::Test {
protected:
    I2C_HandleTypeDef hi2c_;
    tps55289::TPS55289 tps55289_;

    TPS55289Test() : tps55289_(&hi2c_) {}
};

// Test static configuration values
TEST_F(TPS55289Test, StaticValues) {
    EXPECT_FLOAT_EQ(tps55289::TPS55289::get_vref_min_mv(), 45.0f);
    EXPECT_FLOAT_EQ(tps55289::TPS55289::get_vref_lsb_mv(), 0.5645f);
    EXPECT_EQ(tps55289::TPS55289::get_vref_max_code(), 0x7FE);
}

// Test VREF code conversion
TEST_F(TPS55289Test, VREFConversion) {
    // Test vref_mv_to_code
    uint16_t code = tps55289_.vref_mv_to_code(500.0f);
    EXPECT_NEAR(code, 788, 2);  // (500 - 45) / 0.5645 ≈ 788

    // Test ref_code_to_vref_mv
    float vref = tps55289_.ref_code_to_vref_mv(code);
    EXPECT_NEAR(vref, 500.0f, 0.6f);  // Allow small rounding error

    // Test voltage_to_ref_code
    code = tps55289_.voltage_to_ref_code(5.0f, 0.1128f);
    EXPECT_GT(code, 0);

    // Test boundary conditions
    EXPECT_EQ(tps55289_.vref_mv_to_code(0.0f), 0);  // Below minimum
    EXPECT_EQ(tps55289_.vref_mv_to_code(2000.0f), 0x7FE);  // Above maximum
}

// Test error code conversion
TEST_F(TPS55289Test, ErrorCodes) {
    auto ec = tps55289::make_error_code(tps55289::ErrorCode::HALError);
    EXPECT_EQ(ec.category().name(), std::string("tps55289"));
    EXPECT_EQ(ec.message(), std::string("HAL error"));

    ec = tps55289::make_error_code(tps55289::ErrorCode::Timeout);
    EXPECT_EQ(ec.message(), std::string("Operation timeout"));

    ec = tps55289::make_error_code(tps55289::ErrorCode::None);
    EXPECT_EQ(ec.message(), std::string("No error"));
}

// Test register write
TEST_F(TPS55289Test, WriteRegister) {
    auto result = tps55289_.write_register(0x00, 0x12);
    EXPECT_TRUE(result.has_value());
}

// Test register read
TEST_F(TPS55289Test, ReadRegister) {
    auto result = tps55289_.read_register(0x00);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 0);  // Mock returns 0
}

// Test current limit
TEST_F(TPS55289Test, CurrentLimit) {
    auto result = tps55289_.set_current_limit_voltage_mv(50.0f);
    EXPECT_TRUE(result.has_value());

    auto current = tps55289_.get_current_limit_voltage_mv();
    EXPECT_TRUE(current.has_value());
    EXPECT_NEAR(current.value(), 50.0f, 0.6f);
}

// Test output voltage
TEST_F(TPS55289Test, OutputVoltage) {
    // Test with internal feedback (0.1128 ratio)
    auto result = tps55289_.set_output_voltage(5.0f);
    EXPECT_TRUE(result.has_value());

    auto voltage = tps55289_.get_output_voltage();
    EXPECT_TRUE(voltage.has_value());
    EXPECT_NEAR(voltage.value(), 5.0f, 0.1f);
}

// Test feedback mode
TEST_F(TPS55289Test, FeedbackMode) {
    auto result = tps55289_.set_feedback_mode(tps55289::FeedbackMode::External);
    EXPECT_TRUE(result.has_value());

    auto mode = tps55289_.get_feedback_mode();
    EXPECT_EQ(mode, tps55289::FeedbackMode::External);
}

// Test slew rate
TEST_F(TPS55289Test, SlewRate) {
    auto result = tps55289_.set_slew_rate_mv_us(5.0f);
    EXPECT_TRUE(result.has_value());

    auto rate = tps55289_.get_slew_rate_mv_us();
    EXPECT_TRUE(rate.has_value());
    EXPECT_NEAR(rate.value(), 5.0f, 0.1f);
}

// Test OCP delay
TEST_F(TPS55289Test, OCPDelay) {
    auto result = tps55289_.set_ocp_delay_us(3072);
    EXPECT_TRUE(result.has_value());

    auto delay = tps55289_.get_ocp_delay_us();
    EXPECT_TRUE(delay.has_value());
    EXPECT_EQ(delay.value(), 3072);
}

// Test CDC compensation
TEST_F(TPS55289Test, CDCCompensation) {
    auto result = tps55289_.set_cdc_compensation_v(0.35f);
    EXPECT_TRUE(result.has_value());

    auto cdc = tps55289_.get_cdc_compensation_v();
    EXPECT_TRUE(cdc.has_value());
    EXPECT_NEAR(cdc.value(), 0.35f, 0.1f);
}

// Test fault indication
TEST_F(TPS55289Test, FaultIndication) {
    auto result = tps55289_.enable_fault_indication("ocp");
    EXPECT_TRUE(result.has_value());

    result = tps55289_.disable_fault_indication("ocp");
    EXPECT_TRUE(result.has_value());
}

// Test mode configuration
TEST_F(TPS55289Test, ModeConfiguration) {
    auto result = tps55289_.set_fsw_doubling(true);
    EXPECT_TRUE(result.has_value());

    result = tps55289_.set_hiccup_mode(true);
    EXPECT_TRUE(result.has_value());

    result = tps55289_.set_vout_discharge(true);
    EXPECT_TRUE(result.has_value());

    result = tps55289_.set_light_load_mode(true);
    EXPECT_TRUE(result.has_value());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
