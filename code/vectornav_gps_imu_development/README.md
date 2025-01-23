# VectorNav VN310 GPS/IMU Driver

## Overview
Driver and application interface for the VectorNav VN310 GPS-Aided IMU. Provides functionality for sensor configuration, data acquisition, and pose estimation.

## Features
- Full VN310 sensor configuration and control
- Binary and ASCII message parsing
- Real-time pose estimation (position, orientation, angular rates)
- Command-line interface for sensor interaction
- Configurable output data rates (1-200 Hz)
- Support for dual antenna GPS configurations

## Project Structure
### Source Files (`src/`)
- `vn310_applet.c` - Main application controller managing device state, message handling, and pose updates
- `vn310_cli.c` - Command-line interface implementation for device control and configuration
- `vn310_driver.c` - Low-level driver handling UART communication, register access, and device protocols
- `vn310_parser.c` - Message parser for both binary and ASCII NMEA-style messages from the device
- `vn310_pose.c` - Pose estimation and coordinate transformation utilities

### Header Files (`inc/`)
- `vn310_applet.h` - Application state structures and initialization interfaces
- `vn310_cli.h` - CLI command definitions and handler interfaces
- `vn310_driver.h` - Driver configuration and communication interfaces
- `vn310_parser.h` - Message parsing structures and utilities
- `vn310_pose.h` - Pose data structures and transformation interfaces

## Basic Usage
```bash
# Device Control
vn310 power <on|off>              # Control device power
vn310 output <enable|disable>      # Control data output
vn310 output freq <1-200>         # Set output frequency in Hz

# Data Access
vn310 cli stream <start|stop>     # Control raw data streaming
vn310 read <parameter>            # Read device parameters
```
