# Phased Array Chamber Test Automation

## Overview
This project provides an automated testing framework for characterizing phased array antennas in an anechoic chamber environment. The system coordinates hardware control, chamber equipment, and measurement processes to perform comprehensive antenna pattern measurements.

## Features
- Automated phased array antenna sweep testing
- Configurable frequency ranges (Ku-band and L-band support)
- Customizable sweep patterns for azimuth and elevation
- Power cycling and array initialization
- Real-time measurement data collection
- Integrated chamber control system

## Configuration

Key system parameters can be configured in `lib/config.py`:
- Frequency settings (Modem and Ku-band frequencies)
- Sweep angle ranges (Theta and Phi)
- Turn table limits and positions
- Serial communication settings
- Test output paths


