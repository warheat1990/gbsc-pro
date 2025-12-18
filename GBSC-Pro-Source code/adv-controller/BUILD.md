# Building ADV Controller Firmware

This guide explains how to build the ADV Controller firmware (HC32F460 + ADV7280/ADV7391) using the ARM GCC toolchain on macOS, Linux, and Windows.

## Prerequisites

### macOS

```bash
# Install via Homebrew
brew install --cask gcc-arm-embedded
```

### Linux (Ubuntu/Debian)

```bash
# Install via apt
sudo apt update
sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi
```

### Linux (Arch)

```bash
sudo pacman -S arm-none-eabi-gcc arm-none-eabi-newlib
```

### Linux (Fedora)

```bash
sudo dnf install arm-none-eabi-gcc-cs arm-none-eabi-newlib
```

### Windows

1. Download ARM GNU Toolchain from: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
2. Run the installer and select "Add path to environment variable"
3. Install Make via one of these options:
   - **Git Bash** (included with Git for Windows): https://git-scm.com/download/win
   - **MSYS2**: https://www.msys2.org/ then run `pacman -S make`
   - **Chocolatey**: `choco install make`

### Verify Installation

```bash
arm-none-eabi-gcc --version
make --version
```

## Building

### Navigate to the project directory

```bash
cd "GBSC-Pro-Source code/adv-controller"
```

### Build Commands

| Command | Description |
|---------|-------------|
| `make` | Build release version (default) |
| `make release` | Build release version explicitly |
| `make debug` | Build debug version (with symbols, no optimization) |
| `make clean` | Remove all build files |
| `make size` | Show firmware size |
| `make disasm` | Generate disassembly listing |
| `make help` | Show available targets |

## Output Files

After a successful build, output files are in:

- **Debug**: `build/debug/`
- **Release**: `build/release/`

| File | Description |
|------|-------------|
| `adv-controller.elf` | ELF executable (for debugging) |
| `adv-controller.bin` | Binary firmware (for flashing) |
| `adv-controller.hex` | Intel HEX format |
| `adv-controller.map` | Linker map file |

## Flashing the Firmware

### Using OpenOCD

```bash
openocd -f interface/jlink.cfg -f target/hc32f460.cfg \
    -c "program build/release/adv-controller.elf verify reset exit"
```

### Using J-Link Commander

```bash
JLinkExe -device HC32F460JETA -if SWD -speed 4000 -autoconnect 1
```

Then in J-Link Commander:
```
loadbin build/release/adv-controller.bin,0x00000000
r
go
```

## Troubleshooting

### "arm-none-eabi-gcc: command not found"

Ensure the toolchain is in your PATH:

**macOS (Apple Silicon)**:
```bash
export PATH="/opt/homebrew/bin:$PATH"
```

**macOS (Intel)**:
```bash
export PATH="/usr/local/bin:$PATH"
```

**Linux**:
```bash
export PATH="/usr/bin:$PATH"
```

**Windows**: Re-run the installer and ensure "Add to PATH" is selected.

### "make: command not found" (Windows)

Use Git Bash, MSYS2, or install Make via Chocolatey.

### Build errors

1. Ensure you're in the correct directory (`adv-controller/`)
2. Run `make clean` and try again
3. Check that all source files exist

## Project Structure

```
adv-controller/
├── src/           # Application source code
├── lib/           # Libraries and SDK
│   ├── cmsis/     # ARM CMSIS headers
│   ├── hc32_ll_driver/  # HC32 low-level drivers
│   └── bsp/       # Board support package
├── build/         # Build output (generated)
├── Makefile
└── BUILD.md
```
