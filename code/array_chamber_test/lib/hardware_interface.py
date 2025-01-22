"""
================================================================================
Hardware interface module for system communication
================================================================================
"""

import time
import serial
from typing import List, Optional

from ..config import SERIAL_PORT, SERIAL_BAUD

class HardwareInterface:
    @staticmethod
    def send_commands(commands: List[str], port: str = SERIAL_PORT, baud: int = SERIAL_BAUD) -> None:
        """Send commands to hardware over serial interface."""
        ser = serial.Serial(port, baud, timeout=0.5)
        start_time = time.time()
        try:
            for command in commands:
                ser.write(f'{command}\r\n'.encode())
                time.sleep(0.1)
                print(f"Sent: {command}")

                while True:
                    response = ser.readline().decode()
                    if time.time() - start_time > 2:
                        print("No response")
                        break
                    if 'OK' in response or response == '':
                        break
        finally:
            ser.close()
        time.sleep(0.1)

    @staticmethod
    def check_connection(port: str = SERIAL_PORT, baud: int = SERIAL_BAUD) -> bool:
        """Verify serial connection to hardware."""
        try:
            with serial.Serial(port, baud, timeout=1):
                return True
        except Exception as e:
            print(f"Failed to open serial port: {str(e)}")
            return False

    @staticmethod
    def to_attenuation_code(db: float, insertion_loss_db: float) -> int:
        """Convert dB value to attenuation code."""
        db -= insertion_loss_db
        return int(round(db * 4)) if db >= 0 else 0 