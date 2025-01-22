"""
================================================================================
Chamber controller module for anechoic chamber control
================================================================================
"""

import math
import random
from datetime import datetime
from typing import Dict, Union, List

from .hardware_interface import HardwareInterface
from ..config import (
    ARRAY_INTERFACE, SYSTEM_CONFIGURATION, PLOT_SAVE_PATH,
    SWEEP_BOTH_DIRECTIONS, ELEVATION_PHI_ADJUSTMENT, TURN_TABLE_ELV_LIMIT,
    AZ_START, AZ_FINISH, EL_START, EL_FINISH,
    POL_START, POL_FINISH, HORN_START, HORN_FINISH,
    PHI_RANGE, THETA_RANGE, PHI_RANGE_RANDOM, THETA_RANGE_RANDOM
)

class ChamberController:
    def __init__(self, sweep_runner):
        """Initialize chamber controller with sweep runner object."""
        self.sweep_runner = sweep_runner
        self.direction_forward = True

    def run_sweep_sequence(self, array_ctrl, ku_freq: int, l_band_freq: int) -> None:
        """Run a complete sweep sequence for given frequencies."""
        print(f'\nSweeping at {ku_freq} GHz\n')

        if ARRAY_INTERFACE == "app_cli_pointing_generator":
            array_ctrl.setup_array(ku_freq, 0, 90)

        for phi_angle in self._select_phi_angles():
            for theta_angle in self._select_theta_angles():
                # Configure array
                if ARRAY_INTERFACE == "app_cli_message_forewarding":
                    array_ctrl.control_array(ku_freq, phi_angle, theta_angle)
                else:
                    array_ctrl.control_pose(phi_angle, theta_angle)

                # Perform sweep
                self.direction_forward = self.perform_sweep(
                    self.direction_forward,
                    ku_freq,
                    l_band_freq,
                    phi_angle,
                    theta_angle
                )

    def perform_sweep(
        self,
        direction_forward: bool,
        ku_freq: int,
        l_band_freq: int,
        phi_angle: float,
        theta_angle: float
    ) -> bool:
        """
        Perform a sweep test with the given parameters.
        Returns updated direction flag for next sweep.
        """
        sweep_angles = self._calculate_sweep_angles(
            direction_forward, phi_angle, theta_angle
        )

        self._run_chamber_sweep(
            ku_freq,
            l_band_freq,
            theta_angle,
            phi_angle,
            sweep_angles
        )

        return not direction_forward if SWEEP_BOTH_DIRECTIONS else direction_forward

    @staticmethod
    def _select_phi_angles() -> List[float]:
        """Select phi angles for sweep based on configuration."""
        if PHI_RANGE_RANDOM:
            num_random_phi = 10
            phi_angles = [random.randint(0, 360) for _ in range(num_random_phi)]
            print(f"\nRandomly selected phi angles: {phi_angles}")
        else:
            phi_angles = list(PHI_RANGE)
            print(f"\nUsing predefined phi angles: {phi_angles}")
        
        return phi_angles

    @staticmethod
    def _select_theta_angles() -> List[float]:
        """Select theta angles for sweep based on configuration."""
        if THETA_RANGE_RANDOM:
            num_random_theta = 10
            theta_angles = [random.randint(5, 50) for _ in range(num_random_theta)]
            print(f"Randomly selected theta angles: {theta_angles}\n")
        else:
            theta_angles = list(THETA_RANGE)
            print(f"Using predefined theta angles: {theta_angles}\n")
        
        return theta_angles

    def _calculate_sweep_angles(
        self,
        direction_forward: bool,
        phi_angle: float,
        theta_angle: float
    ) -> Dict[str, float]:
        """Calculate sweep angles based on configuration and input parameters."""
        az_start = AZ_START
        az_finish = AZ_FINISH
        el_start = EL_START
        el_finish = EL_FINISH

        if SWEEP_BOTH_DIRECTIONS and not direction_forward:
            az_start = -AZ_START
            az_finish = -AZ_FINISH

        if ELEVATION_PHI_ADJUSTMENT:
            calc_elevation = theta_angle * math.sin(math.radians(phi_angle))
            calc_elevation = min(max(calc_elevation, -TURN_TABLE_ELV_LIMIT), TURN_TABLE_ELV_LIMIT)
            el_start = el_finish = calc_elevation

        return {
            "az_start": az_start,
            "az_finish": az_finish,
            "el_start": el_start,
            "el_finish": el_finish,
            "pol_start": POL_START,
            "pol_finish": POL_FINISH,
            "horn_start": HORN_START,
            "horn_finish": HORN_FINISH
        }

    def _run_chamber_sweep(
        self,
        ku_freq: int,
        l_band_freq: int,
        theta_angle: float,
        phi_angle: float,
        sweep_angles: Dict[str, float]
    ) -> None:
        """Execute chamber sweep with specified parameters."""
        # Generate sweep name with test parameters
        sweep_name = self._generate_sweep_name(
            ku_freq, l_band_freq, phi_angle, theta_angle
        )
        
        self.sweep_runner.sweep_name = sweep_name
        print('\nRun sweep')
        
        # Execute sweep test
        self.sweep_runner.run_sweep_test_swt(
            angles_list=sweep_angles,
            save_path=PLOT_SAVE_PATH,
            sPar=False
        )

    @staticmethod
    def _generate_sweep_name(
        ku_freq: int,
        l_band_freq: int,
        phi_angle: float,
        theta_angle: float
    ) -> str:
        """Generate unique sweep name with test parameters."""
        ku_freq_string = str(ku_freq / 1000000000).replace('.', '_')
        l_band_freq_string = str(l_band_freq / 1000000000).replace('.', '_')
        phi_angle_string = str(phi_angle).replace('.', '_')
        theta_angle_string = str(theta_angle).replace('.', '_')

        return (
            f"{datetime.now().strftime('%Y-%m-%d_%H-%M-%S')}_"
            f"{ARRAY_INTERFACE}_"
            f"{SYSTEM_CONFIGURATION}_"
            f"Ku_Freq_{ku_freq_string}GHz_"
            f"L_Freq_{l_band_freq_string}GHz_"
            f"Phi_{phi_angle_string}_"
            f"Theta_{theta_angle_string}"
        ) 