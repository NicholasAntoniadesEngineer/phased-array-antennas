"""
================================================================================
Phased Array Antenna Sweep Automation
================================================================================
Description:
    Main script for automating phased array antenna sweep tests. Coordinates
    hardware control, chamber equipment, and measurement processes for antenna
    characterization.

Author: Nicholas Antoniades
Date: 2024-10-16
================================================================================
"""

# from ChamberClass import ChamberClass
# from ChamberInfo import CHAMBER

from lib.array_controller import ArrayController
from lib.chamber_controller import ChamberController
from config import MODEM_FREQ, KU_FREQ_RANGE


def run_static_pose_sweep():
    """Execute static pose sweep test sequence."""
    print('\n=================================')
    print('Static Pose Sweep')
    print('=================================\n')

    # Initialize controllers
    array_ctrl = ArrayController()
    chamber_sweep_object = ChamberClass(
        sweep_name="Default_file_name",
        range=CHAMBER,
        frequency_GHz=(MODEM_FREQ / 1000000000)
    )
    chamber_ctrl = ChamberController(chamber_sweep_object)

    # Run sweep sequence for each frequency
    for ku_freq in KU_FREQ_RANGE:
        chamber_ctrl.run_sweep_sequence(array_ctrl, ku_freq, MODEM_FREQ)


def main():
    """Main execution function."""
    array_ctrl = ArrayController()
    array_ctrl.power_cycle()
    run_static_pose_sweep()


if __name__ == "__main__":
    main()
