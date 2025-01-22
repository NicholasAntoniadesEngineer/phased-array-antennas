"""
stm32_prog_windows.py

A Windows-specific API for interacting with STM32 microcontrollers via the
STM32CubeProgrammer CLI. Provides a clean interface for common operations:
- Flash programming and verification
- Memory reading and writing
- Sector and full chip erasure
- Device reset functionality

Author: Your Name
"""

import subprocess
import os
import time
import re
from typing import List, Union
from pathlib import Path

# STM32 Programmer CLI path
STM32_PROG_CLI = Path(r'C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe')

# Verify programmer exists
if not STM32_PROG_CLI.exists():
    raise FileNotFoundError(f"STM32 Programmer CLI not found at: {STM32_PROG_CLI}")

def _run_programmer(args: List[str], capture_output: bool = False) -> subprocess.CompletedProcess:
    """
    Execute STM32 programmer with given arguments and handle errors.
    
    Args:
        args: List of command arguments
        capture_output: Whether to capture command output
    
    Returns:
        CompletedProcess instance
    
    Raises:
        RuntimeError: If programmer command fails
    """
    try:
        result = subprocess.run(
            [STM32_PROG_CLI, *args],
            stdout=subprocess.PIPE if capture_output else None,
            check=True
        )
        return result
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"STM32 Programmer command failed: {' '.join(args)}") from e

def flash_erase() -> None:
    """Erase entire flash memory."""
    _run_programmer(['-c', 'port=SWD', '-e', 'all'])

def erase_sectors(sectors: List[int]) -> None:
    """
    Erase specific flash sectors.
    
    Args:
        sectors: List of sector numbers to erase
    """
    sectors_str = ','.join(map(str, sectors))
    _run_programmer(['-c', 'port=SWD', '-e', sectors_str])

def flash(hex_file: Union[str, Path], go: bool = False) -> None:
    """
    Program hex file to device and optionally start execution.
    
    Args:
        hex_file: Path to hex file
        go: Whether to start program execution after flashing
    """
    hex_path = Path(hex_file)
    if not hex_path.exists():
        raise FileNotFoundError(f"Hex file not found: {hex_path}")

    args = ['-c', 'port=SWD', '-w', str(hex_path), '-v']
    if go:
        args.append('-g')
    
    _run_programmer(args)

def target_reset() -> None:
    """Reset target device and wait for it to initialize."""
    _run_programmer(['-c', 'port=SWD', 'mode=Normal'])
    time.sleep(1)  # Allow device to initialize

def read_mem(address: int, count: int = 1) -> List[int]:
    """
    Read 32-bit words from memory.
    
    Args:
        address: Starting memory address
        count: Number of 32-bit words to read
    
    Returns:
        List of read values
    """
    result = _run_programmer([
        '-c', 'port=SWD', 'mode=HOTPLUG',
        '-r32', f'0x{address:x}', f'0x{4*count:x}'
    ], capture_output=True)

    data = []
    lines = result.stdout.decode().split('\r\n')
    for line in lines:
        if line.startswith('0x'):
            values = re.findall(r'(?:([a-f0-9A-F]{8})\s*)', line)[1:]
            data.extend(int(v, 16) for v in values)

    if len(data) != count:
        raise RuntimeError(f"Expected {count} values but got {len(data)}")
    
    return data

def write_mem(address: int, data: List[int]) -> None:
    """
    Write 32-bit words to memory.
    
    Args:
        address: Starting memory address
        data: List of values to write
    """
    data_str = [f'0x{d:x}' for d in data]
    _run_programmer([
        '-c', 'port=SWD', 'mode=HOTPLUG',
        '-w32', f'0x{address:x}', *data_str
    ])

def flash_all(hex_files: List[Union[str, Path]]) -> None:
    """
    Flash multiple hex files and reset target.
    
    Args:
        hex_files: List of hex file paths to program
    """
    for hex_file in hex_files:
        print(f'Programming: {hex_file}')
        flash(hex_file)
    print('Resetting target...')
    target_reset()
