# GBS-Control (ESP8266 Firmware)

ESP8266 firmware for the TV5725 video scaler with OSD, OLED menu, and web interface.

Part of the [GBSC-Pro custom firmware](../README.md) by Brisma.

Based on [gbs-control](https://github.com/ramapcsx2/gbs-control) by ramapcsx2.

## Features

- TV5725 scaler control
- TV OSD via STV9426
- OLED menu with IR remote
- Web interface (WiFi)
- 36 profile slots (A-Z, 0-9)
- ADV7280/ADV7391 control via UART

## Building

```bash
# Install PlatformIO CLI or VS Code extension
# https://platformio.org/install

# Build
pio run

# Upload
pio run --target upload
```

## Project Structure

```
gbs-control/
├── gbs-control.ino     # Main firmware
├── options.h           # Base settings
├── slot.h              # Profile slot structure
├── tv5725.h            # Scaler control
├── pro/                # Pro-specific code
│   ├── drivers/        # Hardware drivers
│   ├── osd/            # TV OSD system
│   └── menu/           # OLED menu system
└── public/             # Web interface
```

## Documentation

- [Web Interface](public/README.md)
- [Original gbs-control docs](https://ramapcsx2.github.io/gbs-control/)
