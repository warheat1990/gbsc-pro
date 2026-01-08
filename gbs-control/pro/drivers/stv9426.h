/*
   STV9426 OSD chip library for ESP8266 (Arduino framework)

   Original by: Karabanov Aleksandr (2024-02-16)
   https://www.youtube.com/channel/UCkNk5gQMIu8k9xW-uENsBzw

*/

#ifndef OSD_STV9426_H_
#define OSD_STV9426_H_

#define ADDR_STV 0x5D

// ====================================================================================
// STV9426 Register Configuration
// ====================================================================================

// Scanline configuration N1 (vertical position row 1)
#define A1_lines 0x43
#define A2_lines 0x00
#define A3_lines 0xD0

// Scanline configuration N2 (vertical position row 2)
#define B1_lines 0x45
#define B2_lines 0x00
#define B3_lines 0xFE

// Scanline configuration N3 (vertical position row 3)
#define C1_lines 0x47
#define C2_lines 0x00
#define C3_lines 0xF3

// Scanline configuration N0 (base position)
#define D1_lines 0x40
#define D2_lines 0x00
#define D3_lines 0x3A

// LINE DURATION (horizontal movement)
#define A1_parameters 0xF0
#define A2_parameters 0x3F
#define A3_parameters 0x3F

// HORIZONTAL DELAY (horizontal offset)
#define B1_parameters 0xF1
#define B2_parameters 0x3F
#define B3_parameters 0x26

// CHARACTER HEIGHT
#define C1_parameters 0xF2
#define C2_parameters 0x3F
#define C3_parameters 0x15

// FREQUENCY MULTIPLIER (helps stabilize picture)
// #define Z1_parameters 0xF7
// #define Z2_parameters 0x3F
// #define Z3_parameters 0x09

// INITIAL PIXEL PERIOD
#define D1_parameters 0xF6
#define D2_parameters 0x3F
#define D3_parameters 0x02

// LOCKING CONDITION TIME CONSTANT (removes jitter)
#define E1_parameters 0xF4
#define E2_parameters 0x3F
#define E3_parameters 0x02

// CAPTURE PROCESS TIME CONSTANT
#define G1_parameters 0xF5
#define G2_parameters 0x3F
#define G3_parameters 0x03

// DISPLAY CONTROL
#define H1_parameters 0xF3
#define H2_parameters 0x3F
#define H3_parameters 0x81

// ====================================================================================
// OSD Font Character Codes (ASCII mapping for STV9426)
// ====================================================================================

// Special icon characters
static const uint8_t horizontal_scale_part1_icon = 0x03; // Horizontal scale part 1 icon
static const uint8_t horizontal_scale_part2_icon = 0x13; // Horizontal scale part 2 icon
static const uint8_t horizontal_move_part1_icon = 0x04; // Horizontal move part 1 icon
static const uint8_t horizontal_move_part2_icon = 0x14; // Horizontal move part 2 icon
static const uint8_t arrow_left_icon = 0x05; // Arrow left icon
static const uint8_t arrow_right_icon = 0x15; // Arrow right icon
static const uint8_t arrow_up_icon = 0x06; // Arrow up icon
static const uint8_t arrow_down_icon = 0x16; // Arrow down icon
static const uint8_t vertical_move_part1_icon = 0x07; // Vertical move part 1 icon
static const uint8_t vertical_move_part2_icon = 0x17; // Vertical move part 2 icon
static const uint8_t vertical_scale_part1_icon = 0x08; // Vertical scale part 1 icon
static const uint8_t vertical_scale_part2_icon = 0x18; // Vertical scale part 2 icon
static const uint8_t borders_part1_icon = 0x09; // Borders part 1 icon
static const uint8_t borders_part2_icon = 0x19; // Borders part 2 icon
static const uint8_t contrast_part1_icon = 0x0A; // Contrast part 1 icon
static const uint8_t contrast_part2_icon = 0x1A; // Contrast part 2 icon
static const uint8_t brightness_part1_icon = 0x0B; // Brightness part 1 icon
static const uint8_t brightness_part2_icon = 0x1B; // Brightness part 2 icon
static const uint8_t enable_icon = 0x0D; // Enable icon
static const uint8_t disable_icon = 0x1D; // Disable icon

// ====================================================================================
// OSD Colors - Attribute Byte Format (LSB of character code pair)
// ====================================================================================
// Datasheet format: BK3 | BK2 | BK1 | BK0 | FL | RF | GF | BF
//
// Bit 7:   BK3 - Shadow enable (1=shadow extends 1 pixel right/down)
// Bit 6-4: BK[2:0] - Background RGB
// Bit 3:   FL - Flash enable
// Bit 2-0: RF/GF/BF - Foreground RGB
//
// RGB color indices (0-7):
//   000 = Black, 001 = Blue, 010 = Green, 011 = Cyan
//   100 = Red,   101 = Magenta, 110 = Yellow, 111 = White

// ====================================================================================
// OSD Theme System - Color Building Blocks
// ====================================================================================

// Foreground color indices (bits 2-0)
#define OSD_FG_BLACK   0
#define OSD_FG_BLUE    1
#define OSD_FG_GREEN   2
#define OSD_FG_CYAN    3
#define OSD_FG_RED     4
#define OSD_FG_MAGENTA 5
#define OSD_FG_YELLOW  6
#define OSD_FG_WHITE   7

// Background color indices (bits 6-4, pre-shifted)
#define OSD_BG_BLACK   0x00
#define OSD_BG_BLUE    0x10
#define OSD_BG_GREEN   0x20
#define OSD_BG_CYAN    0x30
#define OSD_BG_RED     0x40
#define OSD_BG_MAGENTA 0x50
#define OSD_BG_YELLOW  0x60
#define OSD_BG_WHITE   0x70

// Modifier flags
#define OSD_SHADOW     0x80  // Bit 7: shadow effect
#define OSD_FLASH      0x08  // Bit 3: flashing effect

// Macro to combine foreground + background into color byte
#define OSD_COLOR(fg, bg) ((bg) | (fg))

// ====================================================================================
// OSD Theme - Semantic Color Variables
// ====================================================================================
// These variables define the color scheme for all OSD elements.
// They can be modified at runtime to change the theme.

// Text colors (defined in osd-core.cpp)
extern uint8_t OSD_TEXT_NORMAL;      // Normal menu text
extern uint8_t OSD_TEXT_SELECTED;    // Selected/highlighted item
extern uint8_t OSD_TEXT_DISABLED;    // Unavailable option

// Special value: use menu line color (yellow if selected, white otherwise)
#define OSD_COLOR_AUTO 0xFF

// Navigation elements (defined in osd-core.cpp)
extern uint8_t OSD_ICON_PAGE;        // Page numbers, nav arrows
extern uint8_t OSD_CURSOR_ACTIVE;    // Active row cursor
extern uint8_t OSD_CURSOR_INACTIVE;  // Inactive row cursor

// Background (defined in osd-core.cpp)
extern uint8_t OSD_BACKGROUND;       // Menu background fill

// ====================================================================================
// OSD Theme Presets
// ====================================================================================

// Theme IDs
#define OSD_THEME_CLASSIC    0   // Blue background, white/yellow text (default)
#define OSD_THEME_DARK       1   // Black background, white/cyan text
#define OSD_THEME_LIGHT      2   // White background, black/blue text
#define OSD_THEME_RETRO      3   // Green background, black/yellow text (CRT style)
#define OSD_THEME_COUNT      4   // Total number of themes

// Theme names for menu display
static const char* OSD_THEME_NAMES[] = {
    "Classic",
    "Dark",
    "Light",
    "Retro"
};

// Current theme ID (defined in osd-core.cpp)
extern uint8_t OSD_currentTheme;

// Set OSD theme - changes all color variables at once
// themeId: OSD_THEME_CLASSIC, OSD_THEME_DARK, OSD_THEME_LIGHT, or OSD_THEME_RETRO
inline void OSD_setTheme(uint8_t themeId)
{
    OSD_currentTheme = themeId;

    switch (themeId) {
        case OSD_THEME_DARK:
            // Black background, white/yellow text
            OSD_TEXT_NORMAL      = OSD_COLOR(OSD_FG_WHITE, OSD_BG_BLACK);
            OSD_TEXT_SELECTED    = OSD_COLOR(OSD_FG_YELLOW, OSD_BG_BLACK);
            OSD_TEXT_DISABLED    = OSD_COLOR(OSD_FG_RED, OSD_BG_BLACK);
            OSD_ICON_PAGE        = OSD_COLOR(OSD_FG_GREEN, OSD_BG_BLACK);
            OSD_CURSOR_ACTIVE    = OSD_COLOR(OSD_FG_BLACK, OSD_BG_YELLOW);
            OSD_CURSOR_INACTIVE  = OSD_COLOR(OSD_FG_BLACK, OSD_BG_BLACK);
            OSD_BACKGROUND       = OSD_COLOR(OSD_FG_BLACK, OSD_BG_BLACK);
            break;

        case OSD_THEME_LIGHT:
            // White background, black/blue text
            OSD_TEXT_NORMAL      = OSD_COLOR(OSD_FG_BLACK, OSD_BG_WHITE);
            OSD_TEXT_SELECTED    = OSD_COLOR(OSD_FG_BLUE, OSD_BG_WHITE);
            OSD_TEXT_DISABLED    = OSD_COLOR(OSD_FG_RED, OSD_BG_WHITE);
            OSD_ICON_PAGE        = OSD_COLOR(OSD_FG_GREEN, OSD_BG_WHITE);
            OSD_CURSOR_ACTIVE    = OSD_COLOR(OSD_FG_WHITE, OSD_BG_BLUE);
            OSD_CURSOR_INACTIVE  = OSD_COLOR(OSD_FG_WHITE, OSD_BG_WHITE);
            OSD_BACKGROUND       = OSD_COLOR(OSD_FG_WHITE, OSD_BG_WHITE);
            break;

        case OSD_THEME_RETRO:
            // Green background, black/yellow text (CRT terminal style)
            OSD_TEXT_NORMAL      = OSD_COLOR(OSD_FG_BLACK, OSD_BG_GREEN);
            OSD_TEXT_SELECTED    = OSD_COLOR(OSD_FG_YELLOW, OSD_BG_GREEN);
            OSD_TEXT_DISABLED    = OSD_COLOR(OSD_FG_RED, OSD_BG_GREEN);
            OSD_ICON_PAGE        = OSD_COLOR(OSD_FG_WHITE, OSD_BG_GREEN);
            OSD_CURSOR_ACTIVE    = OSD_COLOR(OSD_FG_GREEN, OSD_BG_YELLOW);
            OSD_CURSOR_INACTIVE  = OSD_COLOR(OSD_FG_GREEN, OSD_BG_GREEN);
            OSD_BACKGROUND       = OSD_COLOR(OSD_FG_GREEN, OSD_BG_GREEN);
            break;

        case OSD_THEME_CLASSIC:
        default:
            // Blue background, white/yellow text (original)
            OSD_TEXT_NORMAL      = OSD_COLOR(OSD_FG_WHITE, OSD_BG_BLUE);
            OSD_TEXT_SELECTED    = OSD_COLOR(OSD_FG_YELLOW, OSD_BG_BLUE);
            OSD_TEXT_DISABLED    = OSD_COLOR(OSD_FG_RED, OSD_BG_BLUE);
            OSD_ICON_PAGE        = OSD_COLOR(OSD_FG_GREEN, OSD_BG_BLUE);
            OSD_CURSOR_ACTIVE    = OSD_COLOR(OSD_FG_BLACK, OSD_BG_YELLOW);
            OSD_CURSOR_INACTIVE  = OSD_COLOR(OSD_FG_BLUE, OSD_BG_BLUE);
            OSD_BACKGROUND       = OSD_COLOR(OSD_FG_BLUE, OSD_BG_BLUE);
            break;
    }
}

// Get current theme ID
inline uint8_t OSD_getTheme()
{
    return OSD_currentTheme;
}

// Get theme name for display
inline const char* OSD_getThemeName(uint8_t themeId)
{
    if (themeId >= OSD_THEME_COUNT) return "Unknown";
    return OSD_THEME_NAMES[themeId];
}

// Cycle to next theme (useful for quick switching)
inline void OSD_nextTheme()
{
    OSD_setTheme((OSD_currentTheme + 1) % OSD_THEME_COUNT);
}

// ====================================================================================
// OSD Row Identifiers
// ====================================================================================
// ROW_1 = 0x00 (top), ROW_2 = 0x02 (middle), ROW_3 = 0x03 (bottom)

static const uint8_t ROW_1 = 0x00;
static const uint8_t ROW_2 = 0x02;
static const uint8_t ROW_3 = 0x03;

// Maximum menu rows supported (can expand 3→6 in future)
#ifndef OSD_MAX_MENU_ROWS
#define OSD_MAX_MENU_ROWS 3
#endif

// Menu line colors - current color for each row (part of theme system)
extern uint8_t menuLineColors[OSD_MAX_MENU_ROWS];

// Helper to resolve OSD_COLOR_AUTO to actual row color
inline uint8_t OSD_resolveColor(uint8_t row, uint8_t color) {
    if (color == OSD_COLOR_AUTO && row >= 1 && row <= OSD_MAX_MENU_ROWS) {
        return menuLineColors[row - 1];
    }
    return color;
}

// ====================================================================================
// Low-Level I2C Communication
// ====================================================================================

// Send 3-byte command to STV9426 via I2C
inline void OSD_sendCommand(char reg, char bank, char value)
{
    Wire.beginTransmission(ADDR_STV);
    Wire.write(reg);       // Register address
    Wire.write(bank);      // Memory bank (0x00=row1, 0x02=row2, 0x03=row3)
    Wire.write(value);     // Value to write
    Wire.endTransmission();
}

// ====================================================================================
// OSD Initialization
// ====================================================================================

// Initialize STV9426 OSD chip with default configuration
inline void OSD_init()
{
    // Clear spacing registers
    for (byte reg = 0x40; reg < 0x48; ++reg) {
        OSD_sendCommand(reg, 0x00, 0x00);
    }
    // Configure scanline positions
    OSD_sendCommand(A1_lines, A2_lines, A3_lines);                // Row 1 vertical position
    OSD_sendCommand(B1_lines, B2_lines, B3_lines);                // Row 2 vertical position
    OSD_sendCommand(C1_lines, C2_lines, C3_lines);                // Row 3 vertical position
    OSD_sendCommand(D1_lines, D2_lines, D3_lines);                // Base vertical position
    // Configure display parameters
    OSD_sendCommand(A1_parameters, A2_parameters, A3_parameters); // Line duration
    OSD_sendCommand(B1_parameters, B2_parameters, B3_parameters); // Horizontal delay
    OSD_sendCommand(C1_parameters, C2_parameters, C3_parameters); // Character height
    OSD_sendCommand(D1_parameters, D2_parameters, D3_parameters); // Initial pixel period
    OSD_sendCommand(E1_parameters, E2_parameters, E3_parameters); // Locking time constant
    OSD_sendCommand(G1_parameters, G2_parameters, G3_parameters); // Capture time constant
    OSD_sendCommand(H1_parameters, H2_parameters, H3_parameters); // Display control
}

// ====================================================================================
// Row Clear Functions (FBLK clear)
// ====================================================================================

// Clear symbols on a row (set all character codes to 0x00)
// row: ROW_1 (0x00), ROW_2 (0x02), or ROW_3 (0x03)
inline void OSD_clearRowSymbols(uint8_t row)
{
    for (uint16_t addr = 0x00; addr < 0x100; ++addr) {
        OSD_sendCommand(addr, row, 0x00);
    }
}

// Clear colors on a row (set all attributes to 0xC0 = black on black)
// row: ROW_1 (0x00), ROW_2 (0x02), or ROW_3 (0x03)
inline void OSD_clearRowColors(uint8_t row)
{
    for (uint16_t addr = 0x00; addr < 0x100; ++addr) {
        OSD_sendCommand(addr, row, 0xC0);
    }
}

// ====================================================================================
// Character Write Functions (write char + color at position)
// ====================================================================================

// Map row number (1-3) to hardware bank (0x00, 0x02, 0x03)
inline uint8_t OSD_rowToBank(uint8_t row) {
    if (row == 1) return 0x00;
    if (row == 2) return 0x02;
    return 0x03;
}

// Map hardware bank (0x00, 0x02, 0x03) to row number (1-3)
inline uint8_t OSD_bankToRow(uint8_t bank) {
    if (bank == 0x00) return 1;
    if (bank == 0x02) return 2;
    return 3;
}

// Static variable for string/char continuation position
static uint8_t _osd_string_continue_pos = 0;

// Write character with color on specified row
// row: 1, 2, or 3
// pos: logical position (0-27), or 0xFF to continue from last position
// charCode: ASCII character code (n0-n9, A-Z, a-z, etc.)
// color: OSD_COLOR_AUTO (default) uses menu line color, or specify explicit color
inline void OSD_writeCharAtRow(uint8_t row, uint8_t pos, uint8_t charCode, uint8_t color = OSD_COLOR_AUTO)
{
    // Handle 0xFF continuation
    if (pos == 0xFF)
        pos = _osd_string_continue_pos;
    _osd_string_continue_pos = pos + 1;

    // Resolve auto color to menu line color
    color = OSD_resolveColor(row, color);

    uint8_t bank = OSD_rowToBank(row);
    uint8_t hwPos = (pos * 2) + 1;  // Convert logical to hardware pos

    // Character mapping for OSD font (sorted by output byte value)
    switch (charCode) {
        case ' ':
            // Space: keep background from color, set foreground same as background
            // This makes space "invisible" but with correct background color
            color = (color & 0xF0) | ((color >> 4) & 0x07);
            break;
        case '\'': charCode = 0x27; break;
        case '.':  charCode = 0x2E; break;
        case '/':  charCode = 0x2F; break;
        case ':':  charCode = 0x3A; break;
        case '+':  charCode = 0x3C; break;
        case '=':  charCode = 0x3D; break;
        case '-':  charCode = 0x3E; break;
    }
    OSD_sendCommand(hwPos, bank, charCode);
    OSD_sendCommand(hwPos - 1, bank, color);
}

// ====================================================================================
// String Write Functions
// ====================================================================================

// Write null-terminated string on specific row at logical position with color
// row: 1, 2, or 3
// startPos: logical starting position (0-27), or 0xFF to continue from last position
// str: null-terminated ASCII string
// color: OSD_COLOR_AUTO (default) uses menu line color, or specify explicit color
// Note: spaces are written with OSD_BACKGROUND to clear the position
inline void OSD_writeStringAtRow(uint8_t row, uint8_t startPos, const char* str,
                                  uint8_t color = OSD_COLOR_AUTO)
{
    if (str == NULL) return;

    // First char uses startPos (handles 0xFF), rest use 0xFF to continue
    bool first = true;
    while (*str != '\0') {
        OSD_writeCharAtRow(row, first ? startPos : 0xFF, *str, color);
        first = false;
        str++;
    }
}

// ====================================================================================
// Clear All OSD Rows
// ====================================================================================

inline void OSD_clearAll()
{
    OSD_clearRowSymbols(ROW_1);
    OSD_clearRowSymbols(ROW_2);
    OSD_clearRowSymbols(ROW_3);
    OSD_clearRowColors(ROW_1);
    OSD_clearRowColors(ROW_2);
    OSD_clearRowColors(ROW_3);
}

// ====================================================================================
// Row Content Clear (fills positions with 'o' character in background color)
// ====================================================================================

// Clear row content from startPos to endPos (1-28 range)
// row: ROW_1/ROW_2/ROW_3 (hardware bank)
// endPos: clear up to this position (exclusive)
// startPos: start from this position (1-28)
inline void OSD_clearRowContent(char row, char endPos, char startPos)
{
    // Position N (1-28) maps to hardware address (N-1)*2 + 1
    for (byte pos = startPos; pos < endPos; ++pos) {
        uint8_t osdPos = 0x01 + (pos - 1) * 2;
        OSD_sendCommand(osdPos, row, 'o');
        OSD_sendCommand(osdPos - 1, row, OSD_BACKGROUND);
    }
}

// ====================================================================================
// Background Fill
// ====================================================================================

// Fill row background with specified color up to endPos (exclusive)
// row: ROW_1/ROW_2/ROW_3
// endPos: logical position (0-28) - fills from 0 to endPos-1
// color: fill color
inline void OSD_fillRowBackground(char row, uint8_t endPos, char color)
{
    // Each logical position uses 2 bytes (char + color), so fill up to endPos*2 bytes
    uint8_t endAddr = endPos * 2;
    for (byte addr = 0x00; addr < endAddr; ++addr) {
        OSD_sendCommand(addr, row, color);
    }
}

// Fill all 3 rows with background color (all 28 positions)
inline void OSD_fillBackground()
{
    OSD_fillRowBackground(ROW_1, 28, OSD_BACKGROUND);
    OSD_fillRowBackground(ROW_2, 28, OSD_BACKGROUND);
    OSD_fillRowBackground(ROW_3, 28, OSD_BACKGROUND);
}

// ====================================================================================
// Display 2-Digit Decimal Number (0-99)
// ====================================================================================

// Display a byte value (0-99) as 2 decimal digits at specified positions
// row: 1, 2, or 3
// value: byte to display (0-99)
// pos1, pos2: logical positions (0-27) for units, tens
// color: OSD_COLOR_AUTO (default) uses menu line color, or specify explicit color
inline void OSD_displayNumber2DigitAtRow(uint8_t row, byte value,
                                          uint8_t pos1, uint8_t pos2,
                                          uint8_t color = OSD_COLOR_AUTO)
{
    OSD_writeCharAtRow(row, pos1, '0' + (value % 10), color);         // units
    OSD_writeCharAtRow(row, pos2, '0' + ((value / 10) % 10), color);  // tens
}

// ====================================================================================
// Display 3-Digit Decimal Number (0-255)
// ====================================================================================

// Display a byte value (0-255) as 3 decimal digits at specified positions
// row: 1, 2, or 3
// value: byte to display (0-255)
// pos1, pos2, pos3: logical positions (0-27) for units, tens, hundreds
// color: OSD_COLOR_AUTO (default) uses menu line color, or specify explicit color
inline void OSD_displayNumber3DigitAtRow(uint8_t row, byte value,
                                          uint8_t pos1, uint8_t pos2, uint8_t pos3,
                                          uint8_t color = OSD_COLOR_AUTO)
{
    OSD_writeCharAtRow(row, pos1, '0' + (value % 10), color);         // units
    OSD_writeCharAtRow(row, pos2, '0' + ((value / 10) % 10), color);  // tens
    OSD_writeCharAtRow(row, pos3, '0' + (value / 100), color);        // hundreds
}

// ====================================================================================
// Display 4-Digit Decimal Number (0-9999)
// ====================================================================================

// Display a 16-bit value (0-9999) as 4 decimal digits at specified positions
// row: 1, 2, or 3
// value: uint16_t to display (0-9999)
// pos1, pos2, pos3, pos4: logical positions (0-27) for units, tens, hundreds, thousands
// color: OSD_COLOR_AUTO (default) uses menu line color, or specify explicit color
inline void OSD_displayNumber4DigitAtRow(uint8_t row, uint16_t value,
                                          uint8_t pos1, uint8_t pos2, uint8_t pos3, uint8_t pos4,
                                          uint8_t color = OSD_COLOR_AUTO)
{
    OSD_writeCharAtRow(row, pos1, '0' + (value % 10), color);          // units
    OSD_writeCharAtRow(row, pos2, '0' + ((value / 10) % 10), color);   // tens
    OSD_writeCharAtRow(row, pos3, '0' + ((value / 100) % 10), color);  // hundreds
    OSD_writeCharAtRow(row, pos4, '0' + ((value / 1000) % 10), color); // thousands
}

// ====================================================================================
// PWM DAC Control (8-bit PWM outputs for analog voltage control)
// ====================================================================================
// STV9425 has 8 PWM outputs (PWM0-PWM7), STV9426 (DIP16) has none
// Output voltage = (value / 256) * VDD
// PWM frequency = FXTAL / 256

// PWM Register addresses (0x3FF8-0x3FFF)
#define OSD_REG_PWM0 0xF8
#define OSD_REG_PWM1 0xF9
#define OSD_REG_PWM2 0xFA
#define OSD_REG_PWM3 0xFB
#define OSD_REG_PWM4 0xFC
#define OSD_REG_PWM5 0xFD
#define OSD_REG_PWM6 0xFE
#define OSD_REG_PWM7 0xFF

// Set PWM output value (0-255)
// channel: 0-7 (PWM0-PWM7)
// value: 0-255 (duty cycle = value/256)
inline void OSD_setPWM(uint8_t channel, uint8_t value)
{
    if (channel > 7) return;
    OSD_sendCommand(OSD_REG_PWM0 + channel, 0x3F, value);
}

// Set all PWM outputs to same value
inline void OSD_setPWMAll(uint8_t value)
{
    for (uint8_t i = 0; i < 8; i++) {
        OSD_setPWM(i, value);
    }
}

// ====================================================================================
// I2C Read Functions
// ====================================================================================
// Read data from STV9426 registers, RAM, or ROM
// Address space: 0x0000-0x03FF = RAM, 0x2000-0x32FF = ROM, 0x3FF0-0x3FFF = Registers

// Read single byte from STV9426
// Returns the byte at the specified address
inline uint8_t OSD_readByte(uint16_t address)
{
    uint8_t addrLSB = address & 0xFF;
    uint8_t addrMSB = (address >> 8) & 0x3F;  // Only 14 bits used

    // First sequence: write address
    Wire.beginTransmission(ADDR_STV);
    Wire.write(addrLSB);
    Wire.write(addrMSB);
    Wire.endTransmission();

    // Second sequence: read data
    Wire.requestFrom(ADDR_STV, (uint8_t)1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0;
}

// Read multiple bytes from STV9426
// buffer: destination buffer
// address: starting address
// length: number of bytes to read
inline void OSD_readBytes(uint8_t* buffer, uint16_t address, uint8_t length)
{
    uint8_t addrLSB = address & 0xFF;
    uint8_t addrMSB = (address >> 8) & 0x3F;

    // First sequence: write address
    Wire.beginTransmission(ADDR_STV);
    Wire.write(addrLSB);
    Wire.write(addrMSB);
    Wire.endTransmission();

    // Second sequence: read data
    Wire.requestFrom(ADDR_STV, length);
    for (uint8_t i = 0; i < length && Wire.available(); i++) {
        buffer[i] = Wire.read();
    }
}

// Read register value (registers are at 0x3FF0-0x3FFF)
// reg: 0xF0-0xFF (only lower nibble used, e.g., 0xF0 → address 0x3FF0)
inline uint8_t OSD_readRegister(uint8_t reg)
{
    return OSD_readByte(0x3FF0 | (reg & 0x0F));
}

// ====================================================================================
// User Definable Characters (UDC)
// ====================================================================================
// Up to 26 user-definable characters (12x18 pixel matrix + 1 shadow slice)
// Each UDC uses 38 bytes in RAM (19 slices x 2 bytes per slice)
// Character numbers 0-25 in SET=1 mode
// RAM address = 38 * CHARACTER_NUMBER + SLICE_NUMBER

// UDC slice data format:
// Even address: PX7-PX0 (pixels 0-7, PX0 = rightmost)
// Odd address:  ----PX11-PX8 (pixels 8-11, PX11 = leftmost)

// Write a single slice of a UDC
// charNum: character number (0-25)
// sliceNum: slice number (0-18, slice 18 is for vertical shadow only)
// pixelsLow: pixels 0-7 (bit 0 = PX0 = rightmost pixel)
// pixelsHigh: pixels 8-11 in bits 0-3 (bit 0 = PX8)
inline void OSD_writeUDCSlice(uint8_t charNum, uint8_t sliceNum, uint8_t pixelsLow, uint8_t pixelsHigh)
{
    if (charNum > 25 || sliceNum > 18) return;
    uint16_t baseAddr = 38 * charNum + sliceNum * 2;
    // Write to RAM (address space 0x0000-0x03FF)
    Wire.beginTransmission(ADDR_STV);
    Wire.write(baseAddr & 0xFF);           // LSB address
    Wire.write((baseAddr >> 8) & 0x3F);    // MSB address
    Wire.write(pixelsLow);                 // Pixels 0-7
    Wire.write(pixelsHigh & 0x0F);         // Pixels 8-11 (only lower 4 bits)
    Wire.endTransmission();
}

// Write complete UDC from 38-byte array
// charNum: character number (0-25)
// data: 38 bytes (19 slices x 2 bytes, even=PX7-0, odd=PX11-8)
inline void OSD_writeUDC(uint8_t charNum, const uint8_t* data)
{
    if (charNum > 25) return;
    uint16_t baseAddr = 38 * charNum;

    Wire.beginTransmission(ADDR_STV);
    Wire.write(baseAddr & 0xFF);
    Wire.write((baseAddr >> 8) & 0x3F);
    for (uint8_t i = 0; i < 38; i++) {
        Wire.write(data[i]);
    }
    Wire.endTransmission();
}

// Read UDC data
// charNum: character number (0-25)
// buffer: 38-byte destination buffer
inline void OSD_readUDC(uint8_t charNum, uint8_t* buffer)
{
    if (charNum > 25) return;
    uint16_t baseAddr = 38 * charNum;
    OSD_readBytes(buffer, baseAddr, 38);
}

// ====================================================================================
// Display Control Register (0x3FF3)
// ====================================================================================
// Bit 7: OSD - Display on/off (0=off, R/G/B/FBLK=0)
// Bit 6: FBK - Fast blanking (1=FBLK always on during no display)
// Bit 5-4: FL[1:0] - Flashing mode
// Bit 2-0: P[8:6] - Page address bits

// Flashing modes
#define OSD_FLASH_OFF   0x00  // No flashing
#define OSD_FLASH_1_1   0x10  // 50% duty cycle
#define OSD_FLASH_1_3   0x20  // 25% duty cycle (1 on, 3 off)
#define OSD_FLASH_3_1   0x30  // 75% duty cycle (3 on, 1 off)

// Turn OSD display on
inline void OSD_displayOn()
{
    OSD_sendCommand(0xF3, 0x3F, 0x81);  // OSD=1, default config
}

// Turn OSD display off (R, G, B, FBLK = 0)
inline void OSD_displayOff()
{
    OSD_sendCommand(0xF3, 0x3F, 0x01);  // OSD=0
}

// Set flashing mode for characters with FL attribute
// mode: OSD_FLASH_OFF, OSD_FLASH_1_1, OSD_FLASH_1_3, or OSD_FLASH_3_1
inline void OSD_setFlashMode(uint8_t mode)
{
    uint8_t reg = OSD_readRegister(0xF3);
    reg = (reg & 0xCF) | (mode & 0x30);
    OSD_sendCommand(0xF3, 0x3F, reg);
}

// Set fast blanking mode
// enabled: true = FBLK=1 during no display, false = FBLK only during character display
inline void OSD_setFastBlanking(bool enabled)
{
    uint8_t reg = OSD_readRegister(0xF3);
    if (enabled) reg |= 0x40;
    else reg &= ~0x40;
    OSD_sendCommand(0xF3, 0x3F, reg);
}

// Select display page (0-7)
// Different pages can be pre-loaded in RAM and switched instantly
inline void OSD_selectPage(uint8_t page)
{
    uint8_t reg = OSD_readRegister(0xF3);
    reg = (reg & 0xF8) | (page & 0x07);
    OSD_sendCommand(0xF3, 0x3F, reg);
}

// ====================================================================================
// Sync Polarity Configuration (0x3FF0)
// ====================================================================================
// Bit 7: VSP - V-SYNC active edge (0=falling, 1=rising)
// Bit 6: HSP - H-SYNC active edge (0=falling, 1=rising)
// Bit 5-0: LD[5:0] - Line duration

// Set sync polarities
// vSyncRising: true = V-SYNC triggers on rising edge
// hSyncRising: true = H-SYNC triggers on rising edge
inline void OSD_setSyncPolarity(bool vSyncRising, bool hSyncRising)
{
    uint8_t reg = OSD_readRegister(0xF0);
    if (vSyncRising) reg |= 0x80;
    else reg &= ~0x80;
    if (hSyncRising) reg |= 0x40;
    else reg &= ~0x40;
    OSD_sendCommand(0xF0, 0x3F, reg);
}

// Set line duration (pixels per line / 12)
// Affects horizontal character size
// value: 0-63 (line duration = (value+1) * 12 pixel periods)
inline void OSD_setLineDuration(uint8_t value)
{
    uint8_t reg = OSD_readRegister(0xF0);
    reg = (reg & 0xC0) | (value & 0x3F);
    OSD_sendCommand(0xF0, 0x3F, reg);
}

// ====================================================================================
// Character Height Configuration (0x3FF2)
// ====================================================================================
// CH[5:0]: Height of character strips in scan lines
// Slice interpolation: SLICE = round(SCAN_LINE * 18 / CH)

// Set character height in scan lines
// height: 1-63 scan lines per character row
inline void OSD_setCharacterHeight(uint8_t height)
{
    if (height < 1) height = 1;
    if (height > 63) height = 63;
    OSD_sendCommand(0xF2, 0x3F, height);
}

// ====================================================================================
// Horizontal Position Configuration (0x3FF1)
// ====================================================================================
// DD[7:0]: Horizontal delay from H-SYNC to first character
// Delay = (DD + 8) * 3 pixel periods
// Minimum value: 4 (36 pixel periods = 3 character widths)

// Set horizontal display position
// delay: 4-255 (actual delay = (delay+8) * 3 pixel periods)
inline void OSD_setHorizontalPosition(uint8_t delay)
{
    if (delay < 4) delay = 4;
    OSD_sendCommand(0xF1, 0x3F, delay);
}

// ====================================================================================
// PLL Configuration
// ====================================================================================

// Set PLL to free-running mode (disables sync lock)
// When enabled, pixel frequency keeps its last value
inline void OSD_setFreeRunning(bool enabled)
{
    uint8_t reg = OSD_readRegister(0xF4);
    if (enabled) reg |= 0x80;
    else reg &= ~0x80;
    OSD_sendCommand(0xF4, 0x3F, reg);
}

// Set frequency multiplier for PLL
// N = 2 * (value + 3), target high frequency ~200MHz
// For 8MHz crystal, use value=10 -> N=26 -> 208MHz
inline void OSD_setFrequencyMultiplier(uint8_t value)
{
    OSD_sendCommand(0xF7, 0x3F, value & 0x0F);
}

// Set initial pixel period (helps PLL converge faster on mode change)
// Calculate: PP = round(8 * 2*(FM+3)*FXTAL / (12*(LD+1)*FHSYNC) - 24)
inline void OSD_setInitialPixelPeriod(uint8_t value)
{
    OSD_sendCommand(0xF6, 0x3F, value);
}

#endif
