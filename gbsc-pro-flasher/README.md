# GBSC Pro Flasher

Cross-platform firmware flash tool for GBSC Pro with automatic device detection.

![GBSC Pro Logo](gbsc-pro-logo.png)

## Features

- **Automatic Detection**: Automatically identifies connected devices and firmware type
- **Dual Device Support**:
  - ADV Controller (HC32F460) via YMODEM protocol
  - GBS-Control (ESP8266) via esptool
- **GUI and CLI Modes**: Use whichever fits your workflow
- **Cross-Platform**: Works on Windows, macOS, and Linux
- **USB Hotplug Detection**: Automatically refreshes when devices are connected/disconnected
- **Drag & Drop**: Drop firmware files directly into the GUI

## Supported Devices

| Device | Chip | Protocol | USB Identifier |
|--------|------|----------|----------------|
| ADV Controller | HC32F460 | YMODEM | VID: 0x2E88, PID: 0x4603 (XHSC) |
| GBS-Control | ESP8266 | esptool | CH340 (VID: 0x1A86) or CP210x |

## Requirements

- Python 3.8 or higher
- Operating System: Windows 10+, macOS 10.14+, or Linux

### Dependencies

```bash
pip install -r requirements.txt
```

Or install manually:

```bash
# Required
pip install pyserial esptool

# Optional (for GUI)
pip install PyQt6
```

## Installation

### Option 1: Clone the repository

```bash
git clone https://github.com/brisma/gbsc-pro.git
cd gbsc-pro/gbsc-pro-flasher
pip install -r requirements.txt
```

### Option 2: Download standalone

1. Download `gbsc_flasher.py`, `requirements.txt`, and `gbsc-pro-logo.png`
2. Install dependencies: `pip install -r requirements.txt`

## Usage

### GUI Mode (Default)

Simply run without arguments to launch the graphical interface:

```bash
python gbsc_flasher.py
```

The GUI will:
1. Automatically detect connected GBSC devices
2. Allow you to select a firmware file (or drag & drop)
3. Flash with a single click

### CLI Mode

#### Auto-detect device and flash

```bash
# Just provide firmware - device is auto-detected
python gbsc_flasher.py firmware.bin

# Or specify port explicitly
python gbsc_flasher.py /dev/cu.usbmodem001 firmware.bin      # macOS
python gbsc_flasher.py /dev/ttyUSB0 firmware.bin            # Linux
python gbsc_flasher.py COM3 firmware.bin                     # Windows
```

#### Force device type

```bash
# Force ADV Controller mode
python gbsc_flasher.py --adv firmware.bin

# Force ESP8266 mode
python gbsc_flasher.py --esp firmware.bin
```

#### List detected devices

```bash
python gbsc_flasher.py --list
```

### Command Line Options

| Option | Description |
|--------|-------------|
| `--gui` | Launch GUI mode (default if no firmware specified) |
| `--adv` | Force ADV Controller (HC32F460) mode |
| `--esp` | Force ESP8266 (GBS-Control) mode |
| `--list` | List all detected devices and exit |
| `--version` | Show version and exit |
| `--help` | Show help message |

## Flashing Instructions

### ADV Controller (HC32F460)

1. **Enter bootloader mode:**
   - Disconnect the USB cable
   - Hold down the button on the device
   - Connect the USB cable while holding the button
   - Release the button after 2-3 seconds

2. **Verify detection:**
   - The device should appear as "ADV Controller" in the GUI
   - Or run `python gbsc_flasher.py --list` to verify

3. **Flash:**
   - Select the `.bin` firmware file
   - Click "Flash Firmware"

### GBS-Control (ESP8266)

1. **Connect the device:**
   - Simply connect the ESP8266 via USB
   - No special mode required (auto-reset is handled)

2. **Verify detection:**
   - The device should appear as "GBS-Control (ESP8266)" in the GUI
   - Or run `python gbsc_flasher.py --list` to verify

3. **Flash:**
   - Select the `.bin` firmware file
   - Click "Flash Firmware"

## Platform-Specific Notes

### Windows

- Install the CH340 driver if your ESP8266 uses this chip
- Devices appear as `COMx` (e.g., `COM3`, `COM4`)
- May require running as Administrator for some USB devices

### macOS

- ADV Controller appears as `/dev/cu.usbmodemXXXX`
- ESP8266 (CH340) appears as `/dev/cu.wchusbserialXXXX`
- No additional drivers needed for most devices

### Linux

- ADV Controller appears as `/dev/ttyACMx`
- ESP8266 appears as `/dev/ttyUSBx`
- You may need to add your user to the `dialout` group:
  ```bash
  sudo usermod -a -G dialout $USER
  ```
  Then log out and back in.

## Troubleshooting

### Device not detected

1. **Check USB connection** - Try a different cable or port
2. **Verify drivers** - Install CH340/CP210x drivers if needed
3. **Check permissions** (Linux) - Add user to `dialout` group
4. **Bootloader mode** (ADV only) - Ensure you're holding the button during connection

### Flash fails with NAK errors (ADV Controller)

- Restart the device in bootloader mode and try again
- Verify the firmware file is correct for ADV Controller
- Check that no other application is using the serial port

### esptool connection timeout (ESP8266)

- Try a lower baud rate (modify `ESP_BAUD` in the script)
- Hold the BOOT button on ESP8266 during connection
- Try the `--before default_reset` option manually

### GUI doesn't start

- Ensure PyQt6 is installed: `pip install PyQt6`
- On Linux, you may need: `sudo apt install python3-pyqt6`
- Use CLI mode as fallback: `python gbsc_flasher.py firmware.bin`

## Building Firmware

### ADV Controller

```bash
cd adv-controller
make
# Output: build/release/adv-controller.bin
```

### GBS-Control (ESP8266)

```bash
cd gbs-control
pio run
# Output: .pio/build/esp8266/firmware.bin
```

## License

This tool is part of the GBSC Pro project.

## Links

- [GBSC Pro Repository](https://github.com/brisma/gbsc-pro)
- [Original gbs-control](https://github.com/ramapcsx2/gbs-control)
- [RetroScaler](https://retroscaler.com)
