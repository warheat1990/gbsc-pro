"""
Serial communication bridge with threading support.
"""

import serial
import time
import threading
from queue import Queue, Empty
from typing import Optional, Callable, List, Dict, Any
from enum import Enum
from PyQt6.QtCore import QObject, pyqtSignal
from .utils.logger import get_logger

class CommandType(Enum):
    """Types of serial commands."""
    READ = "read"
    WRITE = "write"
    DUMP = "dump"
    SEQUENCE = "sequence"


class SerialCommand:
    """Represents a serial command to be executed."""

    def __init__(self, cmd_type: CommandType, device_addr: int,
                 register: int = 0, value: int = 0,
                 callback: Optional[Callable] = None,
                 sequence: Optional[List[Dict[str, int]]] = None,
                 map_value: int = 0x00,
                 use_map_switching: bool = True):
        """
        Initialize a serial command.

        Args:
            cmd_type: Type of command
            device_addr: I2C device address
            register: Register address
            value: Value to write (for WRITE commands)
            callback: Callback function to call with result
            map_value: Register map value (0x00=User Sub Map, 0x20=Int/VDP, 0x40=User Sub Map 2)
            sequence: List of steps for SEQUENCE commands
            use_map_switching: Whether to use map switching commands (rm/wm/dm) or simple commands (r/w/d)
        """
        self.cmd_type = cmd_type
        self.device_addr = device_addr
        self.register = register
        self.value = value
        self.callback = callback
        self.sequence = sequence or []
        self.map_value = map_value
        self.use_map_switching = use_map_switching


class SerialBridge(QObject):
    """
    Thread-safe serial communication bridge.

    Handles serial communication in a background thread to avoid blocking the UI.
    """

    # Signals for thread-safe callback execution
    write_completed = pyqtSignal(object, bool)  # (callback, success)
    read_completed = pyqtSignal(object, object)  # (callback, value)
    dump_completed = pyqtSignal(object, object)  # (callback, values)

    def __init__(self, port: str = 'COM3', baudrate: int = 115200):
        """
        Initialize the serial bridge.

        Args:
            port: Serial port name
            baudrate: Baud rate
        """
        super().__init__()  # Initialize QObject
        self.port = port
        self.baudrate = baudrate
        self.serial: Optional[serial.Serial] = None
        self.logger = get_logger(__name__)

        # Threading
        self.command_queue: Queue = Queue()
        self.worker_thread: Optional[threading.Thread] = None
        self.running = False
        self.connected = False

        # Statistics
        self.stats = {
            'commands_sent': 0,
            'commands_failed': 0,
            'bytes_sent': 0,
            'bytes_received': 0
        }

        # Connect signals to slots for thread-safe callback execution
        self.write_completed.connect(self._handle_write_completed)
        self.read_completed.connect(self._handle_read_completed)
        self.dump_completed.connect(self._handle_dump_completed)

    def _handle_write_completed(self, callback, success):
        """Handle write completion in main thread (slot for write_completed signal)."""
        if callback:
            callback(success)

    def _handle_read_completed(self, callback, value):
        """Handle read completion in main thread (slot for read_completed signal)."""
        if callback:
            callback(value)

    def _handle_dump_completed(self, callback, values):
        """Handle dump completion in main thread (slot for dump_completed signal)."""
        if callback:
            callback(values)

    def connect(self) -> bool:
        """
        Connect to the serial port and start worker thread.

        Returns:
            True if successful, False otherwise
        """
        try:
            self.serial = serial.Serial(
                self.port,
                self.baudrate,
                timeout=1,
                write_timeout=1
            )
            self.connected = True
            self.logger.info(f"Connected to {self.port} at {self.baudrate} baud")

            # Start worker thread
            self.running = True
            self.worker_thread = threading.Thread(target=self._worker, daemon=True)
            self.worker_thread.start()

            return True
        except Exception as e:
            self.logger.error(f"Failed to connect to {self.port}: {e}")
            self.connected = False
            return False

    def disconnect(self):
        """Disconnect from the serial port and stop worker thread."""
        self.running = False

        # Wait for worker thread to finish
        if self.worker_thread and self.worker_thread.is_alive():
            self.worker_thread.join(timeout=2.0)

        if self.serial:
            try:
                self.serial.close()
                self.logger.info(f"Disconnected from {self.port}")
            except Exception as e:
                self.logger.error(f"Error closing serial port: {e}")
            finally:
                self.serial = None

        self.connected = False

    def is_connected(self) -> bool:
        """Check if connected to serial port."""
        return self.connected and self.serial is not None

    def write_register(self, device_addr: int, map_value: int, register: int, value: int,
                      callback: Optional[Callable[[bool], None]] = None, use_map_switching: bool = True):
        """
        Queue a write register command.

        Args:
            device_addr: I2C device address
            map_value: Register map value (0x00, 0x20, 0x40)
            register: Register address
            value: Value to write
            callback: Optional callback function called with success status
        """
        cmd = SerialCommand(
            CommandType.WRITE,
            device_addr,
            register,
            value,
            callback,
            map_value=map_value,
            use_map_switching=use_map_switching
        )
        self.command_queue.put(cmd)

    def read_register(self, device_addr: int, map_value: int, register: int,
                     callback: Optional[Callable[[Optional[int]], None]] = None, use_map_switching: bool = True):
        """
        Queue a read register command.

        Args:
            device_addr: I2C device address
            map_value: Register map value (0x00, 0x20, 0x40)
            register: Register address
            callback: Optional callback function called with register value
        """
        cmd = SerialCommand(
            CommandType.READ,
            device_addr,
            register,
            callback=callback,
            map_value=map_value,
            use_map_switching=use_map_switching
        )
        self.command_queue.put(cmd)

    def read_registers_dump(self, device_addr: int, map_value: int, start_register: int, count: int,
                           callback: Optional[Callable[[Optional[List[int]]], None]] = None, use_map_switching: bool = True):
        """
        Queue a dump command to read multiple consecutive registers.

        Args:
            device_addr: I2C device address
            map_value: Register map value (0x00, 0x20, 0x40)
            start_register: Starting register address
            count: Number of registers to read
            callback: Optional callback function called with list of register values
        """
        cmd = SerialCommand(
            CommandType.DUMP,
            device_addr,
            start_register,
            value=count,  # Reuse value field for count
            callback=callback,
            map_value=map_value,
            use_map_switching=use_map_switching
        )
        self.command_queue.put(cmd)

    def execute_sequence(self, device_addr: int, sequence: List[Dict[str, int]],
                        callback: Optional[Callable[[bool], None]] = None):
        """
        Queue an access sequence command.

        Args:
            device_addr: I2C device address
            sequence: List of steps with 'register' and 'value' keys
            callback: Optional callback function called with success status
        """
        cmd = SerialCommand(
            CommandType.SEQUENCE,
            device_addr,
            sequence=sequence,
            callback=callback
        )
        self.command_queue.put(cmd)

    def _worker(self):
        """Worker thread that processes commands from the queue."""
        self.logger.info("Serial worker thread started")

        while self.running:
            try:
                # Get command with timeout
                cmd = self.command_queue.get(timeout=0.1)

                # Execute command
                if cmd.cmd_type == CommandType.WRITE:
                    success = self._write_register_sync(cmd.device_addr, cmd.map_value, cmd.register, cmd.value, cmd.use_map_switching)
                    if cmd.callback:
                        # Emit signal for thread-safe callback execution
                        self.write_completed.emit(cmd.callback, success)

                elif cmd.cmd_type == CommandType.READ:
                    value = self._read_register_sync(cmd.device_addr, cmd.map_value, cmd.register, cmd.use_map_switching)
                    if cmd.callback:
                        # Emit signal for thread-safe callback execution
                        self.read_completed.emit(cmd.callback, value)

                elif cmd.cmd_type == CommandType.DUMP:
                    values = self._read_registers_dump_sync(cmd.device_addr, cmd.map_value, cmd.register, cmd.value, cmd.use_map_switching)
                    if cmd.callback:
                        # Emit signal for thread-safe callback execution
                        self.dump_completed.emit(cmd.callback, values)

                elif cmd.cmd_type == CommandType.SEQUENCE:
                    success = self._execute_sequence_sync(cmd.device_addr, cmd.sequence)
                    if cmd.callback:
                        # Emit signal for thread-safe callback execution
                        self.write_completed.emit(cmd.callback, success)

                self.command_queue.task_done()

            except Empty:
                # No commands in queue, continue
                continue
            except Exception as e:
                self.logger.error(f"Error in worker thread: {e}")
                self.stats['commands_failed'] += 1

        self.logger.info("Serial worker thread stopped")

    def _write_register_sync(self, device_addr: int, map_value: int, register: int, value: int, use_map_switching: bool = True) -> bool:
        """
        Synchronously write to a register (runs in worker thread).

        Args:
            device_addr: I2C device address
            map_value: Register map value (0x00, 0x20, 0x40)
            register: Register address
            value: Value to write

        Returns:
            True if successful, False otherwise
        """
        if not self.serial:
            return False

        try:
            # Flush any pending input
            self.serial.reset_input_buffer()

            # Send newline to clear prompt
            self.serial.write(b'\r\n')
            time.sleep(0.1)
            self.serial.reset_input_buffer()

            time.sleep(0.05)

            # Choose command based on use_map_switching parameter
            # If False: use simple w command (no register 0x0E switching) - for VPP and ADV7391
            # If True: use wm command (with register 0x0E switching) - for normal maps
            if not use_map_switching:
                # Send command: "w AA RR VV\r\n" (no map parameter)
                cmd_str = f"w {device_addr:02x} {register:02x} {value:02x}\r\n"
            else:
                # Send command: "wm AA MM RR VV\r\n" (with map parameter)
                cmd_str = f"wm {device_addr:02x} {map_value:02x} {register:02x} {value:02x}\r\n"

            for char in cmd_str:
                self.serial.write(char.encode())
                self.serial.flush()
                self.stats['bytes_sent'] += 1

                # Wait for echo of this character (with timeout)
                echo_start = time.time()
                echo_received = False
                while time.time() - echo_start < 0.5:  # 500ms timeout per character
                    if self.serial.in_waiting > 0:
                        echo_byte = self.serial.read(1)
                        self.stats['bytes_received'] += 1
                        echo_received = True
                        break
                    time.sleep(0.001)  # 1ms poll interval

                if not echo_received:
                    self.logger.warning(f"No echo received for character '{char}'")
                    break

            self.stats['commands_sent'] += 1

            # Wait for response: OK or ERR
            start_time = time.time()
            timeout = 2.0
            accumulated_data = b''
            last_char_time = time.time()

            while time.time() - start_time < timeout:
                if self.serial.in_waiting > 0:
                    byte = self.serial.read(1)
                    accumulated_data += byte
                    self.stats['bytes_received'] += 1
                    last_char_time = time.time()

                    try:
                        decoded = accumulated_data.decode('utf-8', errors='ignore')
                        lines = [line.strip() for line in decoded.split('\n') if line.strip()]

                        for line in lines:
                            # Skip echo of command
                            if line.lower().startswith('w '):
                                continue
                            # Skip prompt
                            if line == '>':
                                continue
                            # Check for response
                            if line == "OK":
                                return True
                            elif line == "ERR":
                                self.logger.warning(f"Write failed: ERR")
                                self.stats['commands_failed'] += 1
                                return False

                    except Exception as e:
                        self.logger.debug(f"Parse error: {e}")
                        pass
                else:
                    if len(accumulated_data) > 0 and (time.time() - last_char_time) > 0.3:
                        break
                    time.sleep(0.01)

            # Timeout
            decoded = accumulated_data.decode('utf-8', errors='ignore')
            self.logger.warning(f"Write timeout: {repr(decoded)}")
            self.stats['commands_failed'] += 1
            return False

        except Exception as e:
            self.logger.error(f"Write error: {e}")
            self.stats['commands_failed'] += 1
            return False

    def _read_register_sync(self, device_addr: int, map_value: int, register: int, use_map_switching: bool = True) -> Optional[int]:
        """
        Synchronously read from a register (runs in worker thread).

        Args:
            device_addr: I2C device address
            map_value: Register map value (0x00, 0x20, 0x40)
            register: Register address

        Returns:
            Register value if successful, None otherwise
        """
        if not self.serial:
            return None

        try:
            # Flush any pending input (device might send unsolicited messages)
            self.serial.reset_input_buffer()

            # Send a newline first to clear any pending prompt state
            self.serial.write(b'\r\n')
            time.sleep(0.1)  # Wait for response
            self.serial.reset_input_buffer()  # Clear the response (likely just ">")

            time.sleep(0.05)  # Small delay before command

            # Choose command based on use_map_switching parameter
            # If False: use simple r command (no register 0x0E switching) - for VPP and ADV7391
            # If True: use rm command (with register 0x0E switching) - for normal maps
            if not use_map_switching:
                # Send command: "r AA RR\r\n" (no map parameter)
                cmd_str = f"r {device_addr:02x} {register:02x}\r\n"
            else:
                # Send command: "rm AA MM RR\r\n" (with map parameter)
                cmd_str = f"rm {device_addr:02x} {map_value:02x} {register:02x}\r\n"

            self.logger.debug(f"Sending command: {repr(cmd_str)}")

            for char in cmd_str:
                self.serial.write(char.encode())
                self.serial.flush()
                self.stats['bytes_sent'] += 1

                # Wait for echo of this character (with timeout)
                echo_start = time.time()
                echo_received = False
                while time.time() - echo_start < 0.5:  # 500ms timeout per character
                    if self.serial.in_waiting > 0:
                        echo_byte = self.serial.read(1)
                        self.stats['bytes_received'] += 1
                        echo_received = True
                        break
                    time.sleep(0.001)  # 1ms poll interval

                if not echo_received:
                    self.logger.warning(f"No echo received for character '{char}'")
                    break

            self.stats['commands_sent'] += 1

            # Read response character by character until we get a valid answer
            # The CLI echoes each character as it's sent, then responds
            accumulated_data = b''
            start_time = time.time()
            timeout = 2.0  # 2 second total timeout
            last_char_time = start_time

            self.logger.debug(f"Waiting for response to: {cmd_str.strip()}")

            # Keep reading until timeout or we find a valid response
            while time.time() - start_time < timeout:
                # Read one byte at a time
                if self.serial.in_waiting > 0:
                    byte = self.serial.read(1)
                    accumulated_data += byte
                    self.stats['bytes_received'] += 1
                    last_char_time = time.time()

                    # Try to decode and check if we have a complete response
                    try:
                        decoded = accumulated_data.decode('utf-8', errors='ignore')

                        # Split into lines to check for response
                        lines = [line.strip() for line in decoded.split('\n') if line.strip()]

                        # Look for a 2-digit hex response in the lines
                        for line in lines:
                            # Skip echo of command
                            if line.lower().startswith('r '):
                                continue

                            # Skip prompt
                            if line == '>':
                                continue

                            # Check for hex value (exactly 2 hex digits)
                            if len(line) == 2 and all(c in '0123456789ABCDEFabcdef' for c in line):
                                value = int(line, 16)
                                return value

                            # Check for error
                            if line == "ERR":
                                self.logger.warning(f"Device returned ERR for register 0x{register:02X} (received: {repr(decoded)})")
                                self.stats['commands_failed'] += 1
                                return None
                    except:
                        pass  # Not enough data yet, keep reading
                else:
                    # No data available - check if we've been idle too long
                    if len(accumulated_data) > 0 and (time.time() - last_char_time) > 0.3:
                        # No new data for 300ms, probably done
                        break
                    time.sleep(0.01)  # Small delay before checking again

            # Timeout or end of data - try to parse what we have
            if len(accumulated_data) > 0:
                try:
                    decoded = accumulated_data.decode('utf-8', errors='ignore')
                    self.logger.warning(f"Timeout/incomplete response for 0x{register:02X}: {repr(decoded)}")
                except:
                    self.logger.warning(f"Timeout with {len(accumulated_data)} bytes of unparseable data")
            else:
                self.logger.warning(f"No data received from device for register 0x{register:02X}")

            self.stats['commands_failed'] += 1
            return None

        except Exception as e:
            self.logger.error(f"Read error: {e}")
            self.stats['commands_failed'] += 1
            return None

    def _read_registers_dump_sync(self, device_addr: int, map_value: int, start_register: int, count: int, use_map_switching: bool = True) -> Optional[List[int]]:
        """
        Synchronously read multiple consecutive registers using dump command.

        Args:
            device_addr: I2C device address
            map_value: Register map value (0x00, 0x20, 0x40)
            start_register: Starting register address
            count: Number of registers to read

        Returns:
            List of register values if successful, None otherwise
        """
        if not self.serial:
            return None

        try:
            # Flush any pending input
            self.serial.reset_input_buffer()

            # Send newline to clear prompt
            self.serial.write(b'\r\n')
            time.sleep(0.1)
            self.serial.reset_input_buffer()

            time.sleep(0.05)

            # Choose command based on use_map_switching parameter
            # If False: use simple d command (no register 0x0E switching) - for VPP and ADV7391
            # If True: use dm command (with register 0x0E switching) - for normal maps
            if not use_map_switching:
                # Send command: "d AA RR QQ\r\n" (no map parameter)
                cmd_str = f"d {device_addr:02x} {start_register:02x} {count:02x}\r\n"
            else:
                # Send command: "dm AA MM RR QQ\r\n" (with map parameter)
                cmd_str = f"dm {device_addr:02x} {map_value:02x} {start_register:02x} {count:02x}\r\n"

            self.logger.info(f"Serial: Sending dump: {repr(cmd_str)}")

            for char in cmd_str:
                self.serial.write(char.encode())
                self.serial.flush()
                self.stats['bytes_sent'] += 1

                # Wait for echo of this character (with timeout)
                echo_start = time.time()
                echo_received = False
                while time.time() - echo_start < 0.5:  # 500ms timeout per character
                    if self.serial.in_waiting > 0:
                        echo_byte = self.serial.read(1)
                        self.stats['bytes_received'] += 1
                        echo_received = True
                        break
                    time.sleep(0.001)  # 1ms poll interval

                if not echo_received:
                    self.logger.warning(f"No echo received for character '{char}' in dump command")
                    break

            self.stats['commands_sent'] += 1

            # Read response character by character
            accumulated_data = b''
            start_time = time.time()
            # Scale timeout based on count: 3s base + 20ms per register
            timeout = max(3.0, 3.0 + (count * 0.02))
            last_char_time = start_time
            self.logger.debug(f"Dump timeout set to {timeout:.1f}s for {count} registers")

            self.logger.debug(f"Waiting for dump response...")

            while time.time() - start_time < timeout:
                if self.serial.in_waiting > 0:
                    byte = self.serial.read(1)
                    accumulated_data += byte
                    self.stats['bytes_received'] += 1
                    last_char_time = time.time()

                    try:
                        decoded = accumulated_data.decode('utf-8', errors='ignore')
                        lines = [line.strip() for line in decoded.split('\n') if line.strip()]

                        # Accumulate all hex values from all lines
                        all_hex_values = []
                        for line in lines:
                            # Skip echo of command
                            if line.lower().startswith('d '):
                                continue

                            # Skip prompt
                            if line == '>':
                                continue

                            # Check for error
                            if line == "ERR":
                                self.logger.warning(f"Dump failed: ERR")
                                self.stats['commands_failed'] += 1
                                return None

                            # Extract hex values from this line
                            parts = line.split()
                            for part in parts:
                                if len(part) == 2 and all(c in '0123456789ABCDEFabcdef' for c in part):
                                    all_hex_values.append(int(part, 16))

                        # Check if we have all the values we expected
                        if len(all_hex_values) >= count:
                            values = all_hex_values[:count]  # Take only what we need
                            return values

                    except Exception as e:
                        self.logger.debug(f"Parse error: {e}")
                        pass
                else:
                    if len(accumulated_data) > 0 and (time.time() - last_char_time) > 0.3:
                        break
                    time.sleep(0.01)

            # Try final parse of accumulated data
            if len(accumulated_data) > 0:
                try:
                    decoded = accumulated_data.decode('utf-8', errors='ignore')
                    lines = [line.strip() for line in decoded.split('\n') if line.strip()]

                    all_hex_values = []
                    for line in lines:
                        if line.lower().startswith('d '):
                            continue
                        if line == '>':
                            continue
                        if line == "ERR":
                            self.logger.warning(f"Dump failed: ERR")
                            self.stats['commands_failed'] += 1
                            return None

                        parts = line.split()
                        for part in parts:
                            if len(part) == 2 and all(c in '0123456789ABCDEFabcdef' for c in part):
                                all_hex_values.append(int(part, 16))

                    if len(all_hex_values) >= count:
                        values = all_hex_values[:count]
                        return values

                    self.logger.warning(f"Dump incomplete: got {len(all_hex_values)}/{count} values")
                except Exception as e:
                    self.logger.warning(f"Failed to parse dump response: {e}")
            else:
                self.logger.warning(f"No data received for dump")

            self.stats['commands_failed'] += 1
            return None

        except Exception as e:
            self.logger.error(f"Dump error: {e}")
            self.stats['commands_failed'] += 1
            return None

    def _execute_sequence_sync(self, device_addr: int, sequence: List[Dict[str, int]]) -> bool:
        """
        Synchronously execute a sequence of writes (runs in worker thread).

        Args:
            device_addr: I2C device address
            sequence: List of steps with 'register' and 'value' keys

        Returns:
            True if all writes successful, False otherwise
        """
        for step in sequence:
            # Get register and value, converting strings to integers if needed
            register = step.get('register', 0)
            if isinstance(register, str):
                register = int(register, 0)  # Support hex strings like '0x0E'

            value = step.get('value', 0)
            if isinstance(value, str):
                value = int(value, 0)  # Support hex strings

            success = self._write_register_sync(
                device_addr,
                register,
                value
            )
            if not success:
                return False
            time.sleep(0.01)  # Small delay between writes
        return True

    def get_stats(self) -> Dict[str, int]:
        """Get communication statistics."""
        return self.stats.copy()

    def clear_stats(self):
        """Clear communication statistics."""
        self.stats = {
            'commands_sent': 0,
            'commands_failed': 0,
            'bytes_sent': 0,
            'bytes_received': 0
        }

    def __del__(self):
        """Cleanup on deletion."""
        self.disconnect()
