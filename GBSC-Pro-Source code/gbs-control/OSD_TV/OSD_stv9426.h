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

extern char currentColor;      // Current character color (was: colour1)
extern char currentRow;        // Current OSD row 0x00/0x02/0x03 (was: number_stroca)
extern char digitPos1;         // Position for ones digit (was: sequence_number1)
extern char digitPos2;         // Position for tens digit (was: sequence_number2)
extern char digitPos3;         // Position for hundreds digit (was: sequence_number3)

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
static uint8_t OSD_HEADER           = OSD_COLOR(OSD_FG_BLACK, OSD_BG_YELLOW); // 0x60 - Section headers

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

// Clear symbols on row 1
inline void OSD_clearRow1Symbols()
{
    for (byte addr = 0x00; addr < 0x3F; ++addr) {
        OSD_sendCommand(addr, 0x00, 0x00);
    }
}

// Clear colors on row 1
inline void OSD_clearRow1Colors()
{
    for (byte addr = 0x00; addr < 0xFF; ++addr) {
        OSD_sendCommand(addr, 0x00, 0xC0);
    }
}

// Clear symbols on row 2
inline void OSD_clearRow2Symbols()
{
    for (byte addr = 0x00; addr < 0xFF; ++addr) {
        OSD_sendCommand(addr, 0x02, 0x00);
    }
}

// Clear colors on row 2
inline void OSD_clearRow2Colors()
{
    for (byte addr = 0x00; addr < 0xFF; ++addr) {
        OSD_sendCommand(addr, 0x02, 0xC0);
    }
}

// Clear symbols on row 3
inline void OSD_clearRow3Symbols()
{
    for (byte addr = 0x00; addr < 0xFF; ++addr) {
        OSD_sendCommand(addr, 0x03, 0x00);
    }
}

// Clear colors on row 3
inline void OSD_clearRow3Colors()
{
    for (byte addr = 0x00; addr < 0xFF; ++addr) {
        OSD_sendCommand(addr, 0x03, 0xC0);
    }
}

// ====================================================================================
// Character Write Functions (write char + color at position)
// ====================================================================================

// Write character with color on row 1
// charCode: ASCII character code (n0-n9, A-Z, a-z, etc.)
// pos: horizontal position (P0-P27)
// color: color code (OSD_TEXT_NORMAL, OSD_TEXT_SELECTED, etc.)
inline void OSD_writeCharRow1(volatile int charCode, volatile int pos, volatile int color)
{
    OSD_sendCommand(pos, 0x00, charCode);      // Write character
    OSD_sendCommand(pos - 1, 0x00, color);     // Write color
}

// Write character with color on row 2
inline void OSD_writeCharRow2(volatile int charCode, volatile int pos, volatile int color)
{
    OSD_sendCommand(pos, 0x02, charCode);
    OSD_sendCommand(pos - 1, 0x02, color);
}

// Write character with color on row 3
inline void OSD_writeCharRow3(volatile int charCode, volatile int pos, volatile int color)
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
inline void writeChar(volatile int symbol, volatile int pos)
{
    OSD_sendCommand(pos, currentRow, symbol);
    OSD_sendCommand(pos - 1, currentRow, currentColor);
}

// ====================================================================================
// Profile Name Display
// ====================================================================================

// Display 9-character profile name at positions P15-P23
inline void displayProfileName()
{
    // Positions _15 to _23: _15=0x1F, each position is +2
    for (uint8_t i = 0; i < 9; ++i) {
        writeChar(profileChars[i], 0x1F + i * 2);  // _15=0x1F, _16=0x21, ...
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
inline void clearRowContent(char row, char endPos, char startPos)
{
    currentRow = row;
    currentColor = OSD_BACKGROUND;
    // Position mapping: case N writes to position _(N-1)
    // i.e., case 1 -> _0, case 2 -> _1, etc.
    for (byte pos = startPos; pos < endPos; ++pos) {
        // Calculate position: _N = 0x01 + N*2, so for case N we need _(N-1) = 0x01 + (N-1)*2
        uint8_t osdPos = 0x01 + (pos - 1) * 2;
        writeChar(o, osdPos);
    }
}

// ====================================================================================
// Background Fill
// ====================================================================================

// Fill row background with specified color
// row: ROW_1/ROW_2/ROW_3
// length: number of positions to fill
// color: fill color
inline void fillRowBackground(char row, char length, char color)
{
    currentRow = row;
    for (byte addr = 0x00; addr < length; ++addr) {
        OSD_sendCommand(addr, currentRow, color);
    }
}

// Fill all 3 rows with background color
inline void OSD_fillBackground()
{
    fillRowBackground(ROW_1, _27, OSD_BACKGROUND);
    fillRowBackground(ROW_2, _27, OSD_BACKGROUND);
    fillRowBackground(ROW_3, _27, OSD_BACKGROUND);
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
inline void displayNumber3Digit(byte value)
{
    writeChar(digitChars[value % 10], digitPos1);         // units
    writeChar(digitChars[(value / 10) % 10], digitPos2);  // tens
    writeChar(digitChars[value / 100], digitPos3);        // hundreds
}


// ====================================================================================
// Display Inverted 3-Digit Number (255-value) on Row 1
// ====================================================================================

// Display (255 - value) as 3 decimal digits at fixed positions P19/P20/P21 on row 1
// Always uses OSD_TEXT_NORMAL color
// Example: displayNumber3DigitInverted(0) displays "255", displayNumber3DigitInverted(255) displays "000"
inline void displayNumber3DigitInverted(byte value)
{
    byte inverted = 255 - value;
    OSD_writeCharRow1(digitChars[inverted % 10], P21, OSD_TEXT_NORMAL);         // units
    OSD_writeCharRow1(digitChars[(inverted / 10) % 10], P20, OSD_TEXT_NORMAL);  // tens
    OSD_writeCharRow1(digitChars[inverted / 100], P19, OSD_TEXT_NORMAL);        // hundreds
}

#endif
