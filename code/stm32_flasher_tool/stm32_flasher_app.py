"""
stm32_flasher_app.py

A PyQt5-based GUI application for flashing STM32 firmware files.
Features:
- Automatic firmware file detection
- Support for Debug/Release builds
- Real-time file monitoring
- Simple one-click flashing
"""

import os
import glob
import sys
import time
from pathlib import Path
from typing import Dict, List

from PyQt5.QtWidgets import (QApplication, QWidget, QGridLayout, QLabel, QPushButton, QMessageBox, QCheckBox)
from PyQt5.QtCore import QTimer, Qt
from stm32_prog_windows import flash_all
import argparse

class FlasherApp(QWidget):
    """Main application window for the STM32 Flasher tool."""
    
    def __init__(self, firmware_path: str):
        super().__init__()
        self.firmware_path = firmware_path
        self.compile_type = 'Debug'
        self.firmware_files: Dict[str, List[str]] = {}
        
        self._init_ui()
        self._setup_timer()
        self._load_firmware_files()

    def _init_ui(self) -> None:
        """Initialize the user interface."""
        self.setWindowTitle("STM32 Firmware Flasher")
        self.setGeometry(100, 100, 600, 300)
        
        self.grid = QGridLayout()
        self.setLayout(self.grid)
        
        # Create header labels
        headers = ["Device", "Firmware Files", "Last Modified", "Actions"]
        for col, header in enumerate(headers):
            self.grid.addWidget(QLabel(header), 0, col)
        
        # Add compile type selector
        self.compile_checkbox = QCheckBox("Release Mode")
        self.compile_checkbox.stateChanged.connect(self._on_compile_type_changed)
        self.grid.addWidget(self.compile_checkbox, 0, 4)

    def _setup_timer(self) -> None:
        """Setup automatic UI refresh timer."""
        self.refresh_timer = QTimer(self)
        self.refresh_timer.setInterval(2000)  # 2 second refresh
        self.refresh_timer.timeout.connect(self._refresh_ui)
        self.refresh_timer.start()

    def _load_firmware_files(self) -> None:
        """Load firmware files from the specified path."""
        pattern = os.path.join(self.firmware_path, '**', 'CM?', 
                             self.compile_type, '*CM?.hex')
        firmware_paths = glob.glob(pattern, recursive=True)
        
        self.firmware_files = {}
        for i in range(0, len(firmware_paths), 2):
            if i + 1 < len(firmware_paths):
                device_name = Path(firmware_paths[i]).stem[:-3]  # Remove CM? suffix
                self.firmware_files[device_name] = firmware_paths[i:i+2]

    def _on_compile_type_changed(self, state: int) -> None:
        """Handle compile type checkbox state changes."""
        self.compile_type = 'Release' if state == Qt.Checked else 'Debug'
        self._load_firmware_files()
        self._refresh_ui()

    def _refresh_ui(self) -> None:
        """Refresh the UI with current firmware information."""
        # Clear existing widgets except headers and checkbox
        for i in reversed(range(self.grid.count())):
            widget = self.grid.itemAt(i).widget()
            if isinstance(widget, (QLabel, QPushButton)) and widget != self.compile_checkbox:
                widget.deleteLater()

        # Add firmware information
        for row, (device_name, hex_files) in enumerate(self.firmware_files.items(), start=1):
            # Device name
            self.grid.addWidget(QLabel(device_name), row, 0)
            
            # Firmware files
            self.grid.addWidget(QLabel('\n'.join(Path(f).name for f in hex_files)), row, 1)
            
            # Last modified times
            try:
                mtimes = [time.ctime(os.path.getmtime(f)) for f in hex_files]
                self.grid.addWidget(QLabel('\n'.join(mtimes)), row, 2)
                
                # Flash button
                flash_btn = QPushButton('Flash')
                flash_btn.clicked.connect(lambda checked, d=device_name: self._flash_device(d))
                self.grid.addWidget(flash_btn, row, 3)
            except OSError as e:
                self.grid.addWidget(QLabel(f"Error: {str(e)}"), row, 2)

    def _flash_device(self, device_name: str) -> None:
        """Flash firmware to the selected device."""
        try:
            flash_all(self.firmware_files[device_name])
        except Exception as e:
            QMessageBox.critical(
                self,
                "Flashing Error",
                f"Failed to flash {device_name}:\n{str(e)}",
                QMessageBox.Ok
            )

def main():
    """Application entry point."""
    parser = argparse.ArgumentParser(description='STM32 Firmware Flasher Tool')
    parser.add_argument('--base-dir', type=str, 
                       default=os.path.dirname(os.path.abspath(__file__)),
                       help='Base directory path containing firmware files')
    
    args = parser.parse_args()
    fw_path = os.path.join(args.base_dir, '..', '..', '..')
    print(f"Searching for firmware in: {fw_path}")

    app = QApplication(sys.argv)
    window = FlasherApp(fw_path)
    window.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
