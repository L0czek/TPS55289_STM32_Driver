# TPS55289 Buck-Boost Regulator Driver

I2C driver for the Texas Instruments TPS55289 18-V, 4-A synchronous buck-boost regulator with I2C control for STM32 microcontrollers.

[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![C++ Standard](https://img.shields.io/badge/C++17-blue.svg)](https://en.cppreference.com/w/cpp/17)

## Features

- **Output Voltage Control**: Configure output from 0.5V to 18V with 0.5645mV resolution
- **Current Limiting**: Programmable current limit with sense resistor monitoring
- **Slew Rate Control**: Selectable slew rates (1.25, 2.5, 5.0, 10.0 mV/µs)
- **OCP Delay**: Configurable overcurrent protection delay (128µs to 12.288ms)
- **Feedback Mode**: Internal or external feedback configuration
- **Mode Control**: FPWM/PFM, hiccup mode, FSW doubling, VOUT discharge
- **Fault Monitoring**: Short circuit, overcurrent, overvoltage detection
- **Cable Droop Compensation**: Adjustable CDC for load-line regulation

## Integration as Submodule

Add this driver as a Git submodule to your project:

```bash
git submodule add https://github.com/yourorg/TPS55289_STM32_Driver.git vendor/TPS55289
```

In your project's `CMakeLists.txt`:

```cmake
# Add the driver as a subdirectory
add_subdirectory(vendor/TPS55289/library)

# Link the library to your target
target_link_libraries(your_target PRIVATE tps55289)
```

## Requirements

- C++17 compiler (for `std::expected`)
- STM32 HAL library (must be defined in parent project as `stm32_hal` target)
- I2C peripheral support

## Quick Start

```cpp
#include "tps55289.hpp"

// Initialize driver with I2C handle and address
tps55289::TPS55289 regulator(&hi2c, 0x74);  // Default address (MODE pin high)

// Configure output voltage to 5.0V
if (auto result = regulator.set_output_voltage(5.0f); result.has_value()) {
    // Output voltage set successfully
}

// Enable the regulator
regulator.enable();

// Read back the actual output voltage
if (auto voltage = regulator.get_output_voltage(); voltage.has_value()) {
    float actual_voltage = voltage.value();  // e.g., 4.998V
}

// Enable current limit at 3A with 10mΩ sense resistor
regulator.set_output_current_limit(3.0f, 0.01f);
```

## I2C Address Configuration

The TPS55289 address is determined by the MODE pin strapping:

| MODE Pin | I2C Address | Address Constant |
|----------|-------------|------------------|
| Low (GND) | 0x75 | `TPS55289_ADDRESS_MODE_LOW` |
| High (VCC) | 0x74 | `TPS55289_ADDRESS_MODE_HIGH` (default) |

## Register Map

| Address | Name | Description | R/W |
|---------|------|-------------|-----|
| 0x00 | REF_LSB | REF[7:0] - LSB of reference voltage | R/W |
| 0x01 | REF_MSB | REF[10:8] - MSB of reference voltage | R/W |
| 0x02 | IOUT_LIMIT | IOUT limit setting | R/W |
| 0x03 | VOUT_SR | Slew rate and OCP delay | R/W |
| 0x04 | VOUT_FS | Full-scale feedback configuration | R/W |
| 0x05 | CDC | Cable droop compensation | R/W |
| 0x06 | MODE | Device operating mode | R/W |
| 0x07 | STATUS | Device status and fault flags | R/O |

## API Reference

### Device Control

| Method | Description |
|--------|-------------|
| `enable()` | Enable the regulator output |
| `disable()` | Disable the regulator output |
| `output_enabled()` | Check if output is enabled |
| `verify_device_present()` | Verify device is on I2C bus |

### Output Voltage

| Method | Description |
|--------|-------------|
| `set_output_voltage(float)` | Set output voltage in volts |
| `get_output_voltage()` | Get current output voltage |
| `set_reference_voltage_mv(float)` | Set reference voltage directly |
| `get_reference_voltage_mv()` | Get reference voltage |

### Current Limit

| Method | Description |
|--------|-------------|
| `set_output_current_limit(float, float)` | Set current limit (amps, sense resistor) |
| `get_output_current_limit(float)` | Get current limit |
| `enable_current_limit()` | Enable current limit feature |
| `disable_current_limit()` | Disable current limit feature |

### Slew Rate & OCP Delay

| Method | Description |
|--------|-------------|
| `set_slew_rate_mv_us(float)` | Set slew rate (1.25, 2.5, 5.0, 10.0) |
| `get_slew_rate_mv_us()` | Get current slew rate |
| `set_ocp_delay_us(uint32_t)` | Set OCP delay (128, 3072, 6144, 12288) |
| `get_ocp_delay_us()` | Get OCP delay |

### Feedback Configuration

| Method | Description |
|--------|-------------|
| `set_feedback_mode(FeedbackMode)` | Set internal/external feedback |
| `get_feedback_mode()` | Get current feedback mode |
| `set_internal_feedback_ratio(uint8_t)` | Set internal feedback ratio (0-3) |
| `set_external_feedback_ratio(float)` | Set external feedback ratio |

### Fault Indication (CDC)

| Method | Description |
|--------|-------------|
| `enable_fault_indication(string)` | Enable SCP/OCP/OVP indication |
| `disable_fault_indication(string)` | Disable fault indication |
| `set_cdc_compensation_v(float)` | Set CDC voltage (0-0.7V) |
| `get_cdc_compensation_v()` | Get CDC voltage |

### Mode Configuration

| Method | Description |
|--------|-------------|
| `set_fsw_doubling(bool)` | Enable FSW doubling |
| `set_hiccup_mode(bool)` | Enable hiccup mode |
| `set_vout_discharge(bool)` | Enable VOUT discharge |
| `set_light_load_mode(bool)` | Set FPWM (true) or PFM (false) mode |

### Status & Faults

| Method | Description |
|--------|-------------|
| `read_status()` | Read status register |
| `has_fault()` | Check for any fault |
| `has_scp()` | Check for short circuit fault |
| `has_ocp()` | Check for overcurrent fault |
| `has_ovp()` | Check for overvoltage fault |
| `get_operating_mode()` | Get operating mode (Buck/Boost/BuckBoost) |

## Static Configuration Values

| Method | Value | Description |
|--------|-------|-------------|
| `get_vref_min_mv()` | 45.0 | Minimum reference voltage (mV) |
| `get_vref_lsb_mv()` | 0.5645 | Reference LSB size (mV) |
| `get_vref_max_code()` | 0x7FE | Maximum REF code |

## Complete Example

```cpp
#include "tps55289.hpp"
#include <iostream>

int main() {
    // Initialize TPS55289 driver
    tps55289::TPS55289 regulator(&hi2c1, 0x74);
    
    // Verify device is present
    if (auto result = regulator.verify_device_present(); !result.has_value()) {
        // Handle error
        return -1;
    }
    
    // Configure output to 12V
    if (auto result = regulator.set_output_voltage(12.0f); !result.has_value()) {
        // Handle error
        return -1;
    }
    
    // Set current limit to 4A with 5mΩ sense resistor
    regulator.set_output_current_limit(4.0f, 0.005f);
    
    // Set slew rate to 5.0 mV/µs
    regulator.set_slew_rate_mv_us(5.0f);
    
    // Enable OCP with 3072µs delay
    regulator.set_ocp_delay_us(3072);
    
    // Enable the regulator
    regulator.enable();
    
    // Main loop
    while (true) {
        // Read output voltage
        if (auto voltage = regulator.get_output_voltage(); voltage.has_value()) {
            std::cout << "Output Voltage: " << voltage.value() << "V" << std::endl;
        }
        
        // Check for faults
        if (regulator.has_fault()) {
            if (regulator.has_scp()) std::cout << "SCP fault!" << std::endl;
            if (regulator.has_ocp()) std::cout << "OCP fault!" << std::endl;
            if (regulator.has_ovp()) std::cout << "OVP fault!" << std::endl;
        }
        
        HAL_Delay(1000);
    }
}
```

## Error Handling

The driver uses `std::expected` for error handling:

```cpp
auto result = regulator.set_output_voltage(5.0f);
if (!result.has_value()) {
    std::error_code ec = result.error();
    std::cout << "Error: " << ec.message() << std::endl;
    
    if (ec.category() == tps55289::get_error_category()) {
        switch (static_cast<tps55289::ErrorCode>(ec.value())) {
            case tps55289::ErrorCode::Timeout:
                // Handle timeout
                break;
            case tps55289::ErrorCode::HALError:
                // Handle HAL error
                break;
            // ... other error codes
        }
    }
}
```

## Troubleshooting

### Device Not Detected
- Verify I2C address matches MODE pin configuration
- Check I2C pull-up resistors (typically 4.7kΩ)
- Verify I2C clock speed (standard mode: 100kHz, fast mode: 400kHz)

### Output Voltage Incorrect
- Verify sense resistor value matches configuration
- Check feedback configuration (internal vs external)
- Ensure REF code is within valid range (0-0x7FE)

### Fault Conditions
- SCP: Check for short circuit on output
- OCP: Reduce load current or increase current limit
- OVP: Verify input voltage and load conditions

## License

MIT License - see LICENSE file for details
