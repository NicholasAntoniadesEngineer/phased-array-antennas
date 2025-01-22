# STM32 Firmware Flasher Tool

A PyQt5-based GUI application for easily flashing firmware to STM32 microcontrollers.

## Features

- User-friendly graphical interface
- Automatic firmware file detection
- Support for Debug/Release builds
- Real-time file monitoring
- One-click firmware flashing
- Multi-device support

## Requirements

- Python 3.x
- PyQt5
- STM32 programmer utilities

## Installation

1. Clone this repository
2. Install dependencies:
```bash
pip install PyQt5
```

## Usage

Run the application:
```bash
python stm32_flasher_app.py
```

Optional arguments:
- `--base-dir`: Specify custom base directory for firmware files

## Interface

The GUI displays:
- Device names
- Available firmware files
- Last modification timestamps
- Flash buttons for each device

Toggle between Debug/Release builds using the checkbox in the top-right corner.

## File Structure

- `stm32_flasher_app.py`: Main GUI application
- `stm32_prog_windows.py`: Firmware flashing implementation

## Notes

- Firmware files should follow the naming convention: `*CM?.hex`
- Files should be organized in Debug/Release folders
- The tool automatically refreshes every 2 seconds to detect new firmware files 