/*
   STV9426 OSD chip library for ESP8266 (Arduino framework)

   Original by: Karabanov Aleksandr (2024-02-16)
   https://www.youtube.com/channel/UCkNk5gQMIu8k9xW-uENsBzw

   Refactored: Variable names translated from Russian for readability

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
// Global State Variables
// ====================================================================================

extern uint8_t currentColor;   // Current character color (was: colour1)
extern uint8_t currentRow;     // Current OSD row 0x00/0x02/0x03 (was: number_stroca)
extern uint8_t digitPos1;      // Position for ones digit (was: sequence_number1)
extern uint8_t digitPos2;      // Position for tens digit (was: sequence_number2)
extern uint8_t digitPos3;      // Position for hundreds digit (was: sequence_number3)

// ====================================================================================
// OSD Font Character Codes (ASCII mapping for STV9426)
// ====================================================================================

// Digit characters '0'-'9'
static const uint8_t n0 = 0x30, n1 = 0x31, n2 = 0x32, n3 = 0x33, n4 = 0x34,
                     n5 = 0x35, n6 = 0x36, n7 = 0x37, n8 = 0x38, n9 = 0x39;

// Digit characters (alternate naming)
static const uint8_t _0_ = 0x30, _1_ = 0x31, _2_ = 0x32, _3_ = 0x33, _4_ = 0x34,
                     _5_ = 0x35, _6_ = 0x36, _7_ = 0x37, _8_ = 0x38, _9_ = 0x39;

// Lowercase letters 'a'-'z'
static const uint8_t a = 0x61, b = 0x62, c = 0x63, d = 0x64, e = 0x65, f = 0x66,
                     g = 0x67, h = 0x68, i = 0x69, j = 0x6A, k = 0x6B, l = 0x6C,
                     m = 0x6D, n = 0x6E, o = 0x6F, p = 0x70, q = 0x71, r = 0x72,
                     s = 0x73, t = 0x74, u = 0x75, v = 0x76, w = 0x77, x = 0x78,
                     y = 0x79, z = 0x7A;

// Uppercase letters 'A'-'Z'
static const uint8_t A = 0x41, B = 0x42, C = 0x43, D = 0x44, E = 0x45, F = 0x46,
                     G = 0x47, H = 0x48, I = 0x49, J = 0x4A, K = 0x4B, L = 0x4C,
                     M = 0x4D, N = 0x4E, O = 0x4F, P = 0x50, Q = 0x51, R = 0x52,
                     S = 0x53, T = 0x54, U = 0x55, V = 0x56, W = 0x57, X = 0x58,
                     Y = 0x59, Z = 0x5A;

// Special icon characters
static const uint8_t icon1 = 0x09, icon2 = 0x19, icon3 = 0x05,
                     icon4 = 0x15, icon5 = 0x06, icon6 = 0x16;

// Special characters: = 0x3D, - 0x3E, . 0x2E, / 0x2F, : 0x3A, ' 0x27

// ====================================================================================
// OSD Character Positions (horizontal positions P0-P27)
// ====================================================================================
// Each position is 2 bytes apart: P0=0x01, P1=0x03, ..., Pn=0x01+n*2

static const uint8_t P0 = 0x01, P1 = 0x03, P2 = 0x05, P3 = 0x07, P4 = 0x09,
                     P5 = 0x0B, P6 = 0x0D, P7 = 0x0F, P8 = 0x11, P9 = 0x13, P10 = 0x15,
                     P11 = 0x17, P12 = 0x19, P13 = 0x1B, P14 = 0x1D, P15 = 0x1F,
                     P16 = 0x21, P17 = 0x23, P18 = 0x25, P19 = 0x27, P20 = 0x29,
                     P21 = 0x2B, P22 = 0x2D, P23 = 0x2F, P24 = 0x31, P25 = 0x33,
                     P26 = 0x35, P27 = 0x37;
// P28 = 0x39, P29 = 0x3B, P30 = 0x3D (unused)

// Alternate position naming (underscore prefix)
static const uint8_t _0 = 0x01, _1 = 0x03, _2 = 0x05, _3 = 0x07, _4 = 0x09,
                     _5 = 0x0B, _6 = 0x0D, _7 = 0x0F, _8 = 0x11, _9 = 0x13, _10 = 0x15,
                     _11 = 0x17, _12 = 0x19, _13 = 0x1B, _14 = 0x1D, _15 = 0x1F,
                     _16 = 0x21, _17 = 0x23, _18 = 0x25, _19 = 0x27, _20 = 0x29,
                     _21 = 0x2B, _22 = 0x2D, _23 = 0x2F, _24 = 0x31, _25 = 0x33,
                     _26 = 0x35, _27 = 0x37;

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

// Text colors
static uint8_t OSD_TEXT_NORMAL      = OSD_COLOR(OSD_FG_WHITE, OSD_BG_BLUE);   // 0x17 - Normal menu text
static uint8_t OSD_TEXT_SELECTED    = OSD_COLOR(OSD_FG_YELLOW, OSD_BG_BLUE);  // 0x16 - Selected/highlighted item
static uint8_t OSD_TEXT_DISABLED    = OSD_COLOR(OSD_FG_RED, OSD_BG_BLUE);     // 0x14 - Unavailable option

// Navigation elements
static uint8_t OSD_ICON_PAGE        = OSD_COLOR(OSD_FG_GREEN, OSD_BG_BLUE);   // 0x12 - Page numbers, nav arrows
static uint8_t OSD_CURSOR_ACTIVE    = OSD_COLOR(OSD_FG_BLACK, OSD_BG_YELLOW); // 0x60 - Active row cursor
static uint8_t OSD_CURSOR_INACTIVE  = OSD_COLOR(OSD_FG_BLUE, OSD_BG_BLUE);    // 0x11 - Inactive row cursor

// Background
static uint8_t OSD_BACKGROUND       = OSD_COLOR(OSD_FG_BLUE, OSD_BG_BLUE);    // 0x11 - Menu background fill

// ====================================================================================
// OSD Row Identifiers
// ====================================================================================
// ROW_1 = 0x00 (top), ROW_2 = 0x02 (middle), ROW_3 = 0x03 (bottom)

static const uint8_t ROW_1 = 0x00;
static const uint8_t ROW_2 = 0x02;
static const uint8_t ROW_3 = 0x03;

// ====================================================================================
// Profile Name Characters (9 character positions)
// ====================================================================================

extern char profileChars[9];  // 9-character profile name array

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

// Legacy wrappers for backwards compatibility
inline void OSD_clearRow1Symbols() { OSD_clearRowSymbols(ROW_1); }
inline void OSD_clearRow2Symbols() { OSD_clearRowSymbols(ROW_2); }
inline void OSD_clearRow3Symbols() { OSD_clearRowSymbols(ROW_3); }
inline void OSD_clearRow1Colors()  { OSD_clearRowColors(ROW_1); }
inline void OSD_clearRow2Colors()  { OSD_clearRowColors(ROW_2); }
inline void OSD_clearRow3Colors()  { OSD_clearRowColors(ROW_3); }

// ====================================================================================
// Character Write Functions (write char + color at position)
// ====================================================================================

// Write character with color on row 1
// charCode: ASCII character code (n0-n9, A-Z, a-z, etc.)
// pos: horizontal position (P0-P27)
// color: color code (OSD_TEXT_NORMAL, OSD_TEXT_SELECTED, etc.)
inline void OSD_writeCharRow1(uint8_t charCode, uint8_t pos, uint8_t color)
{
    OSD_sendCommand(pos, 0x00, charCode);      // Write character
    OSD_sendCommand(pos - 1, 0x00, color);     // Write color
}

// Write character with color on row 2
inline void OSD_writeCharRow2(uint8_t charCode, uint8_t pos, uint8_t color)
{
    OSD_sendCommand(pos, 0x02, charCode);
    OSD_sendCommand(pos - 1, 0x02, color);
}

// Write character with color on row 3
inline void OSD_writeCharRow3(uint8_t charCode, uint8_t pos, uint8_t color)
{
    OSD_sendCommand(pos, 0x03, charCode);
    OSD_sendCommand(pos - 1, 0x03, color);
}

// ====================================================================================
// Dynamic Row Character Write
// ====================================================================================

// Write character at position using currentRow and currentColor globals
// symbol: ASCII character code
// pos: horizontal position (P0-P27 or _0-_27)
inline void OSD_writeCharAt(uint8_t symbol, uint8_t pos)
{
    OSD_sendCommand(pos, currentRow, symbol);
    OSD_sendCommand(pos - 1, currentRow, currentColor);
}

// ====================================================================================
// String Write Functions
// ====================================================================================

// Write character at logical position (0-27) using currentRow and currentColor
// charCode: ASCII character code
// pos: logical position (0-27, converted to P0-P27 internally)
inline void OSD_writeChar(uint8_t charCode, uint8_t pos)
{
    OSD_writeCharAt(charCode, (pos * 2) + 1);  // Convert logical pos to hardware pos
}

// Write null-terminated string starting at logical position
// Uses currentRow and currentColor globals
// start: logical starting position (0-27), or 0xFF to continue from last position
// str: null-terminated ASCII string
// Note: spaces are skipped (not written), special chars are converted
static uint8_t _osd_string_last_pos = 0;

inline void OSD_writeString(uint8_t start, const char* str)
{
    if (str == NULL) return;

    if (start == 0xFF)
        start = _osd_string_last_pos;
    else
        _osd_string_last_pos = start;

    for (uint8_t i = 0; str[i] != '\0'; i++) {
        _osd_string_last_pos = i + start + 1;

        if (str[i] == ' ')
            continue;  // Skip spaces
        else if (str[i] == '=')
            OSD_writeChar(0x3D, i + start);
        else if (str[i] == '.')
            OSD_writeChar(0x2E, i + start);
        else if (str[i] == '\'')
            OSD_writeChar(0x27, i + start);
        else if (str[i] == '-')
            OSD_writeChar(0x3E, i + start);
        else if (str[i] == '/')
            OSD_writeChar(0x2F, i + start);
        else if (str[i] == ':')
            OSD_writeChar(0x3A, i + start);
        else
            OSD_writeChar(str[i], i + start);
    }
}

// Write null-terminated string on specific row at logical position
// row: 1, 2, or 3
// startPos: logical starting position (0-27)
// str: null-terminated ASCII string
// Note: spaces are written with OSD_BACKGROUND color
inline void OSD_writeStringAtLine(uint8_t row, uint8_t startPos, const char* str)
{
    void (*rowFunc)(uint8_t, uint8_t, uint8_t);
    if (row == 1) rowFunc = OSD_writeCharRow1;
    else if (row == 2) rowFunc = OSD_writeCharRow2;
    else rowFunc = OSD_writeCharRow3;

    uint8_t pos = startPos;
    while (*str != '\0') {
        uint8_t hwPos = 1 + pos * 2;  // Convert to hardware position

        if (*str == ' ')
            rowFunc(*str, hwPos, OSD_BACKGROUND);
        else if (*str == '=')
            rowFunc(0x3D, hwPos, OSD_TEXT_NORMAL);
        else if (*str == '.')
            rowFunc(0x2E, hwPos, OSD_TEXT_NORMAL);
        else if (*str == '\'')
            rowFunc(0x27, hwPos, OSD_TEXT_NORMAL);
        else if (*str == '-')
            rowFunc(0x3E, hwPos, OSD_TEXT_NORMAL);
        else if (*str == '/')
            rowFunc(0x2F, hwPos, OSD_TEXT_NORMAL);
        else if (*str == ':')
            rowFunc(0x3A, hwPos, OSD_TEXT_NORMAL);
        else
            rowFunc(*str, hwPos, OSD_TEXT_NORMAL);

        pos++;
        str++;
    }
}

// Draw dashes on a row from startPos to endPos (logical positions 0-27)
// row: 1, 2, or 3
// startPos: starting logical position
// endPos: ending logical position (inclusive)
inline void OSD_drawDashRange(uint8_t row, uint8_t startPos, uint8_t endPos)
{
    void (*rowFunc)(uint8_t, uint8_t, uint8_t);
    if (row == 1) rowFunc = OSD_writeCharRow1;
    else if (row == 2) rowFunc = OSD_writeCharRow2;
    else rowFunc = OSD_writeCharRow3;

    for (uint8_t p = startPos; p <= endPos; p++) {
        rowFunc(0x3E, 0x01 + p * 2, OSD_TEXT_NORMAL);
    }
}

// Write ON or OFF indicator at end of row (positions P23-P25)
// row: 1, 2, or 3
// isOn: true = "ON", false = "OFF"
inline void OSD_writeOnOff(uint8_t row, bool isOn)
{
    void (*rowFunc)(uint8_t, uint8_t, uint8_t);
    if (row == 1) rowFunc = OSD_writeCharRow1;
    else if (row == 2) rowFunc = OSD_writeCharRow2;
    else rowFunc = OSD_writeCharRow3;

    rowFunc(O, P23, OSD_TEXT_NORMAL);
    if (isOn) {
        rowFunc(N, P24, OSD_TEXT_NORMAL);
        rowFunc(F, P25, OSD_BACKGROUND);  // Hide 'F' for "ON"
    } else {
        rowFunc(F, P24, OSD_TEXT_NORMAL);
        rowFunc(F, P25, OSD_TEXT_NORMAL);
    }
}

// ====================================================================================
// Clear All OSD Rows
// ====================================================================================

inline void OSD_clearAll()
{
    OSD_clearRow1Symbols();
    OSD_clearRow2Symbols();
    OSD_clearRow3Symbols();
    OSD_clearRow1Colors();
    OSD_clearRow2Colors();
    OSD_clearRow3Colors();
}

// ====================================================================================
// Row Content Clear (fills positions with 'o' character in background color)
// ====================================================================================

// Clear row content from startPos to endPos (1-28 range)
// row: ROW_1/ROW_2/ROW_3
// endPos: clear up to this position (exclusive)
// startPos: start from this position (1-28)
inline void OSD_clearRowContent(char row, char endPos, char startPos)
{
    currentRow = row;
    currentColor = OSD_BACKGROUND;
    // Position mapping: case N writes to position _(N-1)
    // i.e., case 1 -> _0, case 2 -> _1, etc.
    for (byte pos = startPos; pos < endPos; ++pos) {
        // Calculate position: _N = 0x01 + N*2, so for case N we need _(N-1) = 0x01 + (N-1)*2
        uint8_t osdPos = 0x01 + (pos - 1) * 2;
        OSD_writeCharAt(o, osdPos);
    }
}

// ====================================================================================
// Background Fill
// ====================================================================================

// Fill row background with specified color
// row: ROW_1/ROW_2/ROW_3
// length: number of positions to fill
// color: fill color
inline void OSD_fillRowBackground(char row, char length, char color)
{
    currentRow = row;
    for (byte addr = 0x00; addr < length; ++addr) {
        OSD_sendCommand(addr, currentRow, color);
    }
}

// Fill all 3 rows with background color
inline void OSD_fillBackground()
{
    OSD_fillRowBackground(ROW_1, _27, OSD_BACKGROUND);
    OSD_fillRowBackground(ROW_2, _27, OSD_BACKGROUND);
    OSD_fillRowBackground(ROW_3, _27, OSD_BACKGROUND);
}

// ====================================================================================
// Digit Character Lookup Table
// ====================================================================================

// Maps digit 0-9 to OSD character codes (n0-n9)
static const uint8_t digitChars[10] = {n0, n1, n2, n3, n4, n5, n6, n7, n8, n9};

// ====================================================================================
// Display 3-Digit Decimal Number (0-255)
// ====================================================================================

// Display a byte value (0-255) as 3 decimal digits at positions digitPos1/2/3
// Uses currentRow and currentColor globals
// Example: displayNumber3Digit(123) displays "1" "2" "3" at digitPos3, digitPos2, digitPos1
inline void OSD_displayNumber3Digit(byte value)
{
    OSD_writeCharAt(digitChars[value % 10], digitPos1);         // units
    OSD_writeCharAt(digitChars[(value / 10) % 10], digitPos2);  // tens
    OSD_writeCharAt(digitChars[value / 100], digitPos3);        // hundreds
}


// ====================================================================================
// Display Inverted 3-Digit Number (255-value) on Row 1
// ====================================================================================

// Display (255 - value) as 3 decimal digits at fixed positions P19/P20/P21 on row 1
// Always uses OSD_TEXT_NORMAL color
// Example: displayNumber3DigitInverted(0) displays "255", displayNumber3DigitInverted(255) displays "000"
inline void OSD_displayNumber3DigitInverted(byte value)
{
    byte inverted = 255 - value;
    OSD_writeCharRow1(digitChars[inverted % 10], P21, OSD_TEXT_NORMAL);         // units
    OSD_writeCharRow1(digitChars[(inverted / 10) % 10], P20, OSD_TEXT_NORMAL);  // tens
    OSD_writeCharRow1(digitChars[inverted / 100], P19, OSD_TEXT_NORMAL);        // hundreds
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
