# Shift Register Drivers

This folder contains driver implementations for two commonly used shift register ICs:

- **74HC595**: 8-bit serial-in, parallel-out shift register
- **74HC165**: 8-bit parallel-in, serial-out shift register

## Overview

These drivers provide a hardware abstraction layer for controlling the shift registers in embedded applications. They handle GPIO initialization, data shifting, and control signal management.

### 74HC595 Features
- Serial data input
- 8-bit parallel output
- Output enable control
- Storage register control
- Shift register clearing

### 74HC165 Features
- 8-bit parallel data input
- Serial data output
- Parallel load control
- Output enable control

## Usage Example
```c
// Initialize 74HC595
74HC595pw_config_t config_595 = {
    .DS_pin = &DS_GPIO,    // Serial data input
    .OE_pin = &OE_GPIO,    // Output enable
    .SHCP_pin = &SHCP_GPIO, // Shift register clock
    .STCP_pin = &STCP_GPIO  // Storage register clock
};
74HC595pw_state_t state_595;
74hc595pw_init(&state_595, &config_595);

// Output data
74hc595pw_output_parallel(&state_595, 0xAA);
```
