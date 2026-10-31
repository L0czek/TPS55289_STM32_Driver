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

// Bit masks - field extraction masks for reading/writing specific bits
constexpr uint8_t TPS55289_IOUT_LIMIT_EN_MASK      = 0x80;
constexpr uint8_t TPS55289_IOUT_LIMIT_SETTING_MASK = 0x7F;

// VOUT_SR register field masks
constexpr uint8_t TPS55289_VOUT_SR_OCP_DELAY_MASK = 0x30;  // Bits 5-4
constexpr uint8_t TPS55289_VOUT_SR_SLEW_MASK      = 0x03;  // Bits 1-0

// VOUT_FS register field masks
constexpr uint8_t TPS55289_VOUT_FS_FB_MASK      = 0x80;   // Bit 7
constexpr uint8_t TPS55289_VOUT_FS_INTFB_MASK   = 0x03;   // Bits 1-0

// CDC register field masks
constexpr uint8_t TPS55289_CDC_SC_MASK_MASK   = 0x80;  // Bit 7
constexpr uint8_t TPS55289_CDC_OCP_MASK_MASK  = 0x40;  // Bit 6
constexpr uint8_t TPS55289_CDC_OVP_MASK_MASK  = 0x20;  // Bit 5
constexpr uint8_t TPS55289_CDC_OPTION_MASK    = 0x08;  // Bit 3
constexpr uint8_t TPS55289_CDC_SETTING_MASK   = 0x07;  // Bits 2-0

// MODE register field masks
constexpr uint8_t TPS55289_MODE_OE_MASK      = 0x80;  // Bit 7
constexpr uint8_t TPS55289_MODE_FSWDBL_MASK  = 0x40;  // Bit 6
constexpr uint8_t TPS55289_MODE_HICCUP_MASK  = 0x20;  // Bit 5
constexpr uint8_t TPS55289_MODE_DISCHG_MASK  = 0x10;  // Bit 4
constexpr uint8_t TPS55289_MODE_FPWM_MASK    = 0x02;  // Bit 1

// STATUS register field masks
constexpr uint8_t TPS55289_STATUS_SCP_MASK   = 0x80;  // Bit 7
constexpr uint8_t TPS55289_STATUS_OCP_MASK   = 0x40;  // Bit 6
constexpr uint8_t TPS55289_STATUS_OVP_MASK   = 0x20;  // Bit 5
constexpr uint8_t TPS55289_STATUS_MODE_MASK  = 0x03;  // Bits 1-0

// Named values for OCP_DELAY (bits 5-4 of VOUT_SR)
constexpr uint8_t TPS55289_OCP_DELAY_128US   = 0x00;  // 128 µs
constexpr uint8_t TPS55289_OCP_DELAY_3072US  = 0x01;  // 1.024 × 3 ms
constexpr uint8_t TPS55289_OCP_DELAY_6144US  = 0x02;  // 1.024 × 6 ms
constexpr uint8_t TPS55289_OCP_DELAY_12288US = 0x03;  // 1.024 × 12 ms

// Named values for SR (slew rate, bits 1-0 of VOUT_SR)
constexpr uint8_t TPS55289_SLEW_RATE_1P25MV  = 0x00;  // 1.25 mV/µs
constexpr uint8_t TPS55289_SLEW_RATE_2P5MV   = 0x01;  // 2.5 mV/µs (default)
constexpr uint8_t TPS55289_SLEW_RATE_5P0MV   = 0x02;  // 5 mV/µs
constexpr uint8_t TPS55289_SLEW_RATE_10P0MV  = 0x03;  // 10 mV/µs

// Named values for INTFB (bits 1-0 of VOUT_FS)
constexpr uint8_t TPS55289_INTFB_0P2256        = 0x00;  // 0.2256
constexpr uint8_t TPS55289_INTFB_0P1128        = 0x01;  // 0.1128
constexpr uint8_t TPS55289_INTFB_0P0752        = 0x02;  // 0.0752
constexpr uint8_t TPS55289_INTFB_0P0564        = 0x03;  // 0.0564 (default)

// Named values for CDC setting (bits 2-0 of CDC register)
constexpr uint8_t TPS55289_CDC_0V_COMP       = 0x00;  // 0-V output voltage rise
constexpr uint8_t TPS55289_CDC_0P1V_COMP     = 0x01;  // 0.1-V output voltage rise
constexpr uint8_t TPS55289_CDC_0P2V_COMP     = 0x02;  // 0.2-V output voltage rise
constexpr uint8_t TPS55289_CDC_0P3V_COMP     = 0x03;  // 0.3-V output voltage rise
constexpr uint8_t TPS55289_CDC_0P4V_COMP     = 0x04;  // 0.4-V output voltage rise
constexpr uint8_t TPS55289_CDC_0P5V_COMP     = 0x05;  // 0.5-V output voltage rise
constexpr uint8_t TPS55289_CDC_0P6V_COMP     = 0x06;  // 0.6-V output voltage rise
constexpr uint8_t TPS55289_CDC_0P7V_COMP     = 0x07;  // 0.7-V output voltage rise

// std::expected alias (driver is built as C++23)
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
    I2CError,
    InvalidParameter
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
            case ErrorCode::InvalidParameter: return "Invalid parameter";
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
 * @brief Decoded STATUS register (07h)
 *
 * Reading 07h clears the SCP, OCP and OVP bits (datasheet 7.6.7), so the
 * register must be read once and decoded from that single value.
 */
struct Status {
    bool scp;
    bool ocp;
    bool ovp;
    OperatingMode mode;

    bool any_fault() const { return scp || ocp || ovp; }
};

/**
 * @brief Fault indications that can be masked in the CDC register (05h)
 */
enum class FaultIndication : uint8_t {
    ShortCircuit = 0x80,   // SC_MASK
    OverCurrent  = 0x40,   // OCP_MASK
    OverVoltage  = 0x20    // OVP_MASK
};

/**
 * @brief TPS55289 buck-boost converter driver
 *
 * Call initialize() first: it checks the device answers and caches the active
 * feedback configuration. The output-voltage path uses only that cached ratio,
 * so a failed I2C read can never change the programmed voltage.
 */
class TPS55289 {
public:
    explicit TPS55289(I2C_HandleTypeDef* hi2c, uint8_t address = TPS55289_ADDRESS_MODE_HIGH);
    ~TPS55289();

    // ---- Low-level register access -------------------------------------------------
    expected<void> write_register(uint8_t register_addr, uint8_t value);
    expected<uint8_t> read_register(uint8_t register_addr) const;
    expected<void> update_register_bits(uint8_t register_addr, uint8_t mask, uint8_t value);

    // ---- Device control ------------------------------------------------------------
    /**
     * @brief Probe the device and cache the feedback configuration (VOUT_FS).
     *        Does not modify any register.
     */
    expected<void> initialize();

    /**
     * @brief Check the device answers on the bus (read-only, MODE register read)
     */
    expected<bool> verify_device_present() const;

    expected<void> enable();
    expected<void> disable();
    expected<bool> output_enabled() const;
    bool is_initialized() const { return initialized_; }

    // ---- Reference voltage and output voltage ----------------------------------------
    /** @brief VREF [mV] -> REF code, rounded and clamped to 0..0x7FE (NaN -> 0) */
    static uint16_t vref_mv_to_code(float vref_mv);
    static float ref_code_to_vref_mv(uint16_t code);

    expected<void> write_ref_code(uint16_t code);
    expected<uint16_t> read_ref_code() const;
    expected<void> set_reference_voltage_mv(float vref_mv);
    expected<float> get_reference_voltage_mv() const;

    /**
     * @brief Program the output voltage using the cached feedback ratio
     * @param voltage Requested output in volts; must be within
     *        [get_min_output_voltage(), get_max_output_voltage()]
     * @return The voltage actually programmed (after quantisation)
     */
    expected<float> set_output_voltage(float voltage);
    expected<float> get_output_voltage() const;

    /** @brief Upper bound accepted by set_output_voltage() (application safety limit) */
    expected<void> set_output_voltage_limit(float max_voltage);
    float get_min_output_voltage() const;
    float get_max_output_voltage() const;

    // ---- Current limit ---------------------------------------------------------------
    expected<void> enable_current_limit();
    expected<void> disable_current_limit();
    expected<bool> current_limit_enabled() const;
    expected<void> set_current_limit_voltage_mv(float limit_mv);
    expected<float> get_current_limit_voltage_mv() const;
    /** @param sense_resistor_ohms <= 0 selects the stored sense resistor value */
    expected<void> set_output_current_limit(float current_a, float sense_resistor_ohms = -1.0f);
    expected<float> get_output_current_limit(float sense_resistor_ohms = -1.0f) const;

    // ---- Slew rate and OCP delay -----------------------------------------------------
    expected<void> set_ocp_delay_us(uint32_t delay_us);
    expected<uint32_t> get_ocp_delay_us() const;
    /** @brief Selects the fastest setting not above the request (1.25/2.5/5/10 mV/us) */
    expected<void> set_slew_rate_mv_us(float slew_rate_mv_us);
    expected<float> get_slew_rate_mv_us() const;

    // ---- Feedback configuration ------------------------------------------------------
    expected<void> set_feedback_mode(FeedbackMode mode);
    expected<FeedbackMode> get_feedback_mode() const;
    expected<void> set_internal_feedback_ratio(uint8_t bits);
    expected<float> get_internal_feedback_ratio() const;
    /** @brief Cached ratio used for voltage conversion (valid after initialize()) */
    float get_active_feedback_ratio() const { return feedback_ratio_; }
    /** @brief Output voltage resolution in mV (0.5645 mV / ratio) */
    float get_output_voltage_step_mv() const;

    // ---- Cable droop compensation and fault indication (CDC, 05h) --------------------
    expected<void> set_fault_indication(FaultIndication fault, bool enabled);
    expected<bool> fault_indication_enabled(FaultIndication fault) const;
    expected<void> set_cdc_option(bool external);
    expected<bool> get_cdc_option() const;
    expected<void> set_cdc_compensation_v(float compensation_v);
    expected<float> get_cdc_compensation_v() const;

    // ---- Mode configuration (06h) ----------------------------------------------------
    expected<void> set_fsw_doubling(bool enabled);
    expected<bool> fsw_doubling_enabled() const;
    expected<void> set_hiccup_mode(bool enabled);
    expected<bool> hiccup_mode_enabled() const;
    expected<void> set_vout_discharge(bool enabled);
    expected<bool> vout_discharge_enabled() const;
    expected<void> set_light_load_mode(bool fpwm);
    expected<bool> light_load_mode_fpwm() const;

    // ---- Status and faults -----------------------------------------------------------
    /** @brief Read and decode STATUS once (clears the latched fault bits) */
    expected<Status> read_status() const;

    // ---- Static configuration values -------------------------------------------------
    static constexpr float get_vref_min_mv() { return 45.0f; }
    static constexpr float get_vref_lsb_mv() { return 0.5645f; }
    static constexpr uint16_t get_vref_max_code() { return 0x7FE; }
    static constexpr float get_vref_max_mv() { return get_vref_min_mv() + get_vref_max_code() * get_vref_lsb_mv(); }
    static constexpr float get_vout_min_v() { return 0.8f; }  // datasheet output range 0.8 V - 22 V

    uint8_t get_address() const { return address_; }
    float get_sense_resistor_ohms() const { return sense_resistor_ohms_; }
    expected<void> set_sense_resistor_ohms(float ohms);
    float get_external_feedback_ratio() const { return external_feedback_ratio_; }
    expected<void> set_external_feedback_ratio(float ratio);

private:
    I2C_HandleTypeDef* hi2c_;
    uint8_t address_;
    float sense_resistor_ohms_;
    float external_feedback_ratio_;
    float feedback_ratio_;        // active VOUT/VREF ratio, cached from VOUT_FS
    float max_output_voltage_;    // application limit for set_output_voltage()
    bool initialized_;

    static constexpr float DEFAULT_SENSE_RESISTOR = 0.01f;
    static constexpr float DEFAULT_EXTERNAL_FB_RATIO = 0.1128f;
    static constexpr float DEFAULT_INTERNAL_FB_RATIO = 0.0564f;  // VOUT_FS reset value 03h

    static std::error_code convertHALStatus(HAL_StatusTypeDef status);
    static float intfb_bits_to_ratio(uint8_t bits);
    expected<bool> read_flag(uint8_t register_addr, uint8_t mask) const;
    expected<void> refresh_feedback_ratio();
};

} // namespace tps55289

#endif // __TPS55289_HPP
