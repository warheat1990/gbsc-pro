# =============================================================================
# ADV Controller Firmware - ARM GCC Makefile (macOS/Linux/Windows)
# HC32F460 + ADV7280/ADV7391 Video Processor Controller
# =============================================================================

# Project name
PROJECT = adv-controller

# Build configuration: debug or release
BUILD ?= release

# Toolchain prefix
CROSS_COMPILE ?= arm-none-eabi-

# Tools
CC      = $(CROSS_COMPILE)gcc
AS      = $(CROSS_COMPILE)gcc
LD      = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE    = $(CROSS_COMPILE)size

# =============================================================================
# Directories
# =============================================================================
BUILD_DIR = build/$(BUILD)
SRC_DIR   = src
LIB_DIR   = lib

# =============================================================================
# Source files
# =============================================================================

C_SOURCES = \
    $(SRC_DIR)/main.c \
    $(SRC_DIR)/i2c.c \
    $(SRC_DIR)/uart_dma.c \
    $(SRC_DIR)/videoprocess.c \
    $(SRC_DIR)/debug_uart.c \
    $(SRC_DIR)/adv_cli.c \
    $(SRC_DIR)/flash.c \
    $(SRC_DIR)/fault_handler.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_clk.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_efm.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_gpio.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_interrupts.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_pwc.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_sram.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_usart.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_utility.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_crc.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_fcg.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_tmr0.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_i2c.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_fcm.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_icg.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_aos.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_dma.c \
    $(LIB_DIR)/hc32_ll_driver/src/hc32_ll_wdt.c \
    $(LIB_DIR)/bsp/ev_hc32f460_lqfp100_v2/ev_hc32f460_lqfp100_v2.c \
    $(LIB_DIR)/cmsis/Device/HDSC/hc32f4xx/Source/system_hc32f460.c

ASM_SOURCES = \
    $(LIB_DIR)/cmsis/Device/HDSC/hc32f4xx/Source/GCC/startup_hc32f460.S

# =============================================================================
# Include paths
# =============================================================================
INCLUDES = \
    -I$(SRC_DIR) \
    -I$(LIB_DIR)/cmsis/Device/HDSC/hc32f4xx/Include \
    -I$(LIB_DIR)/cmsis/Include \
    -I$(LIB_DIR)/hc32_ll_driver/inc \
    -I$(LIB_DIR)/bsp/ev_hc32f460_lqfp100_v2

# =============================================================================
# Compiler/Linker flags
# =============================================================================

CPU_FLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft

CFLAGS = $(CPU_FLAGS)
CFLAGS += -Wall
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -std=c99
CFLAGS += $(INCLUDES)
CFLAGS += -DHC32F460
CFLAGS += -DUSE_DDL_DRIVER

ifeq ($(BUILD),debug)
    CFLAGS += -g3 -gdwarf-2 -O0
    CFLAGS += -D__DEBUG
else
    CFLAGS += -Os
endif

ASFLAGS = $(CPU_FLAGS)
ASFLAGS += -x assembler-with-cpp
ASFLAGS += $(INCLUDES)

LDSCRIPT = $(LIB_DIR)/cmsis/Device/HDSC/hc32f4xx/Source/GCC/linker/HC32F460xE.ld

LDFLAGS = $(CPU_FLAGS)
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map,--cref
LDFLAGS += --specs=nano.specs
LDFLAGS += --specs=nosys.specs
LDFLAGS += -lc -lm -lnosys

# =============================================================================
# Object files - flatten to build directory
# =============================================================================
C_OBJECTS   = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
ASM_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
OBJECTS     = $(C_OBJECTS) $(ASM_OBJECTS)

# VPATH tells make where to find source files
VPATH = $(SRC_DIR):$(LIB_DIR)/hc32_ll_driver/src:$(LIB_DIR)/bsp/ev_hc32f460_lqfp100_v2:$(LIB_DIR)/cmsis/Device/HDSC/hc32f4xx/Source:$(LIB_DIR)/cmsis/Device/HDSC/hc32f4xx/Source/GCC

# =============================================================================
# Build rules
# =============================================================================

all: $(BUILD_DIR) $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).bin $(BUILD_DIR)/$(PROJECT).hex
	@echo ""
	@echo "Build complete: $(BUILD)"
	@$(SIZE) $(BUILD_DIR)/$(PROJECT).elf

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/%.o: %.c
	@echo "CC  $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	@echo "AS  $<"
	@$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/$(PROJECT).elf: $(OBJECTS)
	@echo "LD  $@"
	@$(LD) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf
	@echo "BIN $@"
	@$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/$(PROJECT).hex: $(BUILD_DIR)/$(PROJECT).elf
	@echo "HEX $@"
	@$(OBJCOPY) -O ihex $< $@

clean:
	@echo "Cleaning..."
	@rm -rf build

debug:
	@$(MAKE) BUILD=debug

release:
	@$(MAKE) BUILD=release

size: $(BUILD_DIR)/$(PROJECT).elf
	@$(SIZE) --format=berkeley $<

disasm: $(BUILD_DIR)/$(PROJECT).elf
	@$(OBJDUMP) -d -S $< > $(BUILD_DIR)/$(PROJECT).dis

help:
	@echo "ADV Controller Firmware Build System"
	@echo ""
	@echo "Usage: make [target] [BUILD=debug|release]"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build firmware (default)"
	@echo "  debug    - Build debug version"
	@echo "  release  - Build release version"
	@echo "  clean    - Remove build files"
	@echo "  size     - Print firmware size"
	@echo "  disasm   - Generate disassembly"
	@echo "  help     - Print this help"

.PHONY: all clean debug release size disasm help
