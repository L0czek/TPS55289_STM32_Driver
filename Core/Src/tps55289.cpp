#include "tps55289.hpp"
#include <algorithm>
#include <cmath>

namespace tps55289 {

namespace {

std::unexpected<std::error_code> fail(ErrorCode ec) {
    return std::unexpected(make_error_code(ec));
}

} // namespace

// =============================================================================
// TPS55289 Implementation
// =============================================================================

TPS55289::TPS55289(I2C_HandleTypeDef* hi2c, uint8_t address)
    : hi2c_(hi2c), address_(address), sense_resistor_ohms_(DEFAULT_SENSE_RESISTOR),
      external_feedback_ratio_(DEFAULT_EXTERNAL_FB_RATIO),
      feedback_ratio_(DEFAULT_INTERNAL_FB_RATIO),
      max_output_voltage_(get_vref_max_mv() / 1000.0f / DEFAULT_INTERNAL_FB_RATIO),
      initialized_(false)
{
}

TPS55289::~TPS55289() = default;

std::error_code TPS55289::convertHALStatus(HAL_StatusTypeDef status) {
    switch (status) {
        case HAL_OK:      return make_error_code(ErrorCode::None);
        case HAL_TIMEOUT: return make_error_code(ErrorCode::Timeout);
        case HAL_BUSY:    return make_error_code(ErrorCode::I2CError);
        case HAL_ERROR:
        default:          return make_error_code(ErrorCode::HALError);
    }
}

float TPS55289::intfb_bits_to_ratio(uint8_t bits) {
    switch (bits & TPS55289_VOUT_FS_INTFB_MASK) {
        case TPS55289_INTFB_0P2256: return 0.2256f;
        case TPS55289_INTFB_0P1128: return 0.1128f;
        case TPS55289_INTFB_0P0752: return 0.0752f;
        case TPS55289_INTFB_0P0564:
        default:                    return 0.0564f;
    }
}

// =============================================================================
// Low-level I2C operations
// =============================================================================

expected<void> TPS55289::write_register(uint8_t register_addr, uint8_t value) {
    if (!hi2c_) {
        return fail(ErrorCode::HALError);
    }
    if (register_addr > TPS55289_REG_STATUS) {
        return fail(ErrorCode::InvalidRegister);
    }
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(hi2c_, address_ << 1, register_addr, I2C_MEMADD_SIZE_8BIT,
                                                 &value, 1, TPS55289_TIMEOUT_MS);
    if (status != HAL_OK) {
        return std::unexpected(convertHALStatus(status));
    }
    return {};
}

expected<uint8_t> TPS55289::read_register(uint8_t register_addr) const {
    if (!hi2c_) {
        return fail(ErrorCode::HALError);
    }
    if (register_addr > TPS55289_REG_STATUS) {
        return fail(ErrorCode::InvalidRegister);
    }
    uint8_t value = 0;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c_, address_ << 1, register_addr, I2C_MEMADD_SIZE_8BIT,
                                                &value, 1, TPS55289_TIMEOUT_MS);
    if (status != HAL_OK) {
        return std::unexpected(convertHALStatus(status));
    }
    return value;
}

expected<void> TPS55289::update_register_bits(uint8_t register_addr, uint8_t mask, uint8_t value) {
    auto current = read_register(register_addr);
    if (!current) {
        return std::unexpected(current.error());
    }
    const uint8_t next = static_cast<uint8_t>((*current & ~mask) | (value & mask));
    return write_register(register_addr, next);
}

expected<bool> TPS55289::read_flag(uint8_t register_addr, uint8_t mask) const {
    auto reg = read_register(register_addr);
    if (!reg) {
        return std::unexpected(reg.error());
    }
    return (*reg & mask) != 0;
}

// =============================================================================
// Device control
// =============================================================================

expected<void> TPS55289::initialize() {
    // Read-only probe: never write MODE here, its reset value (HICCUP=1) must survive.
    auto present = verify_device_present();
    if (!present) {
        return std::unexpected(present.error());
    }
    if (auto r = refresh_feedback_ratio(); !r) {
        return r;
    }
    initialized_ = true;
    return {};
}

expected<bool> TPS55289::verify_device_present() const {
    auto mode = read_register(TPS55289_REG_MODE);
    if (!mode) {
        return std::unexpected(mode.error());
    }
    return true;
}

expected<void> TPS55289::refresh_feedback_ratio() {
    auto vout_fs = read_register(TPS55289_REG_VOUT_FS);
    if (!vout_fs) {
        return std::unexpected(vout_fs.error());
    }
    feedback_ratio_ = (*vout_fs & TPS55289_VOUT_FS_FB_MASK) ? external_feedback_ratio_
                                                            : intfb_bits_to_ratio(*vout_fs);
    return {};
}

expected<void> TPS55289::enable() {
    return update_register_bits(TPS55289_REG_MODE, TPS55289_MODE_OE_MASK, TPS55289_MODE_OE_MASK);
}

expected<void> TPS55289::disable() {
    return update_register_bits(TPS55289_REG_MODE, TPS55289_MODE_OE_MASK, 0x00);
}

expected<bool> TPS55289::output_enabled() const {
    return read_flag(TPS55289_REG_MODE, TPS55289_MODE_OE_MASK);
}

// =============================================================================
// Reference voltage and output voltage
// =============================================================================

uint16_t TPS55289::vref_mv_to_code(float vref_mv) {
    const float code = std::round((vref_mv - get_vref_min_mv()) / get_vref_lsb_mv());
    if (!(code > 0.0f)) {  // also catches NaN
        return 0;
    }
    return static_cast<uint16_t>(std::min(code, static_cast<float>(get_vref_max_code())));
}

float TPS55289::ref_code_to_vref_mv(uint16_t code) {
    code = std::min<uint16_t>(code & 0x7FF, get_vref_max_code());
    return get_vref_min_mv() + code * get_vref_lsb_mv();
}

expected<void> TPS55289::write_ref_code(uint16_t code) {
    if (code > get_vref_max_code()) {
        return fail(ErrorCode::InvalidParameter);
    }
    // Datasheet 7.6.1: write 00h first, then 01h; the new REF takes effect on the 01h write.
    if (auto r = write_register(TPS55289_REG_REF_LSB, code & 0xFF); !r) {
        return r;
    }
    return write_register(TPS55289_REG_REF_MSB, (code >> 8) & 0x07);
}

expected<uint16_t> TPS55289::read_ref_code() const {
    auto lsb = read_register(TPS55289_REG_REF_LSB);
    if (!lsb) {
        return std::unexpected(lsb.error());
    }
    auto msb = read_register(TPS55289_REG_REF_MSB);
    if (!msb) {
        return std::unexpected(msb.error());
    }
    return static_cast<uint16_t>(((*msb & 0x07) << 8) | *lsb);
}

expected<void> TPS55289::set_reference_voltage_mv(float vref_mv) {
    if (!(vref_mv >= get_vref_min_mv() && vref_mv <= get_vref_max_mv())) {
        return fail(ErrorCode::InvalidParameter);
    }
    return write_ref_code(vref_mv_to_code(vref_mv));
}

expected<float> TPS55289::get_reference_voltage_mv() const {
    auto code = read_ref_code();
    if (!code) {
        return std::unexpected(code.error());
    }
    return ref_code_to_vref_mv(*code);
}

float TPS55289::get_min_output_voltage() const {
    return std::max(get_vout_min_v(), get_vref_min_mv() / 1000.0f / feedback_ratio_);
}

float TPS55289::get_max_output_voltage() const {
    return std::min(max_output_voltage_, get_vref_max_mv() / 1000.0f / feedback_ratio_);
}

expected<void> TPS55289::set_output_voltage_limit(float max_voltage) {
    if (!(max_voltage >= get_vout_min_v())) {
        return fail(ErrorCode::InvalidParameter);
    }
    max_output_voltage_ = max_voltage;
    return {};
}

expected<float> TPS55289::set_output_voltage(float voltage) {
    if (!initialized_) {
        // The feedback ratio is unknown until initialize() has read VOUT_FS.
        return fail(ErrorCode::NotInitialized);
    }
    if (!(voltage >= get_min_output_voltage() && voltage <= get_max_output_voltage())) {
        return fail(ErrorCode::InvalidParameter);
    }

    const uint16_t code = vref_mv_to_code(voltage * feedback_ratio_ * 1000.0f);
    if (auto r = write_ref_code(code); !r) {
        return std::unexpected(r.error());
    }
    // Report what was programmed (computed, not read back: the write succeeded).
    return ref_code_to_vref_mv(code) / 1000.0f / feedback_ratio_;
}

expected<float> TPS55289::get_output_voltage() const {
    if (!initialized_) {
        return fail(ErrorCode::NotInitialized);
    }
    auto vref = get_reference_voltage_mv();
    if (!vref) {
        return std::unexpected(vref.error());
    }
    return *vref / 1000.0f / feedback_ratio_;
}

// =============================================================================
// Current limit
// =============================================================================

expected<void> TPS55289::enable_current_limit() {
    return update_register_bits(TPS55289_REG_IOUT_LIMIT, TPS55289_IOUT_LIMIT_EN_MASK, TPS55289_IOUT_LIMIT_EN_MASK);
}

expected<void> TPS55289::disable_current_limit() {
    return update_register_bits(TPS55289_REG_IOUT_LIMIT, TPS55289_IOUT_LIMIT_EN_MASK, 0x00);
}

expected<bool> TPS55289::current_limit_enabled() const {
    return read_flag(TPS55289_REG_IOUT_LIMIT, TPS55289_IOUT_LIMIT_EN_MASK);
}

expected<void> TPS55289::set_current_limit_voltage_mv(float limit_mv) {
    // Current-sense voltage limit: 0.5 mV/LSB, 0 - 63.5 mV
    if (!(limit_mv >= 0.0f && limit_mv <= 63.5f)) {
        return fail(ErrorCode::InvalidParameter);
    }
    const uint8_t code = static_cast<uint8_t>(std::min(std::lround(limit_mv / 0.5f), 0x7FL));
    return update_register_bits(TPS55289_REG_IOUT_LIMIT, TPS55289_IOUT_LIMIT_SETTING_MASK, code);
}

expected<float> TPS55289::get_current_limit_voltage_mv() const {
    auto reg = read_register(TPS55289_REG_IOUT_LIMIT);
    if (!reg) {
        return std::unexpected(reg.error());
    }
    return (*reg & TPS55289_IOUT_LIMIT_SETTING_MASK) * 0.5f;
}

expected<void> TPS55289::set_output_current_limit(float current_a, float sense_resistor_ohms) {
    if (sense_resistor_ohms <= 0.0f) {
        sense_resistor_ohms = sense_resistor_ohms_;
    }
    if (!(current_a >= 0.0f) || !(sense_resistor_ohms > 0.0f)) {
        return fail(ErrorCode::InvalidParameter);
    }
    return set_current_limit_voltage_mv(current_a * sense_resistor_ohms * 1000.0f);
}

expected<float> TPS55289::get_output_current_limit(float sense_resistor_ohms) const {
    if (sense_resistor_ohms <= 0.0f) {
        sense_resistor_ohms = sense_resistor_ohms_;
    }
    if (!(sense_resistor_ohms > 0.0f)) {
        return fail(ErrorCode::InvalidParameter);
    }
    auto limit_mv = get_current_limit_voltage_mv();
    if (!limit_mv) {
        return std::unexpected(limit_mv.error());
    }
    return *limit_mv / 1000.0f / sense_resistor_ohms;
}

expected<void> TPS55289::set_sense_resistor_ohms(float ohms) {
    if (!(ohms > 0.0f)) {
        return fail(ErrorCode::InvalidParameter);
    }
    sense_resistor_ohms_ = ohms;
    return {};
}

// =============================================================================
// Slew rate and OCP delay
// =============================================================================

expected<void> TPS55289::set_ocp_delay_us(uint32_t delay_us) {
    uint8_t bits;
    switch (delay_us) {
        case 128:   bits = TPS55289_OCP_DELAY_128US;   break;
        case 3072:  bits = TPS55289_OCP_DELAY_3072US;  break;
        case 6144:  bits = TPS55289_OCP_DELAY_6144US;  break;
        case 12288: bits = TPS55289_OCP_DELAY_12288US; break;
        default:
            return fail(ErrorCode::InvalidParameter);
    }
    return update_register_bits(TPS55289_REG_VOUT_SR, TPS55289_VOUT_SR_OCP_DELAY_MASK, bits << 4);
}

expected<uint32_t> TPS55289::get_ocp_delay_us() const {
    auto reg = read_register(TPS55289_REG_VOUT_SR);
    if (!reg) {
        return std::unexpected(reg.error());
    }
    static constexpr uint32_t delays[4] = {128, 3072, 6144, 12288};
    return delays[(*reg >> 4) & 0x03];
}

expected<void> TPS55289::set_slew_rate_mv_us(float slew_rate_mv_us) {
    // Never pick a faster slew rate than requested.
    uint8_t bits;
    if (!(slew_rate_mv_us >= 1.25f)) {
        return fail(ErrorCode::InvalidParameter);
    } else if (slew_rate_mv_us < 2.5f) {
        bits = TPS55289_SLEW_RATE_1P25MV;
    } else if (slew_rate_mv_us < 5.0f) {
        bits = TPS55289_SLEW_RATE_2P5MV;
    } else if (slew_rate_mv_us < 10.0f) {
        bits = TPS55289_SLEW_RATE_5P0MV;
    } else {
        bits = TPS55289_SLEW_RATE_10P0MV;
    }
    return update_register_bits(TPS55289_REG_VOUT_SR, TPS55289_VOUT_SR_SLEW_MASK, bits);
}

expected<float> TPS55289::get_slew_rate_mv_us() const {
    auto reg = read_register(TPS55289_REG_VOUT_SR);
    if (!reg) {
        return std::unexpected(reg.error());
    }
    static constexpr float rates[4] = {1.25f, 2.5f, 5.0f, 10.0f};
    return rates[*reg & TPS55289_VOUT_SR_SLEW_MASK];
}

// =============================================================================
// Feedback configuration
// =============================================================================

expected<void> TPS55289::set_feedback_mode(FeedbackMode mode) {
    const uint8_t value = (mode == FeedbackMode::External) ? TPS55289_VOUT_FS_FB_MASK : 0x00;
    if (auto r = update_register_bits(TPS55289_REG_VOUT_FS, TPS55289_VOUT_FS_FB_MASK, value); !r) {
        return r;
    }
    return refresh_feedback_ratio();
}

expected<FeedbackMode> TPS55289::get_feedback_mode() const {
    auto external = read_flag(TPS55289_REG_VOUT_FS, TPS55289_VOUT_FS_FB_MASK);
    if (!external) {
        return std::unexpected(external.error());
    }
    return *external ? FeedbackMode::External : FeedbackMode::Internal;
}

expected<void> TPS55289::set_internal_feedback_ratio(uint8_t bits) {
    if (bits > 3) {
        return fail(ErrorCode::InvalidParameter);
    }
    if (auto r = update_register_bits(TPS55289_REG_VOUT_FS, TPS55289_VOUT_FS_INTFB_MASK, bits); !r) {
        return r;
    }
    return refresh_feedback_ratio();
}

expected<float> TPS55289::get_internal_feedback_ratio() const {
    auto reg = read_register(TPS55289_REG_VOUT_FS);
    if (!reg) {
        return std::unexpected(reg.error());
    }
    return intfb_bits_to_ratio(*reg);
}

expected<void> TPS55289::set_external_feedback_ratio(float ratio) {
    if (!(ratio > 0.0f && ratio <= 1.0f)) {
        return fail(ErrorCode::InvalidParameter);
    }
    external_feedback_ratio_ = ratio;
    if (initialized_) {
        return refresh_feedback_ratio();
    }
    return {};
}

float TPS55289::get_output_voltage_step_mv() const {
    return get_vref_lsb_mv() / feedback_ratio_;  // e.g. 0.5645 / 0.0564 = 10 mV
}

// =============================================================================
// Cable droop compensation / fault indication (CDC, 05h)
// =============================================================================

expected<void> TPS55289::set_fault_indication(FaultIndication fault, bool enabled) {
    const uint8_t mask = static_cast<uint8_t>(fault);
    return update_register_bits(TPS55289_REG_CDC, mask, enabled ? mask : 0x00);
}

expected<bool> TPS55289::fault_indication_enabled(FaultIndication fault) const {
    return read_flag(TPS55289_REG_CDC, static_cast<uint8_t>(fault));
}

expected<void> TPS55289::set_cdc_option(bool external) {
    return update_register_bits(TPS55289_REG_CDC, TPS55289_CDC_OPTION_MASK, external ? TPS55289_CDC_OPTION_MASK : 0x00);
}

expected<bool> TPS55289::get_cdc_option() const {
    return read_flag(TPS55289_REG_CDC, TPS55289_CDC_OPTION_MASK);
}

expected<void> TPS55289::set_cdc_compensation_v(float compensation_v) {
    if (!(compensation_v >= 0.0f && compensation_v <= 0.7f)) {
        return fail(ErrorCode::InvalidParameter);
    }
    const uint8_t code = static_cast<uint8_t>(std::min(std::lround(compensation_v / 0.1f), 7L));
    return update_register_bits(TPS55289_REG_CDC, TPS55289_CDC_SETTING_MASK, code);
}

expected<float> TPS55289::get_cdc_compensation_v() const {
    auto reg = read_register(TPS55289_REG_CDC);
    if (!reg) {
        return std::unexpected(reg.error());
    }
    return (*reg & TPS55289_CDC_SETTING_MASK) * 0.1f;
}

// =============================================================================
// Mode configuration (06h)
// =============================================================================

expected<void> TPS55289::set_fsw_doubling(bool enabled) {
    return update_register_bits(TPS55289_REG_MODE, TPS55289_MODE_FSWDBL_MASK, enabled ? TPS55289_MODE_FSWDBL_MASK : 0x00);
}

expected<bool> TPS55289::fsw_doubling_enabled() const {
    return read_flag(TPS55289_REG_MODE, TPS55289_MODE_FSWDBL_MASK);
}

expected<void> TPS55289::set_hiccup_mode(bool enabled) {
    return update_register_bits(TPS55289_REG_MODE, TPS55289_MODE_HICCUP_MASK, enabled ? TPS55289_MODE_HICCUP_MASK : 0x00);
}

expected<bool> TPS55289::hiccup_mode_enabled() const {
    return read_flag(TPS55289_REG_MODE, TPS55289_MODE_HICCUP_MASK);
}

expected<void> TPS55289::set_vout_discharge(bool enabled) {
    return update_register_bits(TPS55289_REG_MODE, TPS55289_MODE_DISCHG_MASK, enabled ? TPS55289_MODE_DISCHG_MASK : 0x00);
}

expected<bool> TPS55289::vout_discharge_enabled() const {
    return read_flag(TPS55289_REG_MODE, TPS55289_MODE_DISCHG_MASK);
}

expected<void> TPS55289::set_light_load_mode(bool fpwm) {
    return update_register_bits(TPS55289_REG_MODE, TPS55289_MODE_FPWM_MASK, fpwm ? TPS55289_MODE_FPWM_MASK : 0x00);
}

expected<bool> TPS55289::light_load_mode_fpwm() const {
    return read_flag(TPS55289_REG_MODE, TPS55289_MODE_FPWM_MASK);
}

// =============================================================================
// Status and faults
// =============================================================================

expected<Status> TPS55289::read_status() const {
    auto reg = read_register(TPS55289_REG_STATUS);
    if (!reg) {
        return std::unexpected(reg.error());
    }
    Status s{};
    s.scp = (*reg & TPS55289_STATUS_SCP_MASK) != 0;
    s.ocp = (*reg & TPS55289_STATUS_OCP_MASK) != 0;
    s.ovp = (*reg & TPS55289_STATUS_OVP_MASK) != 0;
    switch (*reg & TPS55289_STATUS_MODE_MASK) {
        case 0x00: s.mode = OperatingMode::Boost;     break;
        case 0x01: s.mode = OperatingMode::Buck;      break;
        case 0x02: s.mode = OperatingMode::BuckBoost; break;
        default:   s.mode = OperatingMode::Reserved;  break;
    }
    return s;
}

} // namespace tps55289
