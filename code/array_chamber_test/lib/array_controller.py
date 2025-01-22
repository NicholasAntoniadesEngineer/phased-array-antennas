"""
================================================================================
Array controller module for phased array antenna system
Handles array-specific commands and control functions
================================================================================
"""

import time
from typing import List, Tuple, Optional

from .hardware_interface import HardwareInterface
from .system_messages import SystemMessages
from ..config import ARRAY_INTERFACE, MODEM_FREQ, POL_ANGLE

class ArrayController:
    def __init__(self):
        self.hardware = HardwareInterface()

    def power_cycle(self) -> None:
        """Perform power cycle of the array."""
        print("\nArray Power Cycle")
        if not self.hardware.check_connection():
            return

        # Power off
        self.hardware.send_commands([SystemMessages.get_power_command(False)])
        time.sleep(1)
        # Power on
        self.hardware.send_commands([SystemMessages.get_power_command(True)])
        time.sleep(3)

    def setup_array(self, ku_freq: int, azimuth: int, elevation: int) -> None:
        """Configure initial array settings."""
        if ARRAY_INTERFACE != "":
            return

        atten_codes = self._get_default_attenuation_codes()
        
        # Set attenuation
        self.hardware.send_commands(
            self._get_rx_atten_commands(ku_freq, atten_codes)
        )
        
        # Set pointing
        self.hardware.send_commands(
            SystemMessages.get_pointing_commands(ku_freq, azimuth, elevation, POL_ANGLE)
        )

    def control_array(self, ku_freq: int, phi_angle: int, theta_angle: int) -> None:
        """Control array pointing and configuration."""
        print(f"Pointing antenna to phi: {phi_angle}, theta: {theta_angle}")

        if ARRAY_INTERFACE == "":
            atten_codes = self._get_default_attenuation_codes()
            commands = []
            
            # Add band and channel configuration
            f_khz = ku_freq // 1000
            f_if_khz = MODEM_FREQ // 1000
            commands.extend([
                SystemMessages.BAND_CONFIG_CMD.format(channel=0, band_code=SystemMessages.BAND_CODES['L']),
                SystemMessages.CHANNEL_CONFIG_CMD.format(channel=0, freq_khz=f_khz, if_freq_khz=f_if_khz)
            ])
            
            # Add attenuation commands
            for i in range(4):
                commands.append(
                    SystemMessages.BPL_ATTEN_CMD.format(channel=0, array=i, atten_code=atten_codes[0][i])
                )
            commands.append(
                SystemMessages.IF_ATTEN_CMD.format(channel=0, atten_code=atten_codes[1][0])
            )
            
            # Add array commands
            for array in ['', '', '', '']:
                commands.extend(
                    SystemMessages.get_phased_array_commands(
                        array=array,
                        phi=phi_angle,
                        theta=theta_angle,
                        gain=0,
                        freq_hz=ku_freq,
                        pol=POL_ANGLE
                    )
                )
            
            self.hardware.send_commands(commands)
        else:
            self._control_pose(phi_angle, theta_angle)

    def _control_pose(self, phi_angle: int, theta_angle: int) -> None:
        """Control array pose for pointing interface."""
        pitch = theta_angle if phi_angle > 90 else 360 - theta_angle
        self.hardware.send_commands(SystemMessages.get_pose_commands(pitch))

    @staticmethod
    def _get_default_attenuation_codes() -> Tuple[List[int], List[int]]:
        """Get default attenuation codes for arrays and SM."""
        atten_values_arrays = [
            HardwareInterface.to_attenuation_code(0.0, 1.6) 
            for _ in range(4)
        ]
        atten_codes_sm = [HardwareInterface.to_attenuation_code(0.0, 0.0)]
        return (atten_values_arrays, atten_codes_sm)

    def _get_rx_atten_commands(
        self,
        f_hz: int,
        atten: Tuple[List[int], List[int]],
        arrays_list: Optional[List[str]] = None
    ) -> List[str]:
        """Generate RX attenuation commands."""
        if arrays_list is None:
            arrays_list = ['', '', '', '']

        f_khz = f_hz // 1000
        f_if_khz = MODEM_FREQ // 1000

        commands = [
            SystemMessages.BAND_CONFIG_CMD.format(channel=0, band_code=SystemMessages.BAND_CODES['L']),
            SystemMessages.CHANNEL_CONFIG_CMD.format(channel=0, freq_khz=f_khz, if_freq_khz=f_if_khz)
        ]

        for i in range(4):
            commands.append(
                SystemMessages.BPL_ATTEN_CMD.format(channel=0, array=i, atten_code=atten[0][i])
            )
        commands.append(
            SystemMessages.IF_ATTEN_CMD.format(channel=0, atten_code=atten[1][0])
        )

        return commands 