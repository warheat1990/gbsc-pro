"""
GBSC-Pro ADV CLI Simulator for Raspberry Pi Pico
Simulates the ADV7280/ADV7391 command-line interface via USB serial
"""

import sys
import gc

# Use utime on MicroPython, time elsewhere
try:
    import utime as time
except ImportError:
    import time

# Virtual I2C device memory
# Structure: {device_address: {map_value: {register_address: value}}}
devices = {
    0x42: {  # ADV7280
        0x00: {},  # User Sub Map
        0x20: {},  # Interrupt/VDP Map
        0x40: {},  # User Sub Map 2
    },
    0x56: {  # ADV7391
        0x00: {},  # Main map
    },
    0x84: {  # VPP (ADV7280)
        0x00: {},  # VPP map (no map switching)
    }
}

# Initialize registers with default values (0x00)
for device_addr in devices:
    for map_val in devices[device_addr]:
        for reg_addr in range(256):
            devices[device_addr][map_val][reg_addr] = 0x00

# Load realistic default values from real GBSC-Pro dump
# All 4 maps are 256 bytes total (full register address space 0x00-0xFF)
#
# ADV7280 (0x42) - User Sub Map (0x00)
# From: dm 42 00 00 FF [users_sub_map] - 255 bytes starting at offset 0x00
adv7280_user_sub_map_data = [
    0x09, 0xC8, 0x04, 0x0C, 0x07, 0x00, 0x02, 0xFF, 0x80, 0x80, 0x00, 0x00, 0x36, 0x7C, 0x00, 0x00,
    0x12, 0x43, 0x00, 0x14, 0x15, 0x04, 0x00, 0x49, 0x93, 0xF1, 0x20, 0x00, 0x30, 0x40, 0x08, 0x10,
    0x00, 0x02, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x58, 0x10, 0x00, 0x00, 0xE1, 0xAE, 0xF2, 0x00, 0xF4,
    0x00, 0x02, 0x41, 0x84, 0x00, 0x02, 0x00, 0x81, 0x24, 0x24, 0x00, 0x44, 0x58, 0x22, 0x64, 0xE4,
    0x90, 0x01, 0x7E, 0xA4, 0xFF, 0xB6, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF, 0x08, 0x08,
    0x08, 0x24, 0xCD, 0x4E, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0xD0, 0x06, 0x00, 0x6D, 0x6D, 0xA0,
    0x10, 0x00, 0x20, 0xD0, 0x10, 0x06, 0x28, 0x03, 0x01, 0x00, 0x00, 0x11, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x51, 0x51, 0x00, 0x00, 0x0C, 0x42, 0x03, 0x63, 0x5A, 0x08, 0x10, 0x00, 0x40, 0x24, 0x48, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x90,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x81, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x69, 0x00, 0x01, 0xB4,
    0x00, 0x10, 0xFF, 0xFF, 0x7F, 0x7F, 0x3E, 0x08, 0x3C, 0x08, 0x3C, 0x9B, 0xAC, 0x4C, 0x00, 0x00,
    0x14, 0x80, 0x80, 0x80, 0x80, 0x25, 0x04, 0x63, 0x65, 0x14, 0x63, 0x55, 0x55, 0x00, 0x00, 0x4A,
    0x44, 0x0C, 0x32, 0x00, 0x15, 0xE0, 0x69, 0x10, 0x00, 0x03, 0xA0, 0x40, 0x04, 0x84, 0x00,
]
for i, val in enumerate(adv7280_user_sub_map_data):
    devices[0x42][0x00][0x00 + i] = val

# ADV7280 (0x42) - User Sub Map 2 (0x40)
# From: dm 42 40 80 69 [user_sub_map_2] - 105 bytes starting at offset 0x80
adv7280_user_sub_map_2_data = [
    0x00, 0xA0, 0x0A, 0x0D, 0x88, 0xF8, 0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x07, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0xCC, 0xCC, 0x00,
    0x01, 0x11, 0x88, 0x1B, 0xD7, 0x23, 0x10, 0x00, 0x00,
]
for i, val in enumerate(adv7280_user_sub_map_2_data):
    devices[0x42][0x40][0x80 + i] = val

# ADV7391 (0x56) - Main map
# From: d 56 00 BC [main] - 188 bytes starting at offset 0x00
adv7391_main_data = [
    0x9C, 0x70, 0x20, 0x03, 0xF0, 0x4E, 0x0E, 0x24, 0x92, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x1B, 0x00, 0x01, 0x80, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x1C, 0x01, 0x00, 0x68, 0x48, 0x00, 0xA0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x0B, 0x04, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x08, 0x00,
    0x1F, 0x7C, 0xF0, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6A,
]
for i, val in enumerate(adv7391_main_data):
    devices[0x56][0x00][0x00 + i] = val

# VPP (0x84) - VPP map
# From: d 84 41 1B [vpp_main] - 26 bytes starting at offset 0x41
vpp_data = [
    0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x40, 0x60, 0x80, 0x02,
]
for i, val in enumerate(vpp_data):
    devices[0x84][0x00][0x41 + i] = val

# Current map register (0x0E) for each device
current_map = {
    0x42: 0x00,  # ADV7280 starts at User Sub Map
    0x56: 0x00,  # ADV7391 (no map switching, but keep track)
}

def write_char(char):
    """Write a single character to stdout (with echo)."""
    sys.stdout.write(char)

def write_str(s):
    """Write a string to stdout."""
    sys.stdout.write(s)

def parse_hex(s):
    """Parse hexadecimal string to integer."""
    try:
        return int(s, 16)
    except:
        return None

def format_hex(value, width=2):
    """Format integer as hexadecimal string."""
    if width == 2:
        return "{:02X}".format(value)
    else:
        return ("{:0" + str(width) + "X}").format(value)

def read_register(device_addr, map_val, reg_addr, use_map_switching=True):
    """
    Read register from virtual device.

    Args:
        device_addr: I2C device address
        map_val: Register map value (ignored if use_map_switching=False)
        reg_addr: Register address
        use_map_switching: Whether to use map switching (default True)

    Returns:
        Register value (0-255) or None if error
    """
    if device_addr not in devices:
        return None

    if use_map_switching:
        # Use specified map
        if map_val not in devices[device_addr]:
            return None
        return devices[device_addr][map_val].get(reg_addr, 0x00)
    else:
        # No map switching - use first/default map
        maps = list(devices[device_addr].keys())
        if not maps:
            return None
        return devices[device_addr][maps[0]].get(reg_addr, 0x00)

def write_register(device_addr, map_val, reg_addr, value, use_map_switching=True):
    """
    Write register to virtual device.

    Args:
        device_addr: I2C device address
        map_val: Register map value (ignored if use_map_switching=False)
        reg_addr: Register address
        value: Value to write (0-255)
        use_map_switching: Whether to use map switching (default True)

    Returns:
        True if success, False if error
    """
    if device_addr not in devices:
        return False

    if use_map_switching:
        # Special handling for map register (0x0E)
        if reg_addr == 0x0E and device_addr in current_map:
            current_map[device_addr] = value

        if map_val not in devices[device_addr]:
            return False
        devices[device_addr][map_val][reg_addr] = value & 0xFF
    else:
        # No map switching - use first/default map
        maps = list(devices[device_addr].keys())
        if not maps:
            return False
        devices[device_addr][maps[0]][reg_addr] = value & 0xFF

    return True

def process_command(cmd_str):
    """
    Process a command string and return response.

    Args:
        cmd_str: Command string (e.g., "rm 42 00 10")

    Returns:
        Response string ("OK", "ERR", or hex value)
    """
    parts = cmd_str.strip().split()
    if not parts:
        return "ERR"

    cmd = parts[0].lower()

    try:
        # Simulate processing delay (50-200ms)
        delay = 50 + (int(time.ticks_ms()) % 150)
        time.sleep_ms(delay)

        if cmd == 'rm' and len(parts) == 4:
            # Read with map: rm AA MM RR
            device_addr = parse_hex(parts[1])
            map_val = parse_hex(parts[2])
            reg_addr = parse_hex(parts[3])

            if device_addr is None or map_val is None or reg_addr is None:
                return "ERR"

            value = read_register(device_addr, map_val, reg_addr, use_map_switching=True)
            if value is None:
                return "ERR"

            return format_hex(value, 2)

        elif cmd == 'r' and len(parts) == 3:
            # Read without map: r AA RR
            device_addr = parse_hex(parts[1])
            reg_addr = parse_hex(parts[2])

            if device_addr is None or reg_addr is None:
                return "ERR"

            value = read_register(device_addr, 0x00, reg_addr, use_map_switching=False)
            if value is None:
                return "ERR"

            return format_hex(value, 2)

        elif cmd == 'wm' and len(parts) == 5:
            # Write with map: wm AA MM RR VV
            device_addr = parse_hex(parts[1])
            map_val = parse_hex(parts[2])
            reg_addr = parse_hex(parts[3])
            val = parse_hex(parts[4])

            if device_addr is None or map_val is None or reg_addr is None or val is None:
                return "ERR"

            success = write_register(device_addr, map_val, reg_addr, val, use_map_switching=True)
            return "OK" if success else "ERR"

        elif cmd == 'w' and len(parts) == 4:
            # Write without map: w AA RR VV
            device_addr = parse_hex(parts[1])
            reg_addr = parse_hex(parts[2])
            val = parse_hex(parts[3])

            if device_addr is None or reg_addr is None or val is None:
                return "ERR"

            success = write_register(device_addr, 0x00, reg_addr, val, use_map_switching=False)
            return "OK" if success else "ERR"

        elif cmd == 'dm' and len(parts) == 5:
            # Dump with map: dm AA MM RR QQ
            device_addr = parse_hex(parts[1])
            map_val = parse_hex(parts[2])
            start_reg = parse_hex(parts[3])
            count = parse_hex(parts[4])

            if device_addr is None or map_val is None or start_reg is None or count is None:
                return "ERR"

            if count == 0 or count > 256:
                return "ERR"

            values = []
            for i in range(count):
                reg_addr = (start_reg + i) & 0xFF
                value = read_register(device_addr, map_val, reg_addr, use_map_switching=True)
                if value is None:
                    return "ERR"
                values.append(format_hex(value, 2))

            return " ".join(values)

        elif cmd == 'd' and len(parts) == 4:
            # Dump without map: d AA RR QQ
            device_addr = parse_hex(parts[1])
            start_reg = parse_hex(parts[2])
            count = parse_hex(parts[3])

            if device_addr is None or start_reg is None or count is None:
                return "ERR"

            if count == 0 or count > 256:
                return "ERR"

            values = []
            for i in range(count):
                reg_addr = (start_reg + i) & 0xFF
                value = read_register(device_addr, 0x00, reg_addr, use_map_switching=False)
                if value is None:
                    return "ERR"
                values.append(format_hex(value, 2))

            return " ".join(values)

        else:
            return "ERR"

    except Exception as e:
        # DEBUG: uncomment to see errors
        # write_str(f"\r\n[ERROR: {e}]\r\n")
        return "ERR"

def main():
    """Main loop - simulate GBSC-Pro ADV CLI."""

    # Setup polling for stdin
    try:
        import uselect
        poller = uselect.poll()
        poller.register(sys.stdin, uselect.POLLIN)
        has_poller = True
    except:
        has_poller = False

    # Buffer for incoming command
    cmd_buffer = ""

    # Print initial prompt
    write_str("> ")

    while True:
        # Check for incoming character
        char = None

        if has_poller:
            # Use uselect polling (MicroPython)
            if poller.poll(0):
                char = sys.stdin.read(1)
        else:
            # Blocking read fallback
            try:
                char = sys.stdin.read(1)
            except:
                pass

        if char:
            # Echo the character immediately (just like real HC32)
            write_char(char)

            if char == '\r' or char == '\n':
                # Command complete - process it
                if cmd_buffer.strip():
                    # Newline after echo
                    write_str("\r\n")

                    # Process command and get response
                    # DEBUG: print command being processed
                    # write_str(f"[DEBUG: cmd='{cmd_buffer}']")
                    response = process_command(cmd_buffer)

                    # Write response
                    write_str(response)
                    write_str("\r\n")

                # Clear buffer and show new prompt
                cmd_buffer = ""
                write_str("> ")

            elif char == '\x08' or char == '\x7f':  # Backspace or DEL
                # Remove last character from buffer
                if cmd_buffer:
                    cmd_buffer = cmd_buffer[:-1]
                    # Erase character on terminal (backspace + space + backspace)
                    write_str("\x08 \x08")

            else:
                # Add to buffer
                cmd_buffer += char

        # Small delay to prevent CPU overload
        time.sleep_ms(1)

        # Periodic garbage collection
        if int(time.ticks_ms()) % 10000 == 0:
            gc.collect()

# Entry point
if __name__ == '__main__':
    try:
        print("GBSC-Pro ADV CLI Simulator v1.0")
        print("Ready for commands (r/w/rm/wm/d/dm)")
        print("")
        main()
    except KeyboardInterrupt:
        print("\r\nSimulator stopped")
    except Exception as e:
        print(f"\r\nError: {e}")
