/**
 * @file tps55289.hpp
 * @brief TPS55289 I2C buck-boost converter driver for STM32
 *
 * This driver provides a modern C++ interface for the Texas Instruments
 * TPS55289 buck-boost converter with I2C control.
 */

#ifndef __TPS55289_HPP
#define __TPS55289_HPP

#include <cstdint>
#include <cstddef>
#include <array>
#include <string>
#include <cstring>
#include <stdexcept>
#include <system_error>

// TPS55289 configuration
#ifndef TPS55289_I2C_ADDRESS
#define TPS55289_I2C_ADDRESS 0x74
#endif

#ifndef TPS55289_TIMEOUT_MS
#define TPS55289_TIMEOUT_MS 1000
#endif

// Include HAL headers if not in unit test mode
#ifndef TPS55289_UNIT_TEST
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_i2c.h"
#else
// Unit test mode: Define minimal stub types needed by tps55289.cpp
typedef enum {
    HAL_OK = 0x00,
    HAL_ERROR = 0x01,
    HAL_BUSY = 0x02,
    HAL_TIMEOUT = 0x03
} HAL_StatusTypeDef;

// I2C memory address size (for HAL_I2C_Mem_Write and HAL_I2C_Mem_Read)
#define I2C_MEMADD_SIZE_8BIT 0x01
#define I2C_MEMADD_SIZE_16BIT 0x02

struct I2C_HandleTypeDef {
    void* Instance;
    uint32_t State;
    void* RegIOHandle;
    uint32_t DevAddress;
    uint32_t MemAddress;
    uint32_t MemAddSize;
    uint32_t Timeout;
};

// External I2C handles (defined in mock_hal.cpp)
extern "C" {
    extern I2C_HandleTypeDef hi2c1;
}

// HAL function declarations (for mock implementation)
extern "C" {
    void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c);
    void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c);
    HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef* hi2c, uint16_t DevAddress,
                                         uint16_t MemAddress, uint16_t MemAddSize,
                                         uint8_t* pData, uint16_t Size, uint32_t Timeout);
    HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef* hi2c, uint16_t DevAddress,
                                        uint16_t MemAddress, uint16_t MemAddSize,
                                        uint8_t* pData, uint16_t Size, uint32_t Timeout);
    void HAL_Init(void);
    void HAL_MspInit(void);
    void HAL_IncTick(void);
    void HAL_Delay(uint32_t Delay);
    HAL_StatusTypeDef HAL_RCC_OscConfig(void* RCC_OscInitStruct);
    HAL_StatusTypeDef HAL_RCC_ClockConfig(void* RCC_ClkInitStruct, uint32_t FlashLatency);
    void HAL_PWREx_ControlVoltageScaling(int Voltage);
    void HAL_GPIO_Init(void* GPIOx, void* GPIO_Init);
    void HAL_GPIO_DeInit(void* GPIOx, uint32_t GPIO_Pin);
    void HAL_I2C_Init(I2C_HandleTypeDef* hi2c);
    HAL_StatusTypeDef HAL_I2CEx_ConfigAnalogFilter(I2C_HandleTypeDef* hi2c, uint32_t AnalogFilter);
    HAL_StatusTypeDef HAL_I2CEx_ConfigDigitalFilter(I2C_HandleTypeDef* hi2c, uint32_t DigitalFilter);
    void Error_Handler(void);
    void __disable_irq(void);
    void __enable_irq(void);
    void HAL_GPIO_TogglePin(void* GPIOx, uint32_t GPIO_Pin);
    void HAL_GPIO_WritePin(void* GPIOx, uint32_t GPIO_Pin, uint32_t PinState);
    uint32_t HAL_GetTick(void);
}
#endif

// Register addresses
constexpr uint8_t TPS55289_REG_REF_LSB      = 0x00;
constexpr uint8_t TPS55289_REG_REF_MSB      = 0x01;
constexpr uint8_t TPS55289_REG_IOUT_LIMIT   = 0x02;
constexpr uint8_t TPS55289_REG_VOUT_SR      = 0x03;
constexpr uint8_t TPS55289_REG_VOUT_FS      = 0x04;
constexpr uint8_t TPS55289_REG_CDC          = 0x05;
constexpr uint8_t TPS55289_REG_MODE         = 0x06;
constexpr uint8_t TPS55289_REG_STATUS       = 0x07;

// I2C addresses selected by the MODE pin
constexpr uint8_t TPS55289_ADDRESS_MODE_HIGH = 0x74;
constexpr uint8_t TPS55289_ADDRESS_MODE_LOW  = 0x75;

// Bit masks
constexpr uint8_t TPS55289_IOUT_LIMIT_EN      = 0x80;
constexpr uint8_t TPS55289_IOUT_LIMIT_SETTING = 0x7F;

constexpr uint8_t TPS55289_VOUT_SR_OCP_DELAY = 0x30;
constexpr uint8_t TPS55289_VOUT_SR_SLEW      = 0x03;

constexpr uint8_t TPS55289_VOUT_FS_FB      = 0x80;
constexpr uint8_t TPS55289_VOUT_FS_INTFB   = 0x03;

constexpr uint8_t TPS55289_CDC_SC_MASK   = 0x80;
constexpr uint8_t TPS55289_CDC_OCP_MASK  = 0x40;
constexpr uint8_t TPS55289_CDC_OVP_MASK  = 0x20;
constexpr uint8_t TPS55289_CDC_OPTION    = 0x08;
constexpr uint8_t TPS55289_CDC_SETTING   = 0x07;

constexpr uint8_t TPS55289_MODE_OE      = 0x80;
constexpr uint8_t TPS55289_MODE_FSWDBL  = 0x40;
constexpr uint8_t TPS55289_MODE_HICCUP  = 0x20;
constexpr uint8_t TPS55289_MODE_DISCHG  = 0x10;
constexpr uint8_t TPS55289_MODE_FPWM    = 0x02;

constexpr uint8_t TPS55289_STATUS_SCP = 0x80;
constexpr uint8_t TPS55289_STATUS_OCP = 0x40;
constexpr uint8_t TPS55289_STATUS_OVP = 0x20;
constexpr uint8_t TPS55289_STATUS_MODE = 0x03;

// Include custom expected for C++17 compatibility
// This must be before the namespace definition so std:: headers are not wrapped
#include "tps55289_expected.hpp"

namespace tps55289 {

/**
 * @brief Error codes for TPS55289 operations
 */
enum class ErrorCode : uint8_t {
    None = 0,
    HALError,
    Timeout,
    InvalidAddress,
    InvalidRegister,
    NotInitialized,
    I2CError
};

/**
 * @brief Feedback mode for output regulation
 */
enum class FeedbackMode : uint8_t {
    Internal = 0,
    External = 1
};

/**
 * @brief Operating mode status
 */
enum class OperatingMode : uint8_t {
    Boost = 0,
    Buck = 1,
    BuckBoost = 2,
    Reserved = 3
};

/**
 * @brief Custom error_category implementation for std::expected
 */
class TPS55289ErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "tps55289";
    }

    std::string message(int ev) const override {
        switch (static_cast<ErrorCode>(ev)) {
            case ErrorCode::None: return "No error";
            case ErrorCode::HALError: return "HAL error";
            case ErrorCode::Timeout: return "Operation timeout";
            case ErrorCode::InvalidAddress: return "Invalid I2C address";
            case ErrorCode::InvalidRegister: return "Invalid register address";
            case ErrorCode::NotInitialized: return "Device not initialized";
            case ErrorCode::I2CError: return "I2C communication error";
            default: return "Unknown error";
        }
    }
};

/**
 * @brief Get the TPS55289 error category instance
 */
inline const TPS55289ErrorCategory& get_error_category() {
    static TPS55289ErrorCategory instance;
    return instance;
}

/**
 * @brief Create an error code from ErrorCode enum
 */
inline std::error_code make_error_code(ErrorCode ec) {
    return std::error_code(static_cast<int>(ec), get_error_category());
}

/**
 * @brief TPS55289 Driver Class
 *
 * This class provides a modern C++ interface for the TPS55289 buck-boost converter.
 * It manages I2C communication internally and provides methods for all
 * TPS55289 register operations with proper error handling via expected.
 */
class TPS55289 {
public:
    /**
     * @brief Construct TPS55289 driver
     * @param hi2c Pointer to I2C handle
     * @param address I2C address (default: 0x74 for MODE pin high)
     */
    explicit TPS55289(I2C_HandleTypeDef* hi2c, uint8_t address = TPS55289_ADDRESS_MODE_HIGH);

    /**
     * @brief Destroy TPS55289 driver
     */
    ~TPS55289();

    // =============================================================================
    // Low-level I2C operations
    // =============================================================================

    /**
     * @brief Write a register value (non-const, modifies initialized flag)
     * @param register_addr Register address
     * @param value Value to write
     * @return expected<void> - error on failure
     */
    expected<void> write_register(uint8_t register_addr, uint8_t value);

    /**
     * @brief Read a register value (non-const, modifies initialized flag)
     * @param register_addr Register address
     * @return expected<uint8_t> - register value or error
     */
    expected<uint8_t> read_register(uint8_t register_addr);

    /**
     * @brief Update specific bits in a register (non-const, modifies initialized flag)
     * @param register_addr Register address
     * @param mask Bit mask for the field
     * @param value New value for the field (aligned to mask)
     * @return expected<void> - error on failure
     */
    expected<void> update_register_bits(uint8_t register_addr, uint8_t mask, uint8_t value);

    /**
     * @brief Read a register value (const, does not modify state)
     * @param register_addr Register address
     * @return uint8_t - register value
     */
    uint8_t read_register_const(uint8_t register_addr) const;

    // =============================================================================
    // Device control
    // =============================================================================

    /**
     * @brief Verify device is present on I2C bus
     * @return expected<bool> - true if device found, error on failure
     */
    expected<bool> verify_device_present();

    /**
     * @brief Enable the output
     * @return expected<void> - error on failure
     */
    expected<void> enable();

    /**
     * @brief Disable the output
     * @return expected<void> - error on failure
     */
    expected<void> disable();

    /**
     * @brief Check if output is enabled
     * @return bool - true if enabled
     */
    bool output_enabled() const;

    /**
     * @brief Check if device is initialized
     * @return bool - true if initialized
     */
    bool is_initialized() const { return initialized_; }

    // =============================================================================
    // Reference voltage and output voltage
    // =============================================================================

    /**
     * @brief Convert output voltage to REF code
     * @param voltage Output voltage in volts
     * @param feedback_ratio Feedback ratio (optional, uses current setting if not provided)
     * @return uint16_t REF code
     */
    uint16_t voltage_to_ref_code(float voltage, float feedback_ratio = -1.0f);

    /**
     * @brief Convert VREF in mV to REF code
     * @param vref_mv VREF in millivolts
     * @return uint16_t REF code
     */
    uint16_t vref_mv_to_code(float vref_mv);

    /**
     * @brief Convert REF code to VREF in mV
     * @param code REF code
     * @return float VREF in millivolts
     */
    float ref_code_to_vref_mv(uint16_t code);

    /**
     * @brief Write REF code to registers
     * @param code REF code (0-0x7FF)
     * @return expected<void> - error on failure
     */
    expected<void> write_ref_code(uint16_t code);

    /**
     * @brief Read REF code from registers
     * @return expected<uint16_t> - REF code or error
     */
    expected<uint16_t> read_ref_code();

    /**
     * @brief Set reference voltage in mV
     * @param vref_mv Reference voltage in millivolts
     * @return expected<void> - error on failure
     */
    expected<void> set_reference_voltage_mv(float vref_mv);

    /**
     * @brief Get reference voltage in mV
     * @return expected<float> - reference voltage or error
     */
    expected<float> get_reference_voltage_mv();

    /**
     * @brief Set output voltage
     * @param voltage Output voltage in volts
     * @return expected<float> - actual voltage set or error
     */
    expected<float> set_output_voltage(float voltage);

    /**
     * @brief Get output voltage
     * @return expected<float> - output voltage or error
     */
    expected<float> get_output_voltage();

    // =============================================================================
    // Current limit
    // =============================================================================

    /**
     * @brief Enable current limit
     * @return expected<void> - error on failure
     */
    expected<void> enable_current_limit();

    /**
     * @brief Disable current limit
     * @return expected<void> - error on failure
     */
    expected<void> disable_current_limit();

    /**
     * @brief Check if current limit is enabled
     * @return bool - true if enabled
     */
    bool current_limit_enabled() const;

    /**
     * @brief Set current limit sense voltage
     * @param limit_mv Sense voltage in millivolts (0-63.5 mV)
     * @return expected<void> - error on failure
     */
    expected<void> set_current_limit_voltage_mv(float limit_mv);

    /**
     * @brief Get current limit sense voltage
     * @return expected<float> - sense voltage in mV or error
     */
    expected<float> get_current_limit_voltage_mv();

    /**
     * @brief Set output current limit
     * @param current_a Output current limit in amperes
     * @param sense_resistor_ohms Sense resistor value in ohms (optional, uses default if not provided)
     * @return expected<void> - error on failure
     */
    expected<void> set_output_current_limit(float current_a, float sense_resistor_ohms = -1.0f);

    /**
     * @brief Get output current limit
     * @param sense_resistor_ohms Sense resistor value in ohms (optional, uses default if not provided)
     * @return expected<float> - current limit in amperes or error
     */
    expected<float> get_output_current_limit(float sense_resistor_ohms = -1.0f);

    // =============================================================================
    // Slew rate and OCP delay
    // =============================================================================

    /**
     * @brief Set OCP delay
     * @param delay_us OCP delay in microseconds (128, 3072, 6144, 12288)
     * @return expected<void> - error on failure
     */
    expected<void> set_ocp_delay_us(uint32_t delay_us);

    /**
     * @brief Get OCP delay
     * @return expected<uint32_t> - OCP delay in microseconds or error
     */
    expected<uint32_t> get_ocp_delay_us();

    /**
     * @brief Set output voltage slew rate
     * @param slew_rate_mv_us Slew rate in mV/us (1.25, 2.5, 5.0, 10.0)
     * @return expected<void> - error on failure
     */
    expected<void> set_slew_rate_mv_us(float slew_rate_mv_us);

    /**
     * @brief Get output voltage slew rate
     * @return expected<float> - slew rate in mV/us or error
     */
    expected<float> get_slew_rate_mv_us();

    // =============================================================================
    // Feedback configuration
    // =============================================================================

    /**
     * @brief Set feedback mode
     * @param mode Feedback mode (Internal or External)
     * @return expected<void> - error on failure
     */
    expected<void> set_feedback_mode(FeedbackMode mode);

    /**
     * @brief Get feedback mode
     * @return FeedbackMode - current feedback mode
     */
    FeedbackMode get_feedback_mode() const;

    /**
     * @brief Set internal feedback ratio
     * @param bits Feedback ratio bits (0-3)
     * @return expected<void> - error on failure
     */
    expected<void> set_internal_feedback_ratio(uint8_t bits);

    /**
     * @brief Get internal feedback ratio
     * @return expected<float> - feedback ratio or error
     */
    expected<float> get_internal_feedback_ratio() const;

    /**
     * @brief Get active feedback ratio
     * @return float - active feedback ratio
     */
    float get_active_feedback_ratio() const;

    /**
     * @brief Get output voltage step size in mV
     * @return uint16_t - step size in millivolts
     */
    uint16_t get_output_voltage_step_mv() const;

    // =============================================================================
    // Cable droop compensation (CDC)
    // =============================================================================

    /**
     * @brief Enable fault indication
     * @param fault Fault type ("scp", "ocp", "ovp")
     * @return expected<void> - error on failure
     */
    expected<void> enable_fault_indication(const std::string& fault);

    /**
     * @brief Disable fault indication
     * @param fault Fault type ("scp", "ocp", "ovp")
     * @return expected<void> - error on failure
     */
    expected<void> disable_fault_indication(const std::string& fault);

    /**
     * @brief Check if fault indication is enabled
     * @param fault Fault type ("scp", "ocp", "ovp")
     * @return bool - true if enabled
     */
    bool fault_indication_enabled(const std::string& fault) const;

    /**
     * @brief Set CDC option
     * @param option CDC option (internal or external)
     * @return expected<void> - error on failure
     */
    expected<void> set_cdc_option(bool external);

    /**
     * @brief Get CDC option
     * @return bool - true if external CDC
     */
    bool get_cdc_option() const;

    /**
     * @brief Set CDC compensation voltage
     * @param compensation_v CDC compensation in volts (0-0.7 V)
     * @return expected<void> - error on failure
     */
    expected<void> set_cdc_compensation_v(float compensation_v);

    /**
     * @brief Get CDC compensation voltage
     * @return expected<float> - CDC compensation in volts or error
     */
    expected<float> get_cdc_compensation_v();

    // =============================================================================
    // Mode configuration
    // =============================================================================

    /**
     * @brief Set FSW doubling
     * @param enabled True to enable FSW doubling
     * @return expected<void> - error on failure
     */
    expected<void> set_fsw_doubling(bool enabled);

    /**
     * @brief Check if FSW doubling is enabled
     * @return bool - true if enabled
     */
    bool fsw_doubling_enabled() const;

    /**
     * @brief Set hiccup mode
     * @param enabled True to enable hiccup mode
     * @return expected<void> - error on failure
     */
    expected<void> set_hiccup_mode(bool enabled);

    /**
     * @brief Check if hiccup mode is enabled
     * @return bool - true if enabled
     */
    bool hiccup_mode_enabled() const;

    /**
     * @brief Set VOUT discharge
     * @param enabled True to enable VOUT discharge
     * @return expected<void> - error on failure
     */
    expected<void> set_vout_discharge(bool enabled);

    /**
     * @brief Check if VOUT discharge is enabled
     * @return bool - true if enabled
     */
    bool vout_discharge_enabled() const;

    /**
     * @brief Set light load mode
     * @param fpwm True for FPWM mode, false for PFM mode
     * @return expected<void> - error on failure
     */
    expected<void> set_light_load_mode(bool fpwm);

    /**
     * @brief Check if light load mode is FPWM
     * @return bool - true if FPWM mode
     */
    bool light_load_mode_fpwm() const;

    // =============================================================================
    // Status and faults
    // =============================================================================

    /**
     * @brief Read status register
     * @return expected<uint8_t> - status register value or error
     */
    expected<uint8_t> read_status();

    /**
     * @brief Check for faults
     * @return bool - true if any fault is present
     */
    bool has_fault() const;

    /**
     * @brief Check for short circuit fault
     * @return bool - true if SCP is active
     */
    bool has_scp() const;

    /**
     * @brief Check for overcurrent fault
     * @return bool - true if OCP is active
     */
    bool has_ocp() const;

    /**
     * @brief Check for overvoltage fault
     * @return bool - true if OVP is active
     */
    bool has_ovp() const;

    /**
     * @brief Get operating mode
     * @return OperatingMode - current operating mode
     */
    OperatingMode get_operating_mode() const;

    // =============================================================================
    // Static configuration values
    // =============================================================================

    /**
     * @brief Get minimum reference voltage in mV
     */
    static constexpr float get_vref_min_mv() { return 45.0f; }

    /**
     * @brief Get VREF LSB size in mV
     */
    static constexpr float get_vref_lsb_mv() { return 0.5645f; }

    /**
     * @brief Get maximum REF code
     */
    static constexpr uint16_t get_vref_max_code() { return 0x7FE; }

    /**
     * @brief Get I2C address
     */
    uint8_t get_address() const { return address_; }

    /**
     * @brief Get sense resistor value
     */
    float get_sense_resistor_ohms() const { return sense_resistor_ohms_; }

    /**
     * @brief Set sense resistor value
     * @param ohms Sense resistor value in ohms
     */
    void set_sense_resistor_ohms(float ohms) { sense_resistor_ohms_ = ohms; }

    /**
     * @brief Get external feedback ratio
     */
    float get_external_feedback_ratio() const { return external_feedback_ratio_; }

    /**
     * @brief Set external feedback ratio
     * @param ratio External feedback ratio
     */
    void set_external_feedback_ratio(float ratio) { external_feedback_ratio_ = ratio; }

private:
    I2C_HandleTypeDef* hi2c_;
    uint8_t address_;
    float sense_resistor_ohms_;
    float external_feedback_ratio_;
    bool initialized_;

    static constexpr float DEFAULT_SENSE_RESISTOR = 0.01f;
    static constexpr float DEFAULT_EXTERNAL_FB_RATIO = 0.1128f;

    /**
     * @brief Convert HAL status to error code
     */
    static std::error_code convertHALStatus(HAL_StatusTypeDef status);

    /**
     * @brief Validate REF code
     */
    static bool validate_ref_code(uint16_t code);

    /**
     * @brief Get feedback ratio bits from value
     */
    static uint8_t get_feedback_ratio_bits(float ratio);

    /**
     * @brief Get output step bits from value
     */
    static uint8_t get_output_step_bits(float step_mv);

    /**
     * @brief Get OCP delay bits from value
     */
    static uint8_t get_ocp_delay_bits(uint32_t delay_us);

    /**
     * @brief Get slew rate bits from value
     */
    static uint8_t get_slew_rate_bits(float slew_rate_mv_us);
};

} // namespace tps55289

#endif // __TPS55289_HPP
