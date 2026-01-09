#!/usr/bin/env python3
"""
GBSC Pro Flasher

Unified flash tool for GBSC Pro with automatic device detection:
  - ADV Controller (HC32F460) via YMODEM protocol
  - GBS-Control (ESP8266) via esptool

Supports Windows, macOS, and Linux.

Usage:
  GUI mode:   python gbsc_flasher.py
  CLI mode:   python gbsc_flasher.py <port> <firmware.bin>
              python gbsc_flasher.py --auto <firmware.bin>

Repository: https://github.com/brisma/gbsc-pro
"""

import sys
import os
import time
import struct
import argparse
from typing import Optional, Callable, List, Tuple
from enum import Enum

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("Error: pyserial is required. Install with: pip install pyserial")
    sys.exit(1)

# =============================================================================
# Version
# =============================================================================

__version__ = "1.0.0"

# =============================================================================
# Constants
# =============================================================================

# YMODEM constants (for ADV Controller)
SOH = 0x01
STX = 0x02
EOT = 0x04
ACK = 0x06
NAK = 0x15
CAN = 0x18
CPMEOF = 0x1A
PACKET_SIZE = 128

# ESP8266 flash settings
ESP_BAUD = 460800  # Reduced from 921600 for better compatibility
ESP_FLASH_MODE = "dio"
ESP_FLASH_FREQ = "40m"
ESP_FLASH_SIZE = "4MB"

# Firmware size thresholds for auto-detection
ADV_MAX_SIZE = 256 * 1024      # ADV firmware is typically ~30KB, max 256KB
ESP_MIN_SIZE = 256 * 1024      # ESP firmware is typically ~850KB+


class ControllerType(Enum):
    ADV = "ADV Controller (HC32F460)"
    ESP = "GBS-Control (ESP8266)"
    UNKNOWN = "Unknown"


# =============================================================================
# Detection Functions
# =============================================================================

def calc_crc16(data: bytes) -> int:
    """Calculate CRC-16-CCITT for YMODEM"""
    crc = 0
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc <<= 1
            crc &= 0xFFFF
    return crc


def list_serial_ports() -> List[str]:
    """List available serial ports"""
    ports = []
    for port in serial.tools.list_ports.comports():
        ports.append(port.device)
    return sorted(ports)


def detect_port_type(port_device: str) -> ControllerType:
    """Detect controller type from port characteristics"""
    if not port_device:
        return ControllerType.UNKNOWN

    for port in serial.tools.list_ports.comports():
        if port.device == port_device:
            # Get port info
            vid = port.vid or 0
            pid = port.pid or 0
            desc = (port.description or '').lower()
            mfr = (port.manufacturer or '').lower()
            device = port.device.lower()

            # Check for HCMGBoot (ADV Controller)
            # XHSC HC32F460 in bootloader mode: VID=0x2E88, PID=0x4603
            if vid == 0x2E88 and pid == 0x4603:
                return ControllerType.ADV
            if 'xhsc' in mfr:
                return ControllerType.ADV
            # macOS: appears as /dev/cu.usbmodemXXXX
            if 'usbmodem' in device:
                return ControllerType.ADV
            # Windows/Linux: check description
            if 'hcmgboot' in desc or 'hc32' in desc:
                return ControllerType.ADV

            # Check for ESP8266 (CH340 or CP2102)
            # CH340: VID=0x1A86, PID=0x7523
            if vid == 0x1A86 and pid == 0x7523:
                return ControllerType.ESP
            # CP210x: VID=0x10C4
            if vid == 0x10C4:
                return ControllerType.ESP
            if any(x in desc or x in mfr for x in ['ch340', 'ch341', 'cp210', 'silicon labs']):
                return ControllerType.ESP
            # macOS: appears as /dev/cu.wchusbserialXXXX
            if 'wchusbserial' in device or 'wch' in device:
                return ControllerType.ESP
            # Linux: typically /dev/ttyUSB0 with CH340
            if 'ttyusb' in device and ('ch340' in desc or 'ch341' in desc):
                return ControllerType.ESP

    return ControllerType.UNKNOWN


def detect_firmware_type(firmware_path: str) -> ControllerType:
    """Detect controller type from firmware file characteristics"""
    if not firmware_path or not os.path.exists(firmware_path):
        return ControllerType.UNKNOWN

    filesize = os.path.getsize(firmware_path)

    # Size-based detection
    if filesize < ADV_MAX_SIZE:
        return ControllerType.ADV
    elif filesize >= ESP_MIN_SIZE:
        return ControllerType.ESP

    # Check for ESP8266 bootloader magic at start
    try:
        with open(firmware_path, 'rb') as f:
            header = f.read(4)
            # ESP8266 firmware starts with 0xE9 (ESP bootloader)
            if header[0] == 0xE9:
                return ControllerType.ESP
    except Exception:
        pass

    return ControllerType.UNKNOWN


def auto_detect(port: Optional[str], firmware: str) -> Tuple[ControllerType, str]:
    """
    Auto-detect controller type and port.
    Returns (ControllerType, port)
    """
    # Try firmware detection first (more reliable)
    fw_type = detect_firmware_type(firmware)

    if port:
        port_type = detect_port_type(port)

        # If both detected, they should match
        if fw_type != ControllerType.UNKNOWN and port_type != ControllerType.UNKNOWN:
            if fw_type != port_type:
                # Firmware type takes precedence, warn user
                return fw_type, port

        # Return whichever is known
        if fw_type != ControllerType.UNKNOWN:
            return fw_type, port
        if port_type != ControllerType.UNKNOWN:
            return port_type, port

        return ControllerType.UNKNOWN, port

    # No port specified, try to find one matching firmware type
    if fw_type != ControllerType.UNKNOWN:
        for p in serial.tools.list_ports.comports():
            if detect_port_type(p.device) == fw_type:
                return fw_type, p.device

    return fw_type, ""


def find_all_devices() -> List[Tuple[str, ControllerType]]:
    """Find all connected GBSC devices and their types"""
    devices = []
    for port in serial.tools.list_ports.comports():
        port_type = detect_port_type(port.device)
        if port_type != ControllerType.UNKNOWN:
            devices.append((port.device, port_type))
    return devices


# =============================================================================
# ADV Controller Flasher (HC32F460 - YMODEM)
# =============================================================================

class ADVFlasher:
    """YMODEM flasher for HCMGBoot bootloader (ADV Controller)"""

    def __init__(self, port: str, baudrate: int = 115200):
        self.port = port
        self.baudrate = baudrate
        self.ser: Optional[serial.Serial] = None
        self.chip_id = ""
        self.unique_id = ""

        self.on_progress: Optional[Callable[[int, int], None]] = None
        self.on_status: Optional[Callable[[str], None]] = None
        self.on_error: Optional[Callable[[str], None]] = None

    def _status(self, msg: str):
        if self.on_status:
            self.on_status(msg)
        else:
            print(f"[*] {msg}")

    def _error(self, msg: str):
        if self.on_error:
            self.on_error(msg)
        else:
            print(f"[ERROR] {msg}")

    def _progress(self, current: int, total: int):
        if self.on_progress:
            self.on_progress(current, total)

    def connect(self) -> bool:
        self._status(f"Connecting to {self.port}...")
        try:
            self.ser = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=2
            )
            self.ser.reset_input_buffer()
            self.ser.reset_output_buffer()
            time.sleep(0.2)

            if self.ser.in_waiting > 0:
                banner = self.ser.read(self.ser.in_waiting).decode('utf-8', errors='ignore').strip()
                self._status(f"Bootloader: {banner}")

            return True
        except serial.SerialException as e:
            self._error(f"Connection failed: {e}")
            return False

    def get_chip_info(self) -> bool:
        self._status("Getting chip info...")
        self.ser.reset_input_buffer()
        self.ser.write(b'\x55')
        self.ser.flush()
        time.sleep(0.2)

        if self.ser.in_waiting >= 48:
            info = self.ser.read(48)
            self.chip_id = info[:6].decode('utf-8', errors='ignore')
            self.unique_id = ' '.join(f'{b:02X}' for b in info[16:32])
            self._status(f"Chip: {self.chip_id}")
            return True
        return False

    def enter_download_mode(self) -> bool:
        self._status("Entering download mode...")
        self.ser.reset_input_buffer()
        self.ser.write(b'1')
        self.ser.flush()
        time.sleep(0.3)

        if self.ser.in_waiting > 0:
            resp = self.ser.read(self.ser.in_waiting).decode('utf-8', errors='ignore').strip()
            self._status(resp)
        return True

    def wait_for_c(self, timeout: float = 10) -> bool:
        start = time.time()
        while time.time() - start < timeout:
            if self.ser.in_waiting > 0:
                b = self.ser.read(1)[0]
                if b == ord('C'):
                    return True
            time.sleep(0.01)
        return False

    def wait_for_ack(self, timeout: float = 5) -> Optional[bool]:
        start = time.time()
        while time.time() - start < timeout:
            if self.ser.in_waiting > 0:
                b = self.ser.read(1)[0]
                if b == ACK:
                    return True
                elif b == NAK:
                    return False
                elif b == CAN:
                    return None
            time.sleep(0.01)
        return None

    def send_packet(self, seq: int, data: bytes) -> Optional[bool]:
        if len(data) < PACKET_SIZE:
            data = data + bytes([CPMEOF] * (PACKET_SIZE - len(data)))

        crc = calc_crc16(data)
        packet = bytes([SOH, seq & 0xFF, (~seq) & 0xFF]) + data + struct.pack('>H', crc)

        self.ser.write(packet)
        self.ser.flush()

        return self.wait_for_ack()

    def send_ymodem_header(self, filename: str, filesize: int) -> Optional[bool]:
        header = filename.encode('ascii') + b'\x00'
        header += str(filesize).encode('ascii') + b'\x00'

        if len(header) < PACKET_SIZE:
            header += bytes([0x00] * (PACKET_SIZE - len(header)))

        return self.send_packet(0, header)

    def send_ymodem_end(self) -> Optional[bool]:
        return self.send_packet(0, bytes([0x00] * PACKET_SIZE))

    def flash(self, firmware_path: str) -> bool:
        if not os.path.exists(firmware_path):
            self._error(f"File not found: {firmware_path}")
            return False

        with open(firmware_path, 'rb') as f:
            firmware = f.read()

        filename = os.path.basename(firmware_path)
        filesize = len(firmware)

        self._status(f"Firmware: {filename} ({filesize} bytes)")

        if not self.connect():
            return False

        self.get_chip_info()

        if not self.enter_download_mode():
            return False

        self._status("Waiting for YMODEM handshake...")
        if not self.wait_for_c():
            self._error("Timeout waiting for bootloader")
            return False

        self._status("Sending file header...")
        if self.send_ymodem_header(filename, filesize) is not True:
            self._error("Header rejected")
            return False

        if not self.wait_for_c():
            self._error("Timeout waiting for data request")
            return False

        self._status("Sending firmware...")
        seq = 1
        offset = 0
        retries = 0

        while offset < filesize:
            chunk = firmware[offset:offset + PACKET_SIZE]
            result = self.send_packet(seq, chunk)

            if result is True:
                offset += PACKET_SIZE
                seq = (seq + 1) & 0xFF
                retries = 0
                self._progress(min(offset, filesize), filesize)
            elif result is False:
                retries += 1
                if retries >= 10:
                    self._error(f"Too many retries at offset {offset}")
                    return False
            else:
                self._error("Transfer cancelled")
                return False

        self._status("Finishing transfer...")
        self.ser.write(bytes([EOT]))
        self.ser.flush()

        if not self.wait_for_ack():
            self.ser.write(bytes([EOT]))
            self.ser.flush()
            self.wait_for_ack()

        if self.wait_for_c(timeout=2):
            self.send_ymodem_end()

        self._status("Flash complete!")
        return True

    def close(self):
        if self.ser:
            self.ser.close()
            self.ser = None


# =============================================================================
# ESP8266 Flasher (via esptool)
# =============================================================================

class ESPFlasher:
    """ESP8266 flasher using esptool"""

    def __init__(self, port: str):
        self.port = port

        self.on_progress: Optional[Callable[[int, int], None]] = None
        self.on_status: Optional[Callable[[str], None]] = None
        self.on_error: Optional[Callable[[str], None]] = None

    def _status(self, msg: str):
        if self.on_status:
            self.on_status(msg)
        else:
            print(f"[*] {msg}")

    def _error(self, msg: str):
        if self.on_error:
            self.on_error(msg)
        else:
            print(f"[ERROR] {msg}")

    def _progress(self, current: int, total: int):
        if self.on_progress:
            self.on_progress(current, total)

    def flash(self, firmware_path: str) -> bool:
        if not os.path.exists(firmware_path):
            self._error(f"File not found: {firmware_path}")
            return False

        filesize = os.path.getsize(firmware_path)
        self._status(f"Firmware: {os.path.basename(firmware_path)} ({filesize} bytes)")
        self._status(f"Port: {self.port}")
        self._status(f"Baud: {ESP_BAUD}")

        try:
            import esptool
        except ImportError:
            self._error("esptool not found. Install with: pip install esptool")
            return False

        self._status("Connecting to ESP8266...")

        # Prepare esptool arguments
        args = [
            "--chip", "esp8266",
            "--port", self.port,
            "--baud", str(ESP_BAUD),
            "--before", "default_reset",
            "--after", "hard_reset",
            "write_flash",
            "--flash_mode", ESP_FLASH_MODE,
            "--flash_freq", ESP_FLASH_FREQ,
            "--flash_size", ESP_FLASH_SIZE,
            "0x0", firmware_path
        ]

        # Monkey-patch esptool's logger to capture progress
        try:
            from esptool.logger import log as esptool_log

            flasher_self = self
            original_progress_bar = esptool_log.progress_bar
            original_print = esptool_log.print

            def custom_progress_bar(cur_iter, total_iters, prefix="", suffix="", bar_length=30):
                pct = int(100 * cur_iter / total_iters) if total_iters > 0 else 0
                flasher_self._progress(pct, 100)

            last_error = [None]  # Use list to allow modification in nested function
            all_messages = []  # Capture all messages for debugging

            def custom_print(*args, **kwargs):
                msg = " ".join(str(a) for a in args)
                all_messages.append(msg)
                if "Connecting" in msg:
                    flasher_self._status("Connecting...")
                elif "Chip is" in msg:
                    flasher_self._status(msg.strip())
                elif "Erasing flash" in msg:
                    flasher_self._status("Erasing flash...")
                elif "Writing" in msg and "%" not in msg:
                    flasher_self._status("Writing flash...")
                elif "Hash of data verified" in msg:
                    flasher_self._status("Verification OK")
                elif "Hard resetting" in msg:
                    flasher_self._status("Resetting device...")
                elif "error" in msg.lower() or "failed" in msg.lower():
                    last_error[0] = msg.strip()

            original_error = esptool_log.error

            def custom_error(msg):
                last_error[0] = msg
                all_messages.append(f"ERROR: {msg}")
                original_error(msg)

            esptool_log.progress_bar = custom_progress_bar
            esptool_log.print = custom_print
            esptool_log.error = custom_error

        except (ImportError, AttributeError):
            # Older esptool version without logger module
            esptool_log = None
            original_progress_bar = None
            original_print = None
            original_error = None
            last_error = [None]
            all_messages = []

        try:
            self._status("Starting flash...")
            esptool.main(args)

            self._progress(100, 100)
            self._status("Flash complete!")
            return True

        except SystemExit as e:
            if e.code == 0:
                self._progress(100, 100)
                self._status("Flash complete!")
                return True
            else:
                if last_error[0]:
                    self._error(last_error[0])
                elif all_messages:
                    # Show last few messages for debugging
                    recent = all_messages[-3:] if len(all_messages) > 3 else all_messages
                    self._error(f"esptool failed (code {e.code}): {'; '.join(recent)}")
                else:
                    self._error(f"esptool failed with code {e.code}")
                return False
        except Exception as e:
            error_msg = str(e) if str(e) else None
            if not error_msg and last_error[0]:
                error_msg = last_error[0]
            elif not error_msg and all_messages:
                recent = all_messages[-3:] if len(all_messages) > 3 else all_messages
                error_msg = '; '.join(recent)
            elif not error_msg:
                error_msg = "Unknown error"
            self._error(f"Error: {error_msg}")
            return False
        finally:
            if esptool_log:
                if original_progress_bar:
                    esptool_log.progress_bar = original_progress_bar
                if original_print:
                    esptool_log.print = original_print
                if original_error:
                    esptool_log.error = original_error

    def close(self):
        pass


# =============================================================================
# Unified Flasher
# =============================================================================

def create_flasher(controller_type: ControllerType, port: str):
    """Create appropriate flasher based on controller type"""
    if controller_type == ControllerType.ADV:
        return ADVFlasher(port)
    elif controller_type == ControllerType.ESP:
        return ESPFlasher(port)
    else:
        raise ValueError(f"Unknown controller type: {controller_type}")


# =============================================================================
# CLI Mode
# =============================================================================

def run_cli(port: Optional[str], firmware: str, force_type: Optional[ControllerType] = None) -> int:
    print()
    print("  GBSC Pro Flasher v" + __version__)
    print("  " + "=" * 50)
    print()

    # Detect or use forced type
    if force_type:
        controller_type = force_type
        if not port:
            # Try to find matching port
            _, port = auto_detect(None, firmware)
    else:
        controller_type, detected_port = auto_detect(port, firmware)
        if not port:
            port = detected_port

    if controller_type == ControllerType.UNKNOWN:
        print("  [!] Could not auto-detect device type.")
        print("      Use --adv or --esp to specify manually.")
        return 1

    if not port:
        print(f"  [!] No {controller_type.value} port found.")
        print("      Connect the device and try again.")
        return 1

    print(f"  Device:   {controller_type.value}")
    print(f"  Port:     {port}")
    print(f"  Firmware: {os.path.basename(firmware)}")
    print()

    flasher = create_flasher(controller_type, port)

    def progress_callback(current, total):
        pct = int(current * 100 / total) if total > 0 else 0
        blocks = pct // 2
        bar = '█' * blocks + '░' * (50 - blocks)
        print(f"\r    [{bar}] {pct:3d}%", end='', flush=True)
        if current >= total:
            print()

    flasher.on_progress = progress_callback

    try:
        success = flasher.flash(firmware)
        if success:
            print()
            print("  " + "=" * 50)
            print("  ✓ Flash complete!")
            print("  " + "=" * 50)
        return 0 if success else 1
    except KeyboardInterrupt:
        print("\n[!] Cancelled")
        return 1
    except Exception as e:
        print(f"\n[-] Error: {e}")
        return 1
    finally:
        flasher.close()


# =============================================================================
# GUI Mode (PyQt6)
# =============================================================================

def run_gui():
    try:
        from PyQt6.QtWidgets import (
            QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
            QLabel, QComboBox, QPushButton, QFileDialog, QProgressBar,
            QTextEdit, QGroupBox, QMessageBox, QLineEdit
        )
        from PyQt6.QtCore import Qt, QThread, pyqtSignal, QTimer
        from PyQt6.QtGui import QFont, QPixmap
    except ImportError:
        print("PyQt6 not installed. Install with: pip install PyQt6")
        print("For CLI mode, use: python gbsc_flasher.py <port> <firmware.bin>")
        sys.exit(1)

    class FlashWorker(QThread):
        progress = pyqtSignal(int, int)
        status = pyqtSignal(str)
        error = pyqtSignal(str)
        finished_signal = pyqtSignal(bool)

        def __init__(self, controller: ControllerType, port: str, firmware: str):
            super().__init__()
            self.controller = controller
            self.port = port
            self.firmware = firmware

        def run(self):
            flasher = create_flasher(self.controller, self.port)
            flasher.on_progress = lambda c, t: self.progress.emit(c, t)
            flasher.on_status = lambda m: self.status.emit(m)
            flasher.on_error = lambda m: self.error.emit(m)

            try:
                success = flasher.flash(self.firmware)
                self.finished_signal.emit(success)
            except Exception as e:
                self.error.emit(str(e))
                self.finished_signal.emit(False)
            finally:
                flasher.close()

    class MainWindow(QMainWindow):
        def __init__(self):
            super().__init__()
            self.worker = None
            self.last_ports = set()
            self.init_ui()

            # Enable drag & drop
            self.setAcceptDrops(True)

            # USB monitoring timer
            self.usb_timer = QTimer()
            self.usb_timer.timeout.connect(self.check_usb_changes)
            self.usb_timer.start(1000)  # Check every second

        def check_usb_changes(self):
            """Check if USB devices have changed"""
            current_ports = set(p.device for p in serial.tools.list_ports.comports())
            if current_ports != self.last_ports:
                removed = self.last_ports - current_ports
                self.last_ports = current_ports
                self.refresh_ports()
                # Reset UI if device was disconnected
                if removed:
                    self.reset_ui()

        def dragEnterEvent(self, event):
            if event.mimeData().hasUrls():
                # Check if it's a .bin file
                for url in event.mimeData().urls():
                    if url.toLocalFile().lower().endswith('.bin'):
                        event.acceptProposedAction()
                        return
            event.ignore()

        def dropEvent(self, event):
            for url in event.mimeData().urls():
                path = url.toLocalFile()
                if path.lower().endswith('.bin') and os.path.exists(path):
                    self.fw_path.setText(path)
                    break

        def init_ui(self):
            self.setWindowTitle(f"GBSC Pro Flasher v{__version__}")
            self.setMinimumSize(550, 520)

            # Set window icon
            logo_path = os.path.join(os.path.dirname(__file__), "gbsc-pro-logo.png")
            if os.path.exists(logo_path):
                from PyQt6.QtGui import QIcon
                self.setWindowIcon(QIcon(logo_path))

            central = QWidget()
            self.setCentralWidget(central)
            layout = QVBoxLayout(central)
            layout.setSpacing(12)

            # Logo and Title
            header_layout = QVBoxLayout()
            header_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)

            # Logo
            if os.path.exists(logo_path):
                logo_label = QLabel()
                logo_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
                pixmap = QPixmap(logo_path)
                # Scale logo to reasonable size
                scaled_pixmap = pixmap.scaledToHeight(50, Qt.TransformationMode.SmoothTransformation)
                logo_label.setPixmap(scaled_pixmap)
                header_layout.addWidget(logo_label)

            # Title
            title = QLabel("GBSC Pro Flasher")
            title.setFont(QFont("Arial", 14, QFont.Weight.Bold))
            title.setAlignment(Qt.AlignmentFlag.AlignCenter)
            title.setStyleSheet("color: #555;")
            header_layout.addWidget(title)

            layout.addLayout(header_layout)

            # Device selection
            device_group = QGroupBox("Device")
            device_layout = QVBoxLayout(device_group)

            # Detected device label
            self.device_label = QLabel("No device detected")
            self.device_label.setFont(QFont("Arial", 13, QFont.Weight.Bold))
            self.device_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
            self.device_label.setStyleSheet("padding: 8px; color: #666;")
            device_layout.addWidget(self.device_label)

            # Port selection
            port_layout = QHBoxLayout()
            self.port_combo = QComboBox()
            self.port_combo.setMinimumWidth(300)
            self.port_combo.currentTextChanged.connect(self.on_port_changed)
            port_layout.addWidget(self.port_combo)

            refresh_btn = QPushButton("Refresh")
            refresh_btn.clicked.connect(self.refresh_ports)
            port_layout.addWidget(refresh_btn)
            device_layout.addLayout(port_layout)

            layout.addWidget(device_group)

            # Firmware selection
            fw_group = QGroupBox("Firmware File")
            fw_layout = QHBoxLayout(fw_group)

            self.fw_path = QLineEdit()
            self.fw_path.setPlaceholderText("Select firmware file or drag & drop...")
            self.fw_path.setReadOnly(True)
            self.fw_path.textChanged.connect(self.on_firmware_changed)
            fw_layout.addWidget(self.fw_path)

            browse_btn = QPushButton("Browse...")
            browse_btn.clicked.connect(self.browse_firmware)
            fw_layout.addWidget(browse_btn)

            layout.addWidget(fw_group)

            # Progress
            progress_group = QGroupBox("Progress")
            progress_layout = QVBoxLayout(progress_group)

            self.progress_bar = QProgressBar()
            self.progress_bar.setMinimum(0)
            self.progress_bar.setMaximum(100)
            self.progress_bar.setMinimumHeight(25)
            progress_layout.addWidget(self.progress_bar)

            self.log_text = QTextEdit()
            self.log_text.setReadOnly(True)
            self.log_text.setMaximumHeight(120)
            # Use platform-appropriate monospace font
            if sys.platform == 'darwin':
                mono_font = "Menlo"
            elif sys.platform == 'win32':
                mono_font = "Consolas"
            else:
                mono_font = "Monospace"
            self.log_text.setFont(QFont(mono_font, 10))
            progress_layout.addWidget(self.log_text)

            layout.addWidget(progress_group)

            # Initialize detected type before creating button
            self.detected_type = ControllerType.UNKNOWN

            # Flash button
            self.flash_btn = QPushButton("Flash Firmware")
            self.flash_btn.setMinimumHeight(50)
            self.flash_btn.setFont(QFont("Arial", 12, QFont.Weight.Bold))
            self.flash_btn.clicked.connect(self.start_flash)
            self.flash_btn.setEnabled(False)
            self.update_flash_button_style()
            layout.addWidget(self.flash_btn)

            # Footer
            footer = QLabel("Auto-detects ADV Controller (HC32) and GBS-Control (ESP8266)")
            footer.setAlignment(Qt.AlignmentFlag.AlignCenter)
            footer.setStyleSheet("color: #888; font-size: 10px; margin-top: 5px;")
            layout.addWidget(footer)

            # Initialize ports and USB monitoring
            self.last_ports = set(p.device for p in serial.tools.list_ports.comports())
            self.refresh_ports()

        def reset_ui(self):
            """Reset UI state when device is disconnected"""
            self.progress_bar.setValue(0)
            self.log_text.clear()

        def update_flash_button_style(self):
            if self.detected_type == ControllerType.ADV:
                color, hover = "#2196F3", "#1976D2"
            elif self.detected_type == ControllerType.ESP:
                color, hover = "#4CAF50", "#45a049"
            else:
                color, hover = "#9E9E9E", "#757575"

            self.flash_btn.setStyleSheet(f"""
                QPushButton {{
                    background-color: {color};
                    color: white;
                    border-radius: 5px;
                }}
                QPushButton:hover {{
                    background-color: {hover};
                }}
                QPushButton:disabled {{
                    background-color: #cccccc;
                    color: #666666;
                }}
            """)

        def refresh_ports(self):
            # Remember current selection
            current_port = self.port_combo.currentData()

            self.port_combo.clear()

            # Only show compatible devices (ADV or ESP)
            devices = find_all_devices()

            for port, ptype in devices:
                if ptype == ControllerType.ADV:
                    label = f"{port} (ADV Controller)"
                elif ptype == ControllerType.ESP:
                    label = f"{port} (ESP8266)"
                else:
                    continue  # Skip unknown devices
                self.port_combo.addItem(label, port)

            if self.port_combo.count() == 0:
                self.port_combo.addItem("No compatible devices found", "")

            # Try to restore previous selection, or auto-select first
            if current_port:
                for i in range(self.port_combo.count()):
                    if self.port_combo.itemData(i) == current_port:
                        self.port_combo.setCurrentIndex(i)
                        break

            self.update_detection()

        def on_port_changed(self):
            self.update_detection()

        def on_firmware_changed(self):
            self.update_detection()

        def update_detection(self):
            port = self.port_combo.currentData()
            firmware = self.fw_path.text()

            # Priority: port type (more reliable) > firmware type
            port_type = detect_port_type(port) if port else ControllerType.UNKNOWN
            fw_type = detect_firmware_type(firmware) if firmware and os.path.exists(firmware) else ControllerType.UNKNOWN

            # Use port type if known, otherwise fall back to firmware type
            if port_type != ControllerType.UNKNOWN:
                self.detected_type = port_type
            elif fw_type != ControllerType.UNKNOWN:
                self.detected_type = fw_type
            else:
                self.detected_type = ControllerType.UNKNOWN

            # Update device label with colored text
            if self.detected_type == ControllerType.ADV:
                self.device_label.setText("ADV Controller (HC32F460)")
                self.device_label.setStyleSheet("padding: 8px; color: #2196F3;")
            elif self.detected_type == ControllerType.ESP:
                self.device_label.setText("GBS-Control (ESP8266)")
                self.device_label.setStyleSheet("padding: 8px; color: #4CAF50;")
            else:
                self.device_label.setText("No device detected")
                self.device_label.setStyleSheet("padding: 8px; color: #666;")

            self.update_flash_button_style()

            # Enable flash button only if everything is ready
            has_port = bool(port and port != "")
            has_firmware = bool(firmware and os.path.exists(firmware))
            can_flash = (
                self.detected_type != ControllerType.UNKNOWN and
                has_port and
                has_firmware
            )
            self.flash_btn.setEnabled(can_flash)

        def browse_firmware(self):
            # Start in current directory or script directory
            start_dir = os.path.dirname(os.path.abspath(__file__))

            path, _ = QFileDialog.getOpenFileName(
                self,
                "Select Firmware File",
                start_dir,
                "Binary Files (*.bin);;All Files (*)"
            )
            if path:
                self.fw_path.setText(path)

        def log(self, msg: str):
            self.log_text.append(msg)
            self.log_text.verticalScrollBar().setValue(
                self.log_text.verticalScrollBar().maximum()
            )

        def start_flash(self):
            port = self.port_combo.currentData()
            firmware = self.fw_path.text()

            if not port or port == "":
                QMessageBox.warning(self, "Error", "Please select a serial port")
                return

            if not firmware or not os.path.exists(firmware):
                QMessageBox.warning(self, "Error", "Please select a valid firmware file")
                return

            if self.detected_type == ControllerType.UNKNOWN:
                QMessageBox.warning(self, "Error", "Could not detect device type")
                return

            self.flash_btn.setEnabled(False)
            self.port_combo.setEnabled(False)
            self.progress_bar.setValue(0)
            self.log_text.clear()

            self.log(f"[*] Flashing {self.detected_type.value}...")

            self.worker = FlashWorker(self.detected_type, port, firmware)
            self.worker.progress.connect(self.on_progress)
            self.worker.status.connect(self.on_status)
            self.worker.error.connect(self.on_error)
            self.worker.finished_signal.connect(self.on_finished)
            self.worker.start()

        def on_progress(self, current: int, total: int):
            pct = int(current * 100 / total) if total > 0 else 0
            self.progress_bar.setValue(pct)

        def on_status(self, msg: str):
            self.log(f"[*] {msg}")

        def on_error(self, msg: str):
            self.log(f"[ERROR] {msg}")

        def on_finished(self, success: bool):
            self.flash_btn.setEnabled(True)
            self.port_combo.setEnabled(True)

            if success:
                self.progress_bar.setValue(100)
                self.log("[✓] Flash complete!")
                QMessageBox.information(
                    self, "Success",
                    f"{self.detected_type.value}\nFirmware flashed successfully!"
                )
            else:
                QMessageBox.critical(
                    self, "Error",
                    "Flash failed. Check the log for details."
                )

    app = QApplication(sys.argv)
    app.setStyle('Fusion')
    window = MainWindow()
    window.show()
    sys.exit(app.exec())


# =============================================================================
# Main
# =============================================================================

def main():
    parser = argparse.ArgumentParser(
        description=f"GBSC Pro Flasher v{__version__} - Automatic device detection",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  GUI mode:
    python gbsc_flasher.py

  Auto-detect and flash:
    python gbsc_flasher.py firmware.bin
    python gbsc_flasher.py /dev/cu.usbmodem001 firmware.bin

  Force device type:
    python gbsc_flasher.py --adv firmware.bin
    python gbsc_flasher.py --esp COM3 firmware.bin

  List devices:
    python gbsc_flasher.py --list

Supported platforms: Windows, macOS, Linux
"""
    )

    parser.add_argument('port_or_firmware', nargs='?',
                        help='Serial port or firmware file (auto-detected)')
    parser.add_argument('firmware', nargs='?',
                        help='Firmware file (if port specified first)')
    parser.add_argument('--gui', action='store_true', help='Launch GUI mode')
    parser.add_argument('--adv', action='store_true',
                        help='Force ADV Controller mode')
    parser.add_argument('--esp', action='store_true',
                        help='Force ESP8266 mode')
    parser.add_argument('--list', action='store_true',
                        help='List detected devices')
    parser.add_argument('--version', action='version',
                        version=f'GBSC Pro Flasher v{__version__}')

    args = parser.parse_args()

    # List devices
    if args.list:
        print(f"\nGBSC Pro Flasher v{__version__}")
        print("\nDetected GBSC Pro devices:")
        print("-" * 40)
        devices = find_all_devices()
        if devices:
            for port, ptype in devices:
                print(f"  {port}: {ptype.value}")
        else:
            print("  No devices found")

        print("\nAll serial ports:")
        print("-" * 40)
        for port in list_serial_ports():
            ptype = detect_port_type(port)
            suffix = f" ({ptype.value})" if ptype != ControllerType.UNKNOWN else ""
            print(f"  {port}{suffix}")
        sys.exit(0)

    # Determine port and firmware from positional args
    port = None
    firmware = None

    if args.port_or_firmware:
        if os.path.exists(args.port_or_firmware):
            # It's a file
            firmware = args.port_or_firmware
            if args.firmware:
                # Error: two files provided
                print("Error: Cannot specify two firmware files")
                sys.exit(1)
        else:
            # It's a port
            port = args.port_or_firmware
            firmware = args.firmware

    # Force type if specified
    force_type = None
    if args.adv:
        force_type = ControllerType.ADV
    elif args.esp:
        force_type = ControllerType.ESP

    # CLI mode if firmware provided
    if firmware:
        sys.exit(run_cli(port, firmware, force_type))

    # Default: GUI mode
    run_gui()


if __name__ == "__main__":
    main()
