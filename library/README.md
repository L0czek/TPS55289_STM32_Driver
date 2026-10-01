# TPS55289 Buck-Boost Regulator Driver

I2C driver for the TI TPS55289 18-V, 4-A同步 buck-boost regulator for STM32 microcontrollers.

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

## API

See `../Core/Inc/tps55289.hpp` for the complete API reference.

## Example Usage

```cpp
#include "tps55289.hpp"

// Initialize driver with I2C handle
TPS55289::Regulator regulator(&hi2c, 0x60);  // I2C address 0x60

// Check if device is present
if (auto result = regulator.probe(); result.has_value()) {
    // Device is ready
}

// Set output voltage (in mV)
regulator.set_output_voltage(5000);  // 5.0V

// Enable the regulator
regulator.enable();

// Read output voltage
if (auto voltage = regulator.get_output_voltage(); voltage.has_value()) {
    // voltage.value() returns voltage in mV
}
```

## I2C Address Configuration

The TPS55289 supports multiple I2C addresses based on pin strapping. Default is `0x60`.

## Features

- Output voltage configuration (0.5V to 18V)
- Enable/disable control
- Output voltage monitoring
- Fault status reading
- Configuration register access

## Register Map

| Register | Name | Description |
|----------|------|-------------|
| 0x00 | VOUT_TARGET | Target output voltage |
| 0x01 | CONFIG | Configuration register |
| 0x02 | FAULT | Fault status register |
| 0x03 | STATUS | Operating status register |
| 0x04 | VOUT meas | Measured output voltage |
| 0x05 | IOUT | Measured output current |
