"""
================================================================================
System-specific messages and commands for phased array control
This file should be modified based on the specific system being interfaced with.

Template Instructions:
1. Replace all command strings with appropriate system-specific commands
2. Update ARRAY_MAP and BAND_CODES according to system configuration
3. Modify command format strings to match system protocol
4. Add any additional system-specific commands as needed
================================================================================
"""

class SystemMessages:
    """Container for system-specific messages and command formats."""
    
    # Power control messages
    POWER_ON_CMD = ''  # Command to power on the system
    POWER_OFF_CMD = ''  # Command to power off the system
    
    # Band configuration messages
    BAND_CONFIG_CMD = ''  # Command to configure frequency band: params(channel, band_code)
    CHANNEL_CONFIG_CMD = ''  # Command to configure channel: params(channel, freq_khz, if_freq_khz)
    
    # Attenuation control messages
    BPL_ATTEN_CMD = ''  # Command to set BPL attenuation: params(channel, phased_array, atten_code)
    IF_ATTEN_CMD = ''  # Command to set IF attenuation: params(channel, atten_code)
    
    # Phased Array control messages
    ARRAY_STEER_CMD = ''  # Command to configure phased_array FBS: params(phased_array, phi, theta, gain, freq_hz, pol, enable)
    ARRAY_RF_ENABLE_CMD = ''  # Command to enable/disable phased_array RF: params(phased_array, enable)
    
    # Pointing control messages
    POSE_CMD = ''  # Command to set pose: params(roll, pitch, yaw, x, y)
    POINTING_GEN_CMD = ''  # Command to generate pointing
    
    # Modem configuration messages
    MODEM_IF_CMD = ''  # Command to configure modem IF: params(freq_hz, enable)
    BEAM_INFO_CMD = ''  # Command to set beam info: params(az, el, freq_hz, pol, enable)
    
    # System configuration - Update these maps according to system specifications
    ARRAY_MAP = {}  # Maps phased_array identifiers to system numbers
    BAND_CODES = {}  # Maps band names to system codes
    
    @classmethod
    def get_power_command(cls, enable: bool) -> str:
        """Generate power control command."""
        return cls.POWER_ON_CMD if enable else cls.POWER_OFF_CMD
    
    @classmethod
    def get_pose_commands(cls, pitch: float) -> list:
        """Generate pose control commands."""
        return [
            cls.POSE_CMD.format(roll=0, pitch=pitch, yaw=0, x=0, y=0),
            cls.POINTING_GEN_CMD
        ]
    
    @classmethod
    def get_phased_array_commands(cls, phased_array: str, phi: float, theta: float, 
                         gain: float, freq_hz: int, pol: float, enable: bool = True) -> list:
        """Generate phased_array control commands."""
        return [
            cls.ARRAY_STEER_CMD.format(
                phased_array=phased_array, phi=phi, theta=theta, gain=gain,
                freq_hz=freq_hz, pol=pol, enable=int(enable)
            ),
            cls.ARRAY_RF_ENABLE_CMD.format(phased_array=phased_array, enable=int(enable))
        ]
    
    @classmethod
    def get_pointing_commands(cls, freq_hz: int, az: float, el: float, 
                            pol: float, enable: bool = True) -> list:
        """Generate pointing control commands."""
        return [
            cls.MODEM_IF_CMD.format(freq_hz=freq_hz, enable=int(enable)),
            cls.BEAM_INFO_CMD.format(az=az, el=el, freq_hz=freq_hz, pol=pol, enable=int(enable)),
            cls.POINTING_GEN_CMD
        ] 