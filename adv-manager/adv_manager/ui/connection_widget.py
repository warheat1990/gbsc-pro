"""
Widget for managing serial connection.
"""

from PyQt6.QtWidgets import (QWidget, QHBoxLayout, QLabel, QComboBox,
                            QPushButton, QMessageBox)
from PyQt6.QtCore import pyqtSignal
from serial.tools import list_ports

from ..serial_bridge import SerialBridge
from .styles import get_connection_status_style, IconHelper


class ConnectionWidget(QWidget):
    """Widget for serial port connection management."""

    # Signals
    connection_changed = pyqtSignal(bool)  # connected state

    def __init__(self, serial_bridge: SerialBridge):
        """
        Initialize the connection widget.

        Args:
            serial_bridge: Serial communication bridge
        """
        super().__init__()
        self.serial = serial_bridge
        self.init_ui()
        self.refresh_ports()

    def init_ui(self):
        """Initialize the user interface."""
        layout = QHBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(5)

        # Port label
        layout.addWidget(QLabel("Serial Port:"))

        # Port combo box
        self.port_combo = QComboBox()
        self.port_combo.setMaximumWidth(250)
        self.port_combo.setToolTip("Select serial port")
        layout.addWidget(self.port_combo)

        # Refresh button
        self.refresh_btn = QPushButton("Refresh")
        self.refresh_btn.setMaximumWidth(70)
        self.refresh_btn.setToolTip("Refresh port list")
        self.refresh_btn.clicked.connect(self.refresh_ports)
        layout.addWidget(self.refresh_btn)

        # Connect/Disconnect button
        self.connect_btn = QPushButton("Connect")
        self.connect_btn.setMaximumWidth(100)
        self.connect_btn.clicked.connect(self.toggle_connection)
        layout.addWidget(self.connect_btn)

        # Reload Registers button
        self.reload_btn = QPushButton("Reload Registers")
        self.reload_btn.setMinimumWidth(140)
        self.reload_btn.setMaximumWidth(140)
        self.reload_btn.setEnabled(False)  # Disabled until connected
        layout.addWidget(self.reload_btn)

        # Status indicator
        self.status_label = QLabel(f"{IconHelper.DISCONNECTED} Disconnected")
        self.status_label.setStyleSheet(get_connection_status_style(False))
        layout.addWidget(self.status_label)

        layout.addStretch()
        self.setLayout(layout)

    def refresh_ports(self):
        """Refresh the list of available serial ports."""
        current_port = self.port_combo.currentData()

        self.port_combo.clear()
        ports = list_ports.comports()

        if not ports:
            self.port_combo.addItem("No ports found", None)
            self.port_combo.setEnabled(False)
            self.connect_btn.setEnabled(False)
        else:
            self.port_combo.setEnabled(True)
            self.connect_btn.setEnabled(True)

            for port in ports:
                # Show port name and description
                display_name = f"{port.device}"
                if port.description and port.description != port.device:
                    display_name += f" - {port.description}"

                self.port_combo.addItem(display_name, port.device)

                # Restore previous selection if it still exists
                if current_port and port.device == current_port:
                    self.port_combo.setCurrentIndex(self.port_combo.count() - 1)

    def toggle_connection(self):
        """Toggle connection to serial port."""
        if not self.serial.is_connected():
            self._connect()
        else:
            self._disconnect()

    def _connect(self):
        """Connect to the selected serial port."""
        port = self.port_combo.currentData()
        if not port:
            QMessageBox.warning(self, "Warning", "Please select a valid serial port!")
            return

        # Update serial bridge port
        self.serial.port = port

        # Attempt connection
        if self.serial.connect():
            self.connect_btn.setText("Disconnect")
            self.status_label.setText(f"{IconHelper.CONNECTED} Connected")
            self.status_label.setStyleSheet(get_connection_status_style(True))
            self.port_combo.setEnabled(False)
            self.refresh_btn.setEnabled(False)
            self.reload_btn.setEnabled(True)
            self.connection_changed.emit(True)
        else:
            QMessageBox.critical(
                self,
                "Connection Error",
                f"Failed to connect to {port}.\n\n"
                "Please check:\n"
                "- Port is not in use by another application\n"
                "- Device is properly connected\n"
                "- You have permission to access the port"
            )

    def _disconnect(self):
        """Disconnect from the serial port."""
        self.serial.disconnect()
        self.connect_btn.setText("Connect")
        self.status_label.setText(f"{IconHelper.DISCONNECTED} Disconnected")
        self.status_label.setStyleSheet(get_connection_status_style(False))
        self.port_combo.setEnabled(True)
        self.refresh_btn.setEnabled(True)
        self.reload_btn.setEnabled(False)
        self.connection_changed.emit(False)

    def is_connected(self) -> bool:
        """Check if currently connected."""
        return self.serial.is_connected()

    def get_selected_port(self) -> str:
        """Get the currently selected port."""
        return self.port_combo.currentData() or ""
