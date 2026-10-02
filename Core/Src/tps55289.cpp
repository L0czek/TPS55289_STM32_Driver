#include "tps55289.hpp"
#include <cstring>
#include <cmath>

namespace tps55289 {

// =============================================================================
// TPS55289 Implementation
// =============================================================================

TPS55289::TPS55289(I2C_HandleTypeDef* hi2c, uint8_t address)
    : hi2c_(hi2c), address_(address), sense_resistor_ohms_(DEFAULT_SENSE_RESISTOR),
      external_feedback_ratio_(DEFAULT_EXTERNAL_FB_RATIO), initialized_(false)
{
}

TPS55289::~TPS55289() = default;

std::error_code TPS55289::convertHALStatus(HAL_StatusTypeDef status) {
    switch (status) {
        case HAL_OK:
            return make_error_code(ErrorCode::None);
        case HAL_TIMEOUT:
            return make_error_code(ErrorCode::Timeout);
        case HAL_ERROR:
            return make_error_code(ErrorCode::HALError);
        case HAL_BUSY:
            return make_error_code(ErrorCode::Timeout);
        default:
            return make_error_code(ErrorCode::HALError);
    }
}

bool TPS55289::validate_ref_code(uint16_t code) {
    return code <= get_vref_max_code();
}

// =============================================================================
// Low-level I2C operations
// =============================================================================

tps55289::expected<void> TPS55289::write_register(uint8_t register_addr, uint8_t value) {
    if (!hi2c_) {
        return make_error_code(ErrorCode::HALError);
    }

    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(hi2c_, (address_ << 1),
                                                  register_addr, I2C_MEMADD_SIZE_8BIT,
                                                  &value, 1, TPS55289_TIMEOUT_MS);
    if (status != HAL_OK) {
        return convertHALStatus(status);
    }

    initialized_ = true;
    return {};
}

tps55289::expected<uint8_t> TPS55289::read_register(uint8_t register_addr) {
    if (!hi2c_) {
        return make_error_code(ErrorCode::HALError);
    }

    uint8_t value = 0;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c_, (address_ << 1),
                                                 register_addr, I2C_MEMADD_SIZE_8BIT,
                                                 &value, 1, TPS55289_TIMEOUT_MS);
    if (status != HAL_OK) {
        return convertHALStatus(status);
    }

    initialized_ = true;
    return value;
}

tps55289::expected<uint8_t> TPS55289::read_register(uint8_t register_addr) const {
    if (!hi2c_) {
        return make_error_code(ErrorCode::HALError);
    }

    uint8_t value = 0;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c_, (address_ << 1),
                                                 register_addr, I2C_MEMADD_SIZE_8BIT,
                                                 &value, 1, TPS55289_TIMEOUT_MS);
    if (status != HAL_OK) {
        return convertHALStatus(status);
    }

    return value;
}

uint8_t TPS55289::read_register_const(uint8_t register_addr) const {
    if (!hi2c_) {
        return 0;
    }

    uint8_t value = 0;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c_, (address_ << 1),
                                                 register_addr, I2C_MEMADD_SIZE_8BIT,
                                                 &value, 1, TPS55289_TIMEOUT_MS);
    (void)status;  // Ignore status for const reads - we just return 0 on error
    return value;
}

tps55289::expected<void> TPS55289::update_register_bits(uint8_t register_addr, uint8_t mask, uint8_t value) {
    auto current_opt = read_register(register_addr);
    if (!current_opt) {
        return current_opt.error();
    }

    uint8_t current = current_opt.value();
    uint8_t new_value = (current & (~mask & 0xFF)) | (value & mask);
    return write_register(register_addr, new_value);
}

// =============================================================================
// Device control
// =============================================================================

tps55289::expected<bool> TPS55289::verify_device_present() {
    if (!hi2c_) {
        return make_error_code(ErrorCode::HALError);
    }

    // Try to write to a register to verify device is present
    uint8_t test_value = 0;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(hi2c_, (address_ << 1),
                                                  TPS55289_REG_MODE, I2C_MEMADD_SIZE_8BIT,
                                                  &test_value, 1, TPS55289_TIMEOUT_MS);
    
    if (status == HAL_OK) {
        // Restore the value (we just read MODE in other operations)
        return true;
    }
    
    return convertHALStatus(status);
}

tps55289::expected<void> TPS55289::enable() {
    auto result = update_register_bits(TPS55289_REG_MODE, TPS55289_MODE_OE_MASK, TPS55289_MODE_OE_MASK);
    if (result) {
        initialized_ = true;
    }
    return result;
}

tps55289::expected<void> TPS55289::disable() {
    auto result = update_register_bits(TPS55289_REG_MODE, TPS55289_MODE_OE_MASK, 0x00);
    if (result) {
        initialized_ = false;
    }
    return result;
}

bool TPS55289::output_enabled() const {
    uint8_t reg = read_register_const(TPS55289_REG_MODE);
    return (reg & TPS55289_MODE_OE_MASK) != 0;
}

// =============================================================================
// Reference voltage and output voltage
// =============================================================================

uint16_t TPS55289::voltage_to_ref_code(float voltage, float feedback_ratio) {
    if (feedback_ratio < 0) {
        feedback_ratio = get_active_feedback_ratio();
    }
    
    float vref_mv = voltage * feedback_ratio * 1000.0f;
    return vref_mv_to_code(vref_mv);
}

uint16_t TPS55289::vref_mv_to_code(float vref_mv) {
    float code_f = std::round((vref_mv - get_vref_min_mv()) / get_vref_lsb_mv());
    int16_t code = static_cast<int16_t>(code_f);
    
    if (code < 0) {
        code = 0;
    }
    if (code > get_vref_max_code()) {
        code = get_vref_max_code();
    }
    
    return static_cast<uint16_t>(code);
}

float TPS55289::ref_code_to_vref_mv(uint16_t code) const {
    code = code & 0x7FF;
    return get_vref_min_mv() + code * get_vref_lsb_mv();
}

tps55289::expected<void> TPS55289::write_ref_code(uint16_t code) {
    if (!validate_ref_code(code)) {
        return make_error_code(ErrorCode::InvalidAddress);
    }
    
    // Datasheet explicitly says to write 00h first, then 01h.
    auto lsb_result = write_register(TPS55289_REG_REF_LSB, code & 0xFF);
    if (!lsb_result) {
        return lsb_result.error();
    }
    
    auto msb_result = write_register(TPS55289_REG_REF_MSB, (code >> 8) & 0x07);
    if (!msb_result) {
        return msb_result.error();
    }
    
    return {};
}

tps55289::expected<uint16_t> TPS55289::read_ref_code() const {
    uint8_t lsb = read_register_const(TPS55289_REG_REF_LSB);
    uint8_t msb = read_register_const(TPS55289_REG_REF_MSB);
    
    return (static_cast<uint16_t>(msb) << 8) | lsb;
}

tps55289::expected<void> TPS55289::set_reference_voltage_mv(float vref_mv) {
    uint16_t code = vref_mv_to_code(vref_mv);
    return write_ref_code(code);
}

tps55289::expected<float> TPS55289::get_reference_voltage_mv() const {
    auto code_opt = read_ref_code();
    if (!code_opt) {
        return code_opt.error();
    }

    return ref_code_to_vref_mv(code_opt.value());
}

tps55289::expected<float> TPS55289::set_output_voltage(float voltage) {
    uint16_t code = voltage_to_ref_code(voltage);
    auto result = write_ref_code(code);
    if (!result) {
        return result.error();
    }
    
    return get_output_voltage();
}

tps55289::expected<float> TPS55289::get_output_voltage() const {
    float feedback_ratio = get_active_feedback_ratio();
    if (feedback_ratio <= 0) {
        return make_error_code(ErrorCode::NotInitialized);
    }

    auto vref_opt = get_reference_voltage_mv();
    if (!vref_opt) {
        return vref_opt.error();
    }

    return (vref_opt.value() / 1000.0f) / feedback_ratio;
}

// =============================================================================
// Current limit
// =============================================================================

tps55289::expected<void> TPS55289::enable_current_limit() {
    return update_register_bits(TPS55289_REG_IOUT_LIMIT, TPS55289_IOUT_LIMIT_EN_MASK, TPS55289_IOUT_LIMIT_EN_MASK);
}

tps55289::expected<void> TPS55289::disable_current_limit() {
    return update_register_bits(TPS55289_REG_IOUT_LIMIT, TPS55289_IOUT_LIMIT_EN_MASK, 0x00);
}

bool TPS55289::current_limit_enabled() const {
    uint8_t reg = read_register_const(TPS55289_REG_IOUT_LIMIT);
    return (reg & TPS55289_IOUT_LIMIT_EN_MASK) != 0;
}

tps55289::expected<void> TPS55289::set_current_limit_voltage_mv(float limit_mv) {
    if (limit_mv < 0.0f || limit_mv > 63.5f) {
        return make_error_code(ErrorCode::InvalidAddress);
    }
    
    uint8_t code = static_cast<uint8_t>(std::round(limit_mv / 0.5f));
    if (code > 0x7F) {
        code = 0x7F;
    }
    
    return update_register_bits(TPS55289_REG_IOUT_LIMIT, TPS55289_IOUT_LIMIT_SETTING_MASK, code);
}

tps55289::expected<float> TPS55289::get_current_limit_voltage_mv() const {
    uint8_t code = read_register_const(TPS55289_REG_IOUT_LIMIT);
    code &= TPS55289_IOUT_LIMIT_SETTING_MASK;
    return code * 0.5f;
}

tps55289::expected<void> TPS55289::set_output_current_limit(float current_a, float sense_resistor_ohms) {
    if (current_a < 0) {
        return make_error_code(ErrorCode::InvalidAddress);
    }
    
    if (sense_resistor_ohms < 0) {
        sense_resistor_ohms = sense_resistor_ohms_;
    }
    
    float limit_mv = current_a * sense_resistor_ohms * 1000.0f;
    return set_current_limit_voltage_mv(limit_mv);
}

tps55289::expected<float> TPS55289::get_output_current_limit(float sense_resistor_ohms) const {
    if (sense_resistor_ohms < 0) {
        sense_resistor_ohms = sense_resistor_ohms_;
    }
    
    auto voltage_opt = get_current_limit_voltage_mv();
    if (!voltage_opt) {
        return voltage_opt.error();
    }
    
    return voltage_opt.value() / 1000.0f / sense_resistor_ohms;
}

// =============================================================================
// Slew rate and OCP delay
// =============================================================================

tps55289::expected<void> TPS55289::set_ocp_delay_us(uint32_t delay_us) {
    uint8_t bits;
    switch (delay_us) {
        case 128: bits = 0x00; break;
        case 3072: bits = 0x01; break;
        case 6144: bits = 0x02; break;
        case 12288: bits = 0x03; break;
        default:
            return make_error_code(ErrorCode::InvalidAddress);
    }
    
    return update_register_bits(TPS55289_REG_VOUT_SR, TPS55289_VOUT_SR_OCP_DELAY_MASK, bits << 4);
}

tps55289::expected<uint32_t> TPS55289::get_ocp_delay_us() {
    auto value_opt = read_register(TPS55289_REG_VOUT_SR);
    if (!value_opt) {
        return value_opt.error();
    }
    
    uint8_t bits = (value_opt.value() >> 4) & 0x03;
    
    switch (bits) {
        case 0x00: return 128;
        case 0x01: return 3072;
        case 0x02: return 6144;
        case 0x03: return 12288;
        default: return 128;  // Default
    }
}

tps55289::expected<void> TPS55289::set_slew_rate_mv_us(float slew_rate_mv_us) {
    uint8_t bits;
    if (slew_rate_mv_us <= 1.25f) {
        bits = 0x00;
    } else if (slew_rate_mv_us <= 2.5f) {
        bits = 0x01;
    } else if (slew_rate_mv_us <= 5.0f) {
        bits = 0x02;
    } else if (slew_rate_mv_us <= 10.0f) {
        bits = 0x03;
    } else {
        return make_error_code(ErrorCode::InvalidAddress);
    }
    
    return update_register_bits(TPS55289_REG_VOUT_SR, TPS55289_VOUT_SR_SLEW_MASK, bits);
}

tps55289::expected<float> TPS55289::get_slew_rate_mv_us() {
    auto value_opt = read_register(TPS55289_REG_VOUT_SR);
    if (!value_opt) {
        return value_opt.error();
    }
    
    uint8_t bits = value_opt.value() & 0x03;
    
    switch (bits) {
        case 0x00: return 1.25f;
        case 0x01: return 2.5f;
        case 0x02: return 5.0f;
        case 0x03: return 10.0f;
        default: return 1.25f;  // Default
    }
}

// =============================================================================
// Feedback configuration
// =============================================================================

tps55289::expected<void> TPS55289::set_feedback_mode(FeedbackMode mode) {
    uint8_t value = (mode == FeedbackMode::External) ? TPS55289_VOUT_FS_FB_MASK : 0x00;
    return update_register_bits(TPS55289_REG_VOUT_FS, TPS55289_VOUT_FS_FB_MASK, value);
}

FeedbackMode TPS55289::get_feedback_mode() const {
    // Read the VOUT_FS register to get the feedback mode
    uint8_t reg = read_register_const(TPS55289_REG_VOUT_FS);
    return (reg & TPS55289_VOUT_FS_FB_MASK) ? FeedbackMode::External : FeedbackMode::Internal;
}

tps55289::expected<void> TPS55289::set_internal_feedback_ratio(uint8_t bits) {
    if (bits > 3) {
        return make_error_code(ErrorCode::InvalidAddress);
    }

    return update_register_bits(TPS55289_REG_VOUT_FS, TPS55289_VOUT_FS_INTFB_MASK, bits);
}

tps55289::expected<float> TPS55289::get_internal_feedback_ratio() const {
    uint8_t reg = read_register_const(TPS55289_REG_VOUT_FS);
    uint8_t intfb = reg & TPS55289_VOUT_FS_INTFB_MASK;
    
    switch (intfb) {
        case TPS55289_INTFB_0P2256: return 0.2256f;
        case TPS55289_INTFB_0P1128: return 0.1128f;
        case TPS55289_INTFB_0P0752: return 0.0752f;
        case TPS55289_INTFB_0P0564: return 0.0564f;
        default: return 0.0564f;  // Default
    }
}

float TPS55289::get_active_feedback_ratio() const {
    uint8_t reg = read_register_const(TPS55289_REG_VOUT_FS);
    if ((reg & TPS55289_VOUT_FS_FB_MASK) != 0) {
        return external_feedback_ratio_;
    } else {
        // Use default value for internal feedback if register read fails
        uint8_t intfb = reg & TPS55289_VOUT_FS_INTFB_MASK;
        switch (intfb) {
            case TPS55289_INTFB_0P2256: return 0.2256f;
            case TPS55289_INTFB_0P1128: return 0.1128f;
            case TPS55289_INTFB_0P0752: return 0.0752f;
            case TPS55289_INTFB_0P0564: return 0.0564f;
            default: return 0.0564f;  // Default
        }
    }
}

uint16_t TPS55289::get_output_voltage_step_mv() const {
    float feedback_ratio = get_active_feedback_ratio();
    if (feedback_ratio <= 0) {
        return 2;
    }
    // Step size = 0.5645mV / feedback_ratio
    return static_cast<uint16_t>(0.5645f / feedback_ratio * 1000.0f);
}

// =============================================================================
// Cable droop compensation (CDC)
// =============================================================================

tps55289::expected<void> TPS55289::enable_fault_indication(const std::string& fault) {
    uint8_t mask;
    if (fault == "scp" || fault == "sc" || fault == "short") {
        mask = TPS55289_CDC_SC_MASK_MASK;
    } else if (fault == "ocp" || fault == "overcurrent") {
        mask = TPS55289_CDC_OCP_MASK_MASK;
    } else if (fault == "ovp" || fault == "overvoltage") {
        mask = TPS55289_CDC_OVP_MASK_MASK;
    } else {
        return make_error_code(ErrorCode::InvalidAddress);
    }

    return update_register_bits(TPS55289_REG_CDC, mask, mask);
}

tps55289::expected<void> TPS55289::disable_fault_indication(const std::string& fault) {
    uint8_t mask;
    if (fault == "scp" || fault == "sc" || fault == "short") {
        mask = TPS55289_CDC_SC_MASK_MASK;
    } else if (fault == "ocp" || fault == "overcurrent") {
        mask = TPS55289_CDC_OCP_MASK_MASK;
    } else if (fault == "ovp" || fault == "overvoltage") {
        mask = TPS55289_CDC_OVP_MASK_MASK;
    } else {
        return make_error_code(ErrorCode::InvalidAddress);
    }

    return update_register_bits(TPS55289_REG_CDC, mask, 0x00);
}

bool TPS55289::fault_indication_enabled(const std::string& fault) const {
    uint8_t reg = read_register_const(TPS55289_REG_CDC);
    if (fault == "scp" || fault == "sc" || fault == "short") {
        return (reg & TPS55289_CDC_SC_MASK_MASK) != 0;
    } else if (fault == "ocp" || fault == "overcurrent") {
        return (reg & TPS55289_CDC_OCP_MASK_MASK) != 0;
    } else if (fault == "ovp" || fault == "overvoltage") {
        return (reg & TPS55289_CDC_OVP_MASK_MASK) != 0;
    }
    return false;
}

tps55289::expected<void> TPS55289::set_cdc_option(bool external) {
    uint8_t value = external ? TPS55289_CDC_OPTION_MASK : 0x00;
    return update_register_bits(TPS55289_REG_CDC, TPS55289_CDC_OPTION_MASK, value);
}

bool TPS55289::get_cdc_option() const {
    uint8_t reg = read_register_const(TPS55289_REG_CDC);
    return (reg & TPS55289_CDC_OPTION_MASK) != 0;
}

tps55289::expected<void> TPS55289::set_cdc_compensation_v(float compensation_v) {
    if (compensation_v < 0.0f || compensation_v > 0.7f) {
        return make_error_code(ErrorCode::InvalidAddress);
    }
    
    uint8_t code = static_cast<uint8_t>(std::round(compensation_v / 0.1f));
    if (code > 7) {
        code = 7;
    }
    
    return update_register_bits(TPS55289_REG_CDC, TPS55289_CDC_SETTING_MASK, code);
}

tps55289::expected<float> TPS55289::get_cdc_compensation_v() {
    auto value_opt = read_register(TPS55289_REG_CDC);
    if (!value_opt) {
        return value_opt.error();
    }

    uint8_t code = value_opt.value() & TPS55289_CDC_SETTING_MASK;
    return code * 0.1f;
}

// =============================================================================
// Mode configuration
// =============================================================================

tps55289::expected<void> TPS55289::set_fsw_doubling(bool enabled) {
    uint8_t value = enabled ? TPS55289_MODE_FSWDBL_MASK : 0x00;
    return update_register_bits(TPS55289_REG_MODE, TPS55289_MODE_FSWDBL_MASK, value);
}

bool TPS55289::fsw_doubling_enabled() const {
    uint8_t reg = read_register_const(TPS55289_REG_MODE);
    return (reg & TPS55289_MODE_FSWDBL_MASK) != 0;
}

tps55289::expected<void> TPS55289::set_hiccup_mode(bool enabled) {
    uint8_t value = enabled ? TPS55289_MODE_HICCUP_MASK : 0x00;
    return update_register_bits(TPS55289_REG_MODE, TPS55289_MODE_HICCUP_MASK, value);
}

bool TPS55289::hiccup_mode_enabled() const {
    uint8_t reg = read_register_const(TPS55289_REG_MODE);
    return (reg & TPS55289_MODE_HICCUP_MASK) != 0;
}

tps55289::expected<void> TPS55289::set_vout_discharge(bool enabled) {
    uint8_t value = enabled ? TPS55289_MODE_DISCHG_MASK : 0x00;
    return update_register_bits(TPS55289_REG_MODE, TPS55289_MODE_DISCHG_MASK, value);
}

bool TPS55289::vout_discharge_enabled() const {
    uint8_t reg = read_register_const(TPS55289_REG_MODE);
    return (reg & TPS55289_MODE_DISCHG_MASK) != 0;
}

tps55289::expected<void> TPS55289::set_light_load_mode(bool fpwm) {
    uint8_t value = fpwm ? TPS55289_MODE_FPWM_MASK : 0x00;
    return update_register_bits(TPS55289_REG_MODE, TPS55289_MODE_FPWM_MASK, value);
}

bool TPS55289::light_load_mode_fpwm() const {
    uint8_t reg = read_register_const(TPS55289_REG_MODE);
    return (reg & TPS55289_MODE_FPWM_MASK) != 0;
}

// =============================================================================
// Status and faults
// =============================================================================

tps55289::expected<uint8_t> TPS55289::read_status() {
    return read_register(TPS55289_REG_STATUS);
}

bool TPS55289::has_fault() const {
    uint8_t reg = read_register_const(TPS55289_REG_STATUS);
    return (reg & (TPS55289_STATUS_SCP_MASK | TPS55289_STATUS_OCP_MASK | TPS55289_STATUS_OVP_MASK)) != 0;
}

bool TPS55289::has_scp() const {
    uint8_t reg = read_register_const(TPS55289_REG_STATUS);
    return (reg & TPS55289_STATUS_SCP_MASK) != 0;
}

bool TPS55289::has_ocp() const {
    uint8_t reg = read_register_const(TPS55289_REG_STATUS);
    return (reg & TPS55289_STATUS_OCP_MASK) != 0;
}

bool TPS55289::has_ovp() const {
    uint8_t reg = read_register_const(TPS55289_REG_STATUS);
    return (reg & TPS55289_STATUS_OVP_MASK) != 0;
}

OperatingMode TPS55289::get_operating_mode() const {
    uint8_t reg = read_register_const(TPS55289_REG_STATUS);
    uint8_t mode = reg & TPS55289_STATUS_MODE_MASK;
    
    switch (mode) {
        case 0x00: return OperatingMode::Boost;
        case 0x01: return OperatingMode::Buck;
        case 0x02: return OperatingMode::BuckBoost;
        default: return OperatingMode::Reserved;
    }
}

} // namespace tps55289
