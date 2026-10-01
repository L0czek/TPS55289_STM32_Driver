# TPS55289 Buck-Boost Regulator Driver

I2C driver for the Texas Instruments TPS55289 18-V, 4-A synchronous buck-boost regulator for STM32 microcontrollers.

[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![C++ Standard](https://img.shields.io/badge/C++17-blue.svg)](https://en.cppreference.com/w/cpp/17)

## Features

- Output voltage 0.5V to 18V with 0.5645mV resolution
- Programmable current limit
- Slew rate control (1.25-10.0 mV/µs)
- Configurable OCP delay (128µs-12.288ms)
- FPWM/PFM, hiccup mode, FSW doubling
- Fault monitoring (SCP, OCP, OVP)
- Cable droop compensation (CDC)

## Documentation

- [Library Documentation](library/README.md) - Integration and API reference
- [Header File](Core/Inc/tps55289.hpp) - Complete API reference

## Directory Structure

```
TPS55289_STM32_Driver/
├── library/              # CMake submodule integration
│   ├── CMakeLists.txt
│   └── README.md
├── Core/
│   ├── Inc/             # Public headers
│   │   └── tps55289.hpp
│   └── Src/             # Implementation
│       ├── tps55289.cpp
│       └── tps55289_expected.hpp
├── examples/            # Example code
├── tests/               # Unit tests
└── README.md
```

## Quick Start

```cpp
#include "tps55289.hpp"

tps55289::TPS55289 regulator(&hi2c, 0x74);
regulator.set_output_voltage(5.0f);
regulator.enable();
```

## Building

```bash
mkdir build && cd build
cmake ..
make
make test
```

## Unit Tests

Run tests from build directory:
```bash
./test_tps55289
```

**Note**: Tests use stub I2C implementation. Some tests may fail due to mock limitations.

## License

MIT License
