"""
================================================================================
Configuration settings for phased array antenna sweep automation
================================================================================
"""

import numpy as np

# Hardware Interface Configuration
ARRAY_INTERFACE = ""
SYSTEM_CONFIGURATION = ""

# Serial Communication Settings
SERIAL_PORT = 'COM6'
SERIAL_BAUD = 115200

# Test Output Configuration
PLOT_SAVE_PATH = r"C:\tests"

# Frequency Settings
MODEM_FREQ = 1500000000  # 1.5 GHz
KU_FREQ_RANGE = [11600000000]  # 11.6 GHz

# Sweep Angle Configuration
THETA_RANGE = np.arange(0, 35, 5)
PHI_RANGE = range(0, 181, 180)

# Sweep Control Flags
SWEEP_BOTH_DIRECTIONS = False
ELEVATION_PHI_ADJUSTMENT = False
PHI_RANGE_RANDOM = False
THETA_RANGE_RANDOM = False

# Turn Table Configuration
TURN_TABLE_ELV_LIMIT = 20
EL_START = 0
EL_FINISH = 0
POL_START = 0
POL_FINISH = 0
AZ_START = -35
AZ_FINISH = 35
HORN_START = 90
HORN_FINISH = 90

# Polarization Angle
POL_ANGLE = 0 if ARRAY_INTERFACE == "" else 90 