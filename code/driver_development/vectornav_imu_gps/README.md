# VectorNav IMU/GPS Driver

This folder contains the driver implementation for the VectorNav VN-310 Dual Antenna GNSS/INS device.

## Overview

The VectorNav VN-310 is a high-performance Inertial Navigation System (INS) that combines dual GNSS receivers with an industrial-grade IMU. This driver provides a hardware abstraction layer for configuring and reading data from the device.

### Features
- UART communication interface with binary protocol support
- Configurable output data rates up to 800Hz
- IMU Measurements:
  - Angular rates (°/s)
  - Linear acceleration (m/s²)
  - Magnetic field (Gauss)
- GPS Data:
  - Position (LLA)
  - Velocity (NED)
  - Dual antenna heading
- Navigation Solutions:
  - Attitude (heading, pitch, roll)
  - Heading accuracy
  - Navigation status

## Dependencies
- UART/Serial communication driver
- Platform timing functions

## Usage Example
```c
// Initialize VN-310
vn310_config_t config = {
    .uart_handle = &huart2,
    .baud_rate = 921600,
    .async_output_rate = 100  // 100 Hz
};
vn310_state_t state;
vn310_init(&state, &config);

// Read IMU data
vn310_imu_data_t imu_data;
vn310_read_imu_measurements(&state, &imu_data);
```