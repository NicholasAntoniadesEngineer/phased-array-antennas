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

## Application Layer
The application layer (`vn310_app`) provides a higher-level interface for interacting with the VN-310 device. It includes:

### Key Features
- Message parsing for both ASCII NMEA-style messages and binary protocol
- Pose data handling (yaw, pitch, roll, latitude, longitude)
- CLI interface for device configuration and control
- Power management and GPIO control for dual antenna setup
- Configurable output rates (1, 2, 4, 5, 10, 20, 25, 40, 50, 100, 200 Hz)

### CLI Commands
The application provides command-line interface commands for:
- Setting output frequency
- Power control
- Device configuration
- Register reading/writing
- Message streaming control
- INS status monitoring

The application layer automatically handles:
- Message parsing and pose updates
- INS status monitoring
- Data streaming management
- Command response handling
