# ADV CLI Simulator

Simulates the HC32 ADV CLI interface on a Raspberry Pi Pico, allowing testing of [ADV Manager](../../adv-manager/) without physical hardware.

## Requirements

- Raspberry Pi Pico (or Pico W)
- MicroPython firmware installed

## Installation

1. Install MicroPython on your Pico: https://micropython.org/download/RPI_PICO/
2. Copy `main.py` to the Pico (it will auto-run on boot)

## Usage

Connect via USB serial at 115200 baud. The simulator responds to the same commands as the real HC32:

| Command | Description |
|---------|-------------|
| `r AA RR` | Read register from device |
| `w AA RR VV` | Write value to register |
| `d AA RR QQ` | Dump QQ registers starting from RR |
| `rm AA MM RR` | Read register from map (ADV7280) |
| `wm AA MM RR VV` | Write value to map register |
| `dm AA MM RR QQ` | Dump from map |

### Parameters

- **AA**: I2C device address (`42`=ADV7280, `56`=ADV7391, `84`=VPP)
- **MM**: Map select (`00`=Main, `20`=Int/VDP, `40`=Map2)
- **RR**: Register address
- **VV**: Value to write
- **QQ**: Count for dump

### Example

```
> r 56 00
9C
> rm 42 00 10
12
> w 56 00 9C
OK
```

## Virtual Devices

The simulator emulates three I2C devices with realistic default values from a real GBSC-Pro dump:

| Device | Address | Maps |
|--------|---------|------|
| ADV7280 | 0x42 | User Sub Map (0x00), Int/VDP (0x20), Map2 (0x40) |
| ADV7391 | 0x56 | Main (0x00) |
| VPP | 0x84 | VPP (0x00) |

## Response Format

- **Read**: Returns 2-digit hex value (e.g., `9C`)
- **Write**: Returns `OK` or `ERR`
- **Dump**: Returns space-separated hex values with newline every 16 bytes
