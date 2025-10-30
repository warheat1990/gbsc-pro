"""
Serial console widget for direct interaction with device.
"""

from PyQt6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QTextEdit,
                            QLineEdit, QPushButton, QLabel)
from PyQt6.QtGui import QFont, QTextCursor
from PyQt6.QtCore import pyqtSignal, QTimer, Qt

from ..serial_bridge import SerialBridge
from ..utils.logger import get_logger


class SerialConsole(QWidget):
    """
    Serial console widget with command input and output display.

    Signals:
        command_executed: Emitted when a command is executed (command, response)
    """

    command_executed = pyqtSignal(str, str)  # command, response

    def __init__(self, serial_bridge: SerialBridge):
        """
        Initialize the serial console.

        Args:
            serial_bridge: Serial communication bridge
        """
        super().__init__()
        self.serial = serial_bridge
        self.logger = get_logger(__name__)
        self.init_ui()

    def init_ui(self):
        """Initialize the user interface."""
        layout = QVBoxLayout()
        layout.setContentsMargins(5, 5, 5, 5)
        layout.setSpacing(5)

        # Output text area
        self.output_text = QTextEdit()
        self.output_text.setReadOnly(True)
        self.output_text.setFont(QFont("Courier", 10))
        self.output_text.setPlaceholderText("Serial console output will appear here...")
        layout.addWidget(self.output_text)

        # Command input row
        cmd_layout = QHBoxLayout()
        cmd_layout.setSpacing(5)

        cmd_layout.addWidget(QLabel("Command:"))

        self.command_input = QLineEdit()
        self.command_input.setPlaceholderText("Enter command (e.g., rm 42 00 00 or d 56 00 FF)")
        self.command_input.setFont(QFont("Courier", 10))
        self.command_input.returnPressed.connect(self.send_command)
        cmd_layout.addWidget(self.command_input)

        send_btn = QPushButton("Send")
        send_btn.setMaximumWidth(70)
        send_btn.clicked.connect(self.send_command)
        cmd_layout.addWidget(send_btn)

        clear_btn = QPushButton("Clear")
        clear_btn.setMaximumWidth(70)
        clear_btn.clicked.connect(self.clear_output)
        cmd_layout.addWidget(clear_btn)

        layout.addLayout(cmd_layout)

        # Help text
        help_layout = QHBoxLayout()
        help_layout.setSpacing(5)
        commands_text = QLabel("Commands:")
        commands_text.setMaximumWidth(60)
        commands_text.setAlignment(Qt.AlignmentFlag.AlignTop)
        commands_text.setStyleSheet("color: gray; font-size: 9px;")
        commands_usage_text = QLabel("rm AA MM RR (read) | wm AA MM RR VV (write) | dm AA MM RR QQ (dump)\n"
                           "r AA RR (read) | w AA RR VV (write) | d AA RR QQ (dump)\n"
                           "AA=device, MM=map, RR=reg, VV=value, QQ=count")
        commands_usage_text.setStyleSheet("color: gray; font-size: 9px;")
        help_layout.addWidget(commands_text)
        help_layout.addWidget(commands_usage_text)
        layout.addLayout(help_layout)

        self.setLayout(layout)

    def send_command(self):
        """Send command entered by user."""
        if not self.serial.is_connected():
            self.append_output("ERROR: Not connected to serial port", "red")
            return

        command = self.command_input.text().strip()
        if not command:
            return

        # Echo command
        self.append_output(f"> {command}", "#0066cc")

        # Clear input
        self.command_input.clear()

        # Execute command based on type
        parts = command.split()
        if not parts:
            return

        cmd_type = parts[0].lower()

        try:
            if cmd_type == 'rm' and len(parts) == 4:
                # Read with map: rm AA MM RR
                addr = int(parts[1], 16)
                map_val = int(parts[2], 16)
                reg = int(parts[3], 16)

                def on_read(value):
                    if value is not None:
                        response = f"{value:02X}"
                        self.append_output(response, "#009900")
                        self.command_executed.emit(command, response)
                    else:
                        self.append_output("ERR", "#cc0000")

                self.serial.read_register(addr, map_val, reg, on_read)

            elif cmd_type == 'r' and len(parts) == 3:
                # Read without map: r AA RR
                addr = int(parts[1], 16)
                reg = int(parts[2], 16)

                def on_read(value):
                    if value is not None:
                        response = f"{value:02X}"
                        self.append_output(response, "#009900")
                        self.command_executed.emit(command, response)
                    else:
                        self.append_output("ERR", "#cc0000")

                self.serial.read_register(addr, 0x00, reg, on_read, use_map_switching=False)

            elif cmd_type == 'wm' and len(parts) == 5:
                # Write with map: wm AA MM RR VV
                addr = int(parts[1], 16)
                map_val = int(parts[2], 16)
                reg = int(parts[3], 16)
                val = int(parts[4], 16)

                def on_write(success):
                    response = "OK" if success else "ERR"
                    color = "#009900" if success else "#cc0000"
                    self.append_output(response, color)
                    self.command_executed.emit(command, response)

                self.serial.write_register(addr, map_val, reg, val, on_write)

            elif cmd_type == 'w' and len(parts) == 4:
                # Write without map: w AA RR VV
                addr = int(parts[1], 16)
                reg = int(parts[2], 16)
                val = int(parts[3], 16)

                def on_write(success):
                    response = "OK" if success else "ERR"
                    color = "#009900" if success else "#cc0000"
                    self.append_output(response, color)
                    self.command_executed.emit(command, response)

                self.serial.write_register(addr, 0x00, reg, val, on_write, use_map_switching=False)

            elif cmd_type == 'dm' and len(parts) == 5:
                # Dump with map: dm AA MM RR QQ
                addr = int(parts[1], 16)
                map_val = int(parts[2], 16)
                start_reg = int(parts[3], 16)
                count = int(parts[4], 16)

                self.append_output(f"Dumping {count} registers from 0x{start_reg:02X}...", "#666666")

                def on_dump_complete(values):
                    if values and len(values) > 0:
                        output = " ".join([f"{v:02X}" for v in values])
                        self.append_output(output, "#009900")
                        self.command_executed.emit(command, output)
                    else:
                        self.append_output("ERR", "#cc0000")

                self.serial.read_registers_dump(addr, map_val, start_reg, count, on_dump_complete)

            elif cmd_type == 'd' and len(parts) == 4:
                # Dump without map: d AA RR QQ
                addr = int(parts[1], 16)
                start_reg = int(parts[2], 16)
                count = int(parts[3], 16)

                self.append_output(f"Dumping {count} registers from 0x{start_reg:02X}...", "#666666")

                def on_dump_complete(values):
                    if values and len(values) > 0:
                        output = " ".join([f"{v:02X}" for v in values])
                        self.append_output(output, "#009900")
                        self.command_executed.emit(command, output)
                    else:
                        self.append_output("ERR", "#cc0000")

                self.serial.read_registers_dump(addr, 0x00, start_reg, count, on_dump_complete, use_map_switching=False)

            else:
                self.append_output("Invalid command format", "#cc0000")
                self.append_output("Usage:", "#666666")
                self.append_output("  With map:    rm AA MM RR | wm AA MM RR VV | dm AA MM RR QQ", "#666666")
                self.append_output("  Without map: r AA RR | w AA RR VV | d AA RR QQ", "#666666")

        except (ValueError, IndexError) as e:
            self.append_output(f"Error parsing command: {e}", "#cc0000")

    def append_output(self, text: str, color: str = "black"):
        """
        Append text to output with specified color.

        Args:
            text: Text to append
            color: HTML color code or name
        """
        cursor = self.output_text.textCursor()
        cursor.movePosition(QTextCursor.MoveOperation.End)
        cursor.insertHtml(f'<span style="color: {color};">{text}</span><br>')
        self.output_text.setTextCursor(cursor)
        self.output_text.ensureCursorVisible()

    def clear_output(self):
        """Clear the output text area."""
        self.output_text.clear()
        self.append_output("Console cleared", "#666666")

    def log_message(self, message: str, color: str = "#666666"):
        """
        Log a message to the console (e.g., from device or system).

        Args:
            message: Message to log
            color: Text color
        """
        self.append_output(message, color)
