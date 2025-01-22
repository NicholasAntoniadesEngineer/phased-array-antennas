# HMC1119 Digital Attenuator Driver

A driver implementation for the Analog Devices HMC1119 7-bit digital attenuator.

## Features

- 31.75 dB attenuation range with 0.25 dB resolution
- Supports both SPI and parallel interface modes
- Configurable insertion loss compensation
- Thread-safe operation
- Comprehensive error handling

## Hardware Interface Support

- **SPI Mode**: Full SPI interface support with chip select control
- **Parallel Mode**: 7-bit parallel data interface with latch enable (LE) control

## API Functions

- `hmc1119_init()`: Initialize the attenuator driver
- `hmc1119_set()`: Set attenuation using raw bit values (0-127)
- `hmc1119_set_db()`: Set attenuation in dB with insertion loss compensation
- `hmc1119_latch()`: Latch the current attenuation value (parallel mode)
- `hmc1119_convert_attenuation_db_to_code()`: Convert dB values to attenuator codes

## Technical Specifications

- Resolution: 7-bit (128 steps)
- Maximum Attenuation: 31.75 dB
- Step Size: 0.25 dB
- Minimum Timing Requirements:
  - LE Pulse Width: 10ns
  - LE Pulse Spacing: 630ns
  - Data Hold Time: 10ns
