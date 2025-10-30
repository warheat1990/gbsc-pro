"""
Input validation utilities.
"""

import re
from typing import Optional


def validate_hex(value: str, min_val: int = 0, max_val: int = 0xFF) -> Optional[int]:
    """
    Validate and parse a hexadecimal string.

    Args:
        value: String to validate (e.g., "0x42", "42", "A5")
        min_val: Minimum allowed value
        max_val: Maximum allowed value

    Returns:
        Parsed integer value, or None if invalid
    """
    try:
        # Remove whitespace
        value = value.strip()

        # Try parsing as hex
        if value.lower().startswith('0x'):
            parsed = int(value, 16)
        elif re.match(r'^[0-9A-Fa-f]+$', value):
            parsed = int(value, 16)
        else:
            # Try as decimal
            parsed = int(value, 10)

        # Validate range
        if min_val <= parsed <= max_val:
            return parsed
        return None
    except (ValueError, TypeError):
        return None


def validate_i2c_address(address: int) -> bool:
    """
    Validate an I2C address.

    Args:
        address: I2C address to validate

    Returns:
        True if valid, False otherwise
    """
    # I2C addresses are 7-bit, so valid range is 0x08 to 0x77
    # (0x00-0x07 and 0x78-0x7F are reserved)
    return 0x08 <= address <= 0x77


def validate_register_value(value: int) -> bool:
    """
    Validate a register value (8-bit).

    Args:
        value: Register value to validate

    Returns:
        True if valid, False otherwise
    """
    return 0 <= value <= 0xFF


def validate_port_name(port: str) -> bool:
    """
    Validate a serial port name.

    Args:
        port: Port name to validate

    Returns:
        True if valid, False otherwise
    """
    # Common patterns for serial ports
    patterns = [
        r'^COM\d+$',  # Windows: COM1, COM2, etc.
        r'^/dev/tty.*$',  # Linux/Mac: /dev/ttyUSB0, /dev/ttyACM0, etc.
        r'^/dev/cu\..*$',  # Mac: /dev/cu.usbserial, etc.
    ]
    return any(re.match(pattern, port) for pattern in patterns)


def sanitize_filename(filename: str) -> str:
    """
    Sanitize a filename by removing invalid characters.

    Args:
        filename: Filename to sanitize

    Returns:
        Sanitized filename
    """
    # Remove invalid characters
    invalid_chars = r'[<>:"/\\|?*]'
    sanitized = re.sub(invalid_chars, '_', filename)

    # Remove leading/trailing spaces and dots
    sanitized = sanitized.strip('. ')

    return sanitized or 'unnamed'
