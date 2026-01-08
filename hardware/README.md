# Hardware Resources

Hardware design files and tools for GBSC-Pro.

Part of the [GBSC-Pro custom firmware](../README.md) by Brisma.

## Contents

| Directory | Description |
|-----------|-------------|
| [bom/](bom/) | Bill of Materials (Excel + interactive HTML) |
| [gerber/](gerber/) | PCB manufacturing files |
| [datasheets/](datasheets/) | IC datasheets (ADV7280, ADV7391, HC32, TV5725, STV9425) |
| [adv-cli-simulator/](adv-cli-simulator/) | HC32 CLI simulator for Raspberry Pi Pico |

## Schematic

The main schematic is available as [GBSC-AV-IR-v1.1-20240923.pdf](GBSC-AV-IR-v1.1-20240923.pdf).

## Building Your Own

1. Download Gerber files from [gerber/](gerber/)
2. Upload to your preferred PCB manufacturer (JLCPCB, PCBWay, etc.)
3. Order components from [bom/](bom/)
4. Flash firmware following the instructions in the main README
