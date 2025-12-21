// ====================================================================================
// osd-render-pro.cpp
// TV OSD Menu Rendering and Handlers
//
// This file contains:
// - menuTable[]: Command dispatch table
// - OSD_handleCommand(): Menu command dispatcher
// - handle_X(): Individual menu screen rendering functions
// ====================================================================================

#include "gbs-control-pro.h"
#include "osd-render-pro.h"
#include "options-pro.h"
#include "../options.h"
#include "../tv5725.h"

#include "drivers/stv9426.h"

// ====================================================================================
// External References - gbs-control.ino
// ====================================================================================

typedef TV5725<GBS_ADDR> GBS;
extern struct runTimeOptions *rto;
extern struct userOptions *uopt;

extern boolean areScanLinesAllowed();
extern void saveUserPrefs();
extern void applyPresets(uint8_t videoMode);

// ====================================================================================
// Menu Helper Functions (application-specific, use global menu state)
// ====================================================================================

// Display 9-character profile name at positions P15-P23
// row: 1, 2, or 3
// color: text color
static void displayProfileName(uint8_t row, uint8_t color)
{
    for (uint8_t i = 0; i < 9; ++i) {
        OSD_writeCharAtRow(row, profileChars[i], 15 + i, color);  // positions 15-23
    }
}

// Get color for menu row (1-based)
static inline uint8_t OSD_getMenuLineColor(uint8_t row) {
    if (row < 1 || row > OSD_MAX_MENU_ROWS) return OSD_TEXT_NORMAL;
    return menuLineColors[row - 1];
}

// Set menu line colors based on selection (scalable to OSD_MAX_MENU_ROWS)
// selectedLine: 1-based row number (1 = first row)
static void OSD_setMenuLineColors(uint8_t selectedLine) {
    for (uint8_t i = 0; i < OSD_MAX_MENU_ROWS; i++) {
        menuLineColors[i] = (i + 1 == selectedLine) ? OSD_TEXT_SELECTED : OSD_TEXT_NORMAL;
    }
}

// Set menu line colors with custom color for a specific line
// customRow: 1-based row number for custom color
// customColor: color to apply (e.g., OSD_TEXT_DISABLED)
static void OSD_setMenuLineColorsCustom(uint8_t selectedLine, uint8_t customRow, uint8_t customColor) {
    OSD_setMenuLineColors(selectedLine);  // Set base colors
    if (customRow >= 1 && customRow <= OSD_MAX_MENU_ROWS) {
        // If custom row is selected, keep SELECTED unless it's DISABLED
        if (customRow == selectedLine && customColor != OSD_TEXT_DISABLED) {
            return;  // Keep SELECTED
        }
        menuLineColors[customRow - 1] = customColor;
    }
}

// Write page navigation icons at position P27 (column 27)
// showUp: true = show up arrow (icon5) on row 1
// pageChar: character to show on row 2 (e.g., '1', '2', 'I')
// showDown: true = show down arrow (icon6) on row 3
static void OSD_writePageIcons(bool showUp, uint8_t pageChar, bool showDown)
{
    if (showUp)
        OSD_writeCharAtRow(1, icon5, 27, OSD_ICON_PAGE);
    OSD_writeCharAtRow(2, pageChar, 27, OSD_ICON_PAGE);
    if (showDown)
        OSD_writeCharAtRow(3, icon6, 27, OSD_ICON_PAGE);
}

// ====================================================================================
// TV OSD Feedback Functions (used by menu navigation)
// ====================================================================================

// Show "limit" feedback on TV OSD row, then clear (blocking)
// row: ROW_1, ROW_2, or ROW_3 (hardware bank values)
// iterations: number of loop iterations for delay (~400 = visible flash)
void OSD_showLimitFeedback(uint8_t row, int iterations) {
    uint8_t logicalRow = OSD_bankToRow(row);
    for (int p = 0; p <= iterations; p++) {
        OSD_writeStringAtRow(logicalRow, 20, "limit", OSD_TEXT_DISABLED);
        OSD_writeCharAtRow(logicalRow, 0x0d, 25, OSD_TEXT_DISABLED);
    }
    OSD_writeStringAtRow(logicalRow, 20, "limit", OSD_BACKGROUND);
    OSD_writeCharAtRow(logicalRow, 0x0d, 25, OSD_BACKGROUND);
}

// Show "OK" feedback on TV OSD row, then clear (blocking)
// row: ROW_1, ROW_2, or ROW_3
// iterations: number of loop iterations for delay (~800 = visible flash)
void OSD_showOkFeedback(uint8_t row, int iterations) {
    uint8_t logicalRow = OSD_bankToRow(row);
    for (int p = 0; p <= iterations; p++) {
        OSD_writeStringAtRow(logicalRow, 25, "OK", OSD_TEXT_DISABLED);
    }
    OSD_writeStringAtRow(logicalRow, 25, "OK", OSD_BACKGROUND);
}

// Show "saving" feedback on TV OSD row, then clear (blocking)
// row: ROW_1, ROW_2, or ROW_3
// startPos: starting position for "saving" text (default 19)
// iterations: number of loop iterations for delay (~800 = visible flash)
void OSD_showSavingFeedback(uint8_t row, uint8_t startPos, int iterations) {
    uint8_t logicalRow = OSD_bankToRow(row);
    for (int p = 0; p <= iterations; p++) {
        OSD_writeStringAtRow(logicalRow, startPos, "saving", OSD_TEXT_DISABLED);
    }
    OSD_writeStringAtRow(logicalRow, startPos, "saving", OSD_BACKGROUND);
}

// Show 4-direction adjustment arrows on TV OSD row
// row: 1, 2, or 3
// dashStart: starting position for dashes (default 8)
// Displays dashes (dashStart-13) and arrow icons (14-17)
void OSD_showAdjustArrows(uint8_t row, uint8_t dashStart) {
    OSD_drawDashRange(row, dashStart, 13);
    OSD_writeCharAtRow(row, 0x03, 14, OSD_CURSOR_ACTIVE);  // up arrow
    OSD_writeCharAtRow(row, 0x08, 15, OSD_CURSOR_ACTIVE);  // left arrow
    OSD_writeCharAtRow(row, 0x18, 16, OSD_CURSOR_ACTIVE);  // right arrow
    OSD_writeCharAtRow(row, 0x13, 17, OSD_CURSOR_ACTIVE);  // down arrow
}

// Highlight menu icon at position (1=top, 2=mid, 3=bottom)
// Writes icon4 with active/inactive cursor color
void OSD_highlightIcon(uint8_t pos) {
    OSD_writeCharAtRow(1, icon4, 0, pos == 1 ? OSD_CURSOR_ACTIVE : OSD_CURSOR_INACTIVE);
    OSD_writeCharAtRow(2, icon4, 0, pos == 2 ? OSD_CURSOR_ACTIVE : OSD_CURSOR_INACTIVE);
    OSD_writeCharAtRow(3, icon4, 0, pos == 3 ? OSD_CURSOR_ACTIVE : OSD_CURSOR_INACTIVE);
}

// ====================================================================================
// MENU TABLE
// ====================================================================================

const MenuEntry menuTable[] = {
    // Cursor Positioning (not saveable)
    {OSD_CMD_CURSOR_ROW1, handle_HighlightRow1, false},
    {OSD_CMD_CURSOR_ROW2, handle_HighlightRow2, false},
    {OSD_CMD_CURSOR_ROW3, handle_HighlightRow3, false},

    // Main Menu (not saveable)
    {OSD_CMD_MAIN_PAGE1,        handle_MainMenu_Page1,        false},
    {OSD_CMD_MAIN_PAGE1_UPDATE, handle_MainMenu_Page1_Update, false},
    {OSD_CMD_MAIN_PAGE2,        handle_MainMenu_Page2,        false},

    // Output Resolution (not saveable)
    {OSD_CMD_OUTPUT_1080_1024_960, handle_OutputRes_1080_1024_960, false},
    {OSD_CMD_OUTPUT_720_480,       handle_OutputRes_720_480,       false},
    {OSD_CMD_OUTPUT_PASSTHROUGH,   handle_OutputRes_PassThrough,   false},

    // Screen Settings
    {OSD_CMD_SCREEN_SETTINGS,        handle_ScreenSettings,          false},
    {OSD_CMD_SCREEN_FULLHEIGHT,      handle_ScreenSettings_FullHeight, true},  // saveable
    {OSD_CMD_SCREEN_FULLHEIGHT_VALUES, handle_ScreenFullHeight_Values, false},

    // Color Settings
    {OSD_CMD_COLOR_PAGE1,        handle_ColorSettings_Page1,        true},   // saveable
    {OSD_CMD_COLOR_PAGE1_VALUES, handle_ColorSettings_Page1_Values, false},
    {OSD_CMD_COLOR_PAGE2,        handle_ColorSettings_Page2,        true},   // saveable
    {OSD_CMD_COLOR_PAGE2_VALUES, handle_ColorSettings_Page2_Values, false},
    {OSD_CMD_COLOR_PAGE3,        handle_ColorSettings_Page3,        true},   // saveable
    {OSD_CMD_COLOR_RGB_LABELS,   handle_ColorSettings_RGB_Labels,   true},   // saveable
    {OSD_CMD_COLOR_RGB_VALUES,   handle_ColorSettings_RGB_Values,   false},

    // System Settings - SV/AV Input
    {OSD_CMD_SYS_SVINPUT_VALUES, handle_SysSettings_SVInput_Values, false},

    // System Settings - General
    {OSD_CMD_SYS_PAGE1,        handle_SysSettings_Page1,        true},   // saveable
    {OSD_CMD_SYS_PAGE1_VALUES, handle_SysSettings_Page1_Values, false},
    {OSD_CMD_SYS_PAGE2,        handle_SysSettings_Page2,        true},   // saveable
    {OSD_CMD_SYS_PAGE2_VALUES, handle_SysSettings_Page2_Values, false},
    {OSD_CMD_SYS_PAGE4,        handle_SysSettings_Page4,        true},   // saveable
    {OSD_CMD_SYS_PAGE4_VALUES, handle_SysSettings_Page4_Values, false},
    {OSD_CMD_SYS_PAGE5,        handle_SysSettings_Page5,        false},
    {OSD_CMD_SYS_PAGE5_VALUES, handle_SysSettings_Page5_Values, false},

    // Developer (not saveable)
    {OSD_CMD_DEV_MEMORY,        handle_Developer_Memory,        false},
    {OSD_CMD_DEV_MEMORY_VALUES, handle_Developer_Memory_Values, false},
    {OSD_CMD_DEV_DEBUG,         handle_Developer_Debug,         false},
    {OSD_CMD_DEV_DEBUG_VALUES,  handle_Developer_Debug_Values,  false},

    // Restart (not saveable)
    {OSD_CMD_RESTART, handle_Restart, false},

    // Profile
    {OSD_CMD_PROFILE_SAVELOAD,    handle_Profile_SaveLoad,    true},   // saveable
    {OSD_CMD_PROFILE_SLOTDISPLAY, handle_Profile_SlotDisplay, false},
    {OSD_CMD_PROFILE_SLOTROW1,    handle_Profile_SlotRow1,    false},
    {OSD_CMD_PROFILE_SLOTROW2,    handle_Profile_SlotRow2,    true},   // saveable
    {OSD_CMD_PROFILE_SLOTROW3,    handle_Profile_SlotRow3,    false},

    // Input Menu
    {OSD_CMD_INPUT_PAGE1,   handle_InputMenu_Page1,     true},   // saveable
    {OSD_CMD_INPUT_PAGE2,   handle_InputMenu_Page2,     true},   // saveable
    {OSD_CMD_INPUT_INFO,    handle_InputInfo,           false},
    {OSD_CMD_INPUT_FORMAT,  handle_InfoDisplay,         false},
    {OSD_CMD_INPUT_SOURCE,  handle_InfoDisplay_Source,  false},

    // Calibration
    {OSD_CMD_ADCCALIB_RUNNING, handle_ADCCalib_Running, true},   // saveable
    {OSD_CMD_ADCCALIB_DISPLAY, handle_ADCCalib_Display, false},
};

const size_t menuTableSize = sizeof(menuTable) / sizeof(menuTable[0]);

// ====================================================================================
// MENU DISPATCHER
// ====================================================================================

void OSD_handleCommand(OsdCommand cmd)
{
    for (size_t i = 0; i < menuTableSize; i++) {
        if (menuTable[i].cmd == cmd) {
            if (menuTable[i].saveable) {
                lastOsdCommand = cmd;
            }
            menuTable[i].handler();
            return;
        }
    }
}

// ====================================================================================
// MENU HANDLER FUNCTIONS
// ====================================================================================

void handle_MainMenu_Page1(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_fillBackground();
    OSD_writeCharAtRow(2, icon4, 0, OSD_CURSOR_INACTIVE);
    OSD_writeCharAtRow(3, icon4, 0, OSD_CURSOR_INACTIVE);
    OSD_writeCharAtRow(1, icon4, 0, OSD_CURSOR_ACTIVE);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "1 Input", OSD_getMenuLineColor(1));
    OSD_writeCharAtRow(1, 0x15, 8, OSD_TEXT_SELECTED);
    OSD_writeStringAtRow(2, 1, "2 Output Resolution", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "3 Screen Settings", OSD_getMenuLineColor(3));
}

void handle_MainMenu_Page1_Update(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writeCharAtRow(1, 0x15, 8, (selectedMenuLine == 1) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeCharAtRow(2, 0x15, 20, (selectedMenuLine == 2) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeCharAtRow(3, 0x15, 18, (selectedMenuLine == 3) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "1 Input", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "2 Output Resolution", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "3 Screen Settings", OSD_getMenuLineColor(3));
}

void handle_MainMenu_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writeCharAtRow(1, 0x15, 18, (selectedMenuLine == 1) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeCharAtRow(2, 0x15, 19, (selectedMenuLine == 2) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writePageIcons(true, '2', false);
    OSD_writeStringAtRow(1, 1, "4 System Settings", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "5 Picture Settings", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "6 Reset Settings", OSD_getMenuLineColor(3));
}

void handle_OutputRes_1080_1024_960(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "1920x1080", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "1280x1024", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "1280x960", OSD_getMenuLineColor(3));
}

void handle_OutputRes_720_480(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', false);
    OSD_writeStringAtRow(1, 1, "1280x720", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "480p/576p", OSD_getMenuLineColor(2));
}

void handle_OutputRes_PassThrough(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, 'I', true);
    OSD_writeStringAtRow(1, 1, "Pass through", OSD_getMenuLineColor(1));
}

void handle_ScreenSettings(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "Move", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Scale", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Borders", OSD_getMenuLineColor(3));
}

// Unified row highlight function
static void highlightRow(uint8_t row)
{
    OSD_fillBackground();
    for (uint8_t r = 1; r <= 3; r++) {
        OSD_writeCharAtRow(r, icon4, 0, (r == row) ? OSD_CURSOR_ACTIVE : OSD_BACKGROUND);
    }
    selectedMenuLine = row;
}

void handle_HighlightRow1(void) { highlightRow(1); }
void handle_HighlightRow2(void) { highlightRow(2); }
void handle_HighlightRow3(void) { highlightRow(3); }

void handle_ColorSettings_Page1(void)
{
    // Line 2 (Scanlines) disabled when not allowed
    uint8_t line2Color = areScanLinesAllowed() ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED;
    OSD_setMenuLineColorsCustom(selectedMenuLine, 2, line2Color);
    OSD_writePageIcons(true, '2', true);
    OSD_writeStringAtRow(1, 1, "ADC gain", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Scanlines", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Line filter", OSD_getMenuLineColor(3));
}

void handle_ColorSettings_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '3', true);
    OSD_writeStringAtRow(1, 1, "Sharpness", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Peaking", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Step response", OSD_getMenuLineColor(3));
}

void handle_ColorSettings_Page3(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '4', false);
    OSD_writeStringAtRow(1, 1, "Default Color", OSD_getMenuLineColor(1));
}

void handle_ColorSettings_RGB_Labels(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "R", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "G", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "B", OSD_getMenuLineColor(3));
}

void handle_ColorSettings_Page1_Values(void)
{
    OSD_drawDashRange(1, 9, 18);   // Row 1: pos 9-18
    OSD_writeCharAtRow(1, 0x3E, 22, OSD_TEXT_NORMAL);

    // Only show dashes for scanlines if they are available
    if (areScanLinesAllowed()) {
        OSD_drawDashRange(2, 10, 19);  // Row 2: pos 10-19
        OSD_writeCharAtRow(2, 0x3E, 22, OSD_TEXT_NORMAL);
        OSD_writeOnOff(2, uopt->wantScanlines);
        // Display scanline strength (0x00-0x50 → 00-05)
        osdDisplayValue = uopt->scanlineStrength;
        OSD_writeCharAtRow(2, '0', 21, OSD_TEXT_NORMAL);
        OSD_writeCharAtRow(2, '0' + (osdDisplayValue >> 4), 20, OSD_TEXT_NORMAL);
    }

    OSD_drawDashRange(3, 12, 22);  // Row 3: pos 12-22
    OSD_writeOnOff(3, uopt->wantVdsLineFilter);
    osdDisplayValue = 255 - GBS::ADC_RGCTRL::read();  // Inverted value
    OSD_displayNumber3DigitAtRow(1, osdDisplayValue, 21, 20, 19, OSD_TEXT_NORMAL);
    OSD_writeOnOff(1, uopt->enableAutoGain != 0);
}

void handle_ColorSettings_Page2_Values(void)
{
    OSD_drawDashRange(1, 10, 22);  // Row 1: P10-P22
    OSD_drawDashRange(2, 8, 19);   // Row 2: P8-P19
    if (!isPeakingLocked()) {
        OSD_drawDashRange(2, 20, 22);  // Row 2: P20-P22 (when not locked)
    }

    OSD_drawDashRange(3, 14, 22);  // Row 3: P14-P22
    OSD_writeOnOff(1, GBS::VDS_PK_LB_GAIN::read() != 0x16);

    if (isPeakingLocked()) {
        // Locked state - overwrite dashes and ON/OFF with LOCKED
        OSD_writeStringAtRow(2, 20, "LOCKED");
    } else {
        OSD_writeOnOff(2, uopt->wantPeaking != 0);
    }

    OSD_writeOnOff(3, uopt->wantStepResponse);
}

void handle_ColorSettings_RGB_Values(void)
{
    OSD_drawDashRange(1, 5, 22);  // Row 1: P5-P22
    OSD_drawDashRange(2, 5, 22);  // Row 2: P5-P22
    OSD_drawDashRange(3, 5, 22);  // Row 3: P5-P22
    // Display R, G, B values at P23-P25 on each row
    OSD_displayNumber3DigitAtRow(1, R_VAL, 25, 24, 23, OSD_TEXT_NORMAL);
    OSD_displayNumber3DigitAtRow(2, G_VAL, 25, 24, 23, OSD_TEXT_NORMAL);
    OSD_displayNumber3DigitAtRow(3, B_VAL, 25, 24, 23, OSD_TEXT_NORMAL);
}

void handle_SysSettings_SVInput_Values(void)
{
    OSD_drawDashRange(1, 7, 22);  // Row 1: P7-P22
    OSD_drawDashRange(2, 6, 22);  // Row 2: P6-P22
    OSD_displayNumber3DigitAtRow(1, GBS::VDS_Y_GAIN::read(), 25, 24, 23, OSD_TEXT_NORMAL);
    OSD_displayNumber3DigitAtRow(2, GBS::VDS_VCOS_GAIN::read(), 25, 24, 23, OSD_TEXT_NORMAL);
}

void handle_SysSettings_Page1(void)
{
    // Line 1 (SV/AV Input Settings) disabled when not SV/AV input
    bool isSvAvInput = (inputType == InputTypeSV) || (inputType == InputTypeAV);
    uint8_t line1Color = isSvAvInput ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED;
    OSD_setMenuLineColorsCustom(selectedMenuLine, 1, line1Color);
    OSD_writeCharAtRow(1, 0x15, 21, (selectedMenuLine == 1) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "SV/AV Input Settings", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Compatibility Mode", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Matched presets", OSD_getMenuLineColor(3));
}

void handle_SysSettings_Page1_Values(void)
{
    OSD_drawDashRange(2, 19, 22);  // Row 2: P19-P22
    OSD_writeOnOff(2, rgbComponentMode == 1);
    OSD_drawDashRange(3, 16, 22);  // Row 3: P16-P22
    OSD_writeOnOff(3, uopt->matchPresetSource);
}

void handle_SysSettings_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', true);
    OSD_writeStringAtRow(1, 1, "Deinterlace", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Force:50Hz to 60Hz", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Lock method", OSD_getMenuLineColor(3));
}

void handle_SysSettings_Page2_Values(void)
{
    OSD_drawDashRange(1, 12, 17);  // Row 1: P12-P17

    if (uopt->deintMode == 0) {
        OSD_writeStringAtRow(1, 18, "Adaptive");
    } else {
        OSD_drawDashRange(1, 18, 22);  // Row 1: P18-P22
        OSD_writeStringAtRow(1, 23, "Bob");
    }

    OSD_drawDashRange(2, 19, 22);  // Row 2: P19-P22
    OSD_drawDashRange(3, 12, 13);  // Row 3: P12-P13
    OSD_writeOnOff(2, uopt->PalForce60);

    if (uopt->frameTimeLockMethod == 0) {
        OSD_writeStringAtRow(3, 14, "0Vtotal<VSST");
    } else {
        OSD_writeStringAtRow(3, 14, "1Vtotal only");
    }
}

void handle_SysSettings_Page4(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '3', false);
    OSD_writeStringAtRow(1, 1, "ADC calibration", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Frame Time lock", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "EnableFrameTimeLock", OSD_getMenuLineColor(3));
}

void handle_SysSettings_Page4_Values(void)
{
    OSD_drawDashRange(1, 16, 22);  // Row 1: P16-P22
    OSD_drawDashRange(2, 16, 22);  // Row 2: P16-P22
    OSD_drawDashRange(3, 20, 22);  // Row 3: P20-P22
    OSD_writeOnOff(1, uopt->enableCalibrationADC);
    OSD_writeOnOff(2, uopt->enableFrameTimeLock);
    OSD_writeOnOff(3, !uopt->disableExternalClockGenerator);
}

void handle_ScreenSettings_FullHeight(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', false);
    OSD_writeStringAtRow(1, 1, "Full height", OSD_getMenuLineColor(1));
}

void handle_ScreenFullHeight_Values(void)
{
    OSD_drawDashRange(1, 12, 22);  // Row 1: P12-P22
    OSD_writeOnOff(1, uopt->wantFullHeight);
}

void handle_Developer_Memory(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, 'I', true);
    OSD_writeStringAtRow(1, 1, "MEM left/right", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "HS left/right", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "HTotal", OSD_getMenuLineColor(3));
}

void handle_Developer_Memory_Values(void)
{
    OSD_drawDashRange(1, 15, 22);  // Row 1: pos 15-22
    OSD_writeCharAtRow(1, 0x03, 23, OSD_CURSOR_ACTIVE);
    OSD_writeCharAtRow(1, 0x13, 24, OSD_CURSOR_ACTIVE);
    OSD_drawDashRange(2, 14, 22);  // Row 2: pos 14-22
    OSD_writeCharAtRow(2, 0x03, 23, OSD_CURSOR_ACTIVE);
    OSD_writeCharAtRow(2, 0x13, 24, OSD_CURSOR_ACTIVE);
    OSD_drawDashRange(3, 7, 22);   // Row 3: pos 7-22
    OSD_displayNumber3DigitAtRow(3, GBS::VDS_HSYNC_RST::read(), 25, 24, 23, OSD_TEXT_NORMAL);
}

void handle_Developer_Debug(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, 'I', true);
    OSD_writeStringAtRow(1, 1, "Debug view", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "ADC filter", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Freeze capture", OSD_getMenuLineColor(3));
}

void handle_Developer_Debug_Values(void)
{
    OSD_drawDashRange(1, 11, 22);  // Row 1: P11-P22
    OSD_drawDashRange(2, 11, 22);  // Row 2: P11-P22
    OSD_drawDashRange(3, 15, 22);  // Row 3: P15-P22
    OSD_writeOnOff(1, GBS::ADC_UNUSED_62::read() != 0x00);
    OSD_writeOnOff(2, GBS::ADC_FLTR::read() > 0);
    OSD_writeOnOff(3, GBS::CAPTURE_ENABLE::read() == 0);  // Inverted: ON when frozen (capture disabled)
}

void handle_SysSettings_Page5(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, 'I', true);
    OSD_writeStringAtRow(1, 1, "Enable OTA", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Restart", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Reset defaults", OSD_getMenuLineColor(3));
}

void handle_SysSettings_Page5_Values(void)
{
    OSD_drawDashRange(1, 11, 22);  // Row 1: P11-P22
    OSD_writeOnOff(1, rto->allowUpdatesOTA);
}

void handle_Profile_SaveLoad(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writeStringAtRow(1, 1, "Loadprofile:", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Saveprofile:", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Active save:", OSD_TEXT_SELECTED);
}

// Generate "profile-N" name directly into profileChars[] (index 1-20)
static void setProfileName(uint8_t index) {
    profileChars[0] = 'p';
    profileChars[1] = 'r';
    profileChars[2] = 'o';
    profileChars[3] = 'f';
    profileChars[4] = 'i';
    profileChars[5] = 'l';
    profileChars[6] = 'e';
    if (index < 10) {
        profileChars[7] = 0x3E;  // '-'
        profileChars[8] = '0' + index;
    } else {
        profileChars[7] = '0' + (index / 10);
        profileChars[8] = '0' + (index % 10);
    }
}

void handle_Profile_SlotDisplay(void)
{
    // Row 1: Load profile display (Load1-Load20 → index 1-20)
    int loadIdx = oled_menuItem - OLED_Profile_Load1;
    if (loadIdx >= 0 && loadIdx < 20) {
        setProfileName(loadIdx + 1);
        displayProfileName(1, OSD_TEXT_NORMAL);
    }

    // Row 3: Active save slot (presetSlot 'A'-'T' → index 1-20)
    if (uopt->presetSlot >= 'A' && uopt->presetSlot <= 'T') {
        setProfileName(uopt->presetSlot - 'A' + 1);
        displayProfileName(3, OSD_TEXT_SELECTED);
    }

    // Row 2: Save profile display (Save1-Save20 → index 1-20)
    int saveIdx = oled_menuItem - OLED_Profile_Save1;
    if (saveIdx >= 0 && saveIdx < 20) {
        setProfileName(saveIdx + 1);
        displayProfileName(2, OSD_TEXT_NORMAL);
    }
}

void handle_Profile_SlotRow1(void)
{
    uopt->presetPreference = OutputCustomized;
    saveUserPrefs();
    uopt->presetPreference = OutputCustomized;

    if (rto->videoStandardInput == 14) {
        rto->videoStandardInput = 15;
    } else {
        applyPresets(rto->videoStandardInput);
    }

    saveUserPrefs();
}

void handle_Profile_SlotRow2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writeStringAtRow(1, 1, "Contrast", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Saturation", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Default", OSD_getMenuLineColor(3));
}

void handle_Profile_SlotRow3(void)
{
    OSD_drawDashRange(1, 13, 18);  // Row 1: P13-P18
    OSD_displayNumber3DigitAtRow(1, contrast, 25, 24, 23, OSD_TEXT_NORMAL);
    OSD_drawDashRange(2, 13, 18);  // Row 2: P13-P18
    OSD_displayNumber3DigitAtRow(2, saturation, 25, 24, 23, OSD_TEXT_NORMAL);
}

void handle_ADCCalib_Running(void)
{
    // Line 2 (Smooth) disabled when lineOption is false
    uint8_t line2Color = lineOption ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED;
    OSD_setMenuLineColorsCustom(selectedMenuLine, 2, line2Color);
    OSD_writeStringAtRow(1, 1, "DoubleLine", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Smooth", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Bright", OSD_getMenuLineColor(3));
}

void handle_InputMenu_Page1(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "RGBs", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "RGsB", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "VGA", OSD_getMenuLineColor(3));
}

void handle_InputInfo(void)
{
    // All rows normal (no selection highlight in info screen)
    OSD_setMenuLineColors(0);
    OSD_writeStringAtRow(1, 0, "Whether to keep the settings", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 0, "Restore in ", OSD_getMenuLineColor(2));

    // Show checkmark on selected option (Changes or Recover)
    OSD_writeCharAtRow(3, 0x15, 2, keepSettings ? OSD_TEXT_SELECTED : OSD_BACKGROUND);
    OSD_writeStringAtRow(3, 3, "Changes", OSD_getMenuLineColor(3));
    OSD_writeCharAtRow(3, 0x15, 13, keepSettings ? OSD_BACKGROUND : OSD_TEXT_SELECTED);
    OSD_writeStringAtRow(3, 0xFF, "    Recover", OSD_getMenuLineColor(3));
}

void handle_InputMenu_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', false);
    OSD_writeStringAtRow(1, 1, "YPBPR", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "SV", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "AV", OSD_getMenuLineColor(3));
};

void handle_InfoDisplay(void)
{
    bool isSV = (oled_menuItem == OLED_Input_SV);
    bool isAV = (oled_menuItem == OLED_Input_AV);

    // Clear or show "Format:" label
    if (isSV) {
        OSD_writeStringAtRow(2, 4, "Format:");
        OSD_writeStringAtRow(3, 4, "                      ");
    } else if (isAV) {
        OSD_writeStringAtRow(3, 4, "Format:");
        OSD_writeStringAtRow(2, 4, "                      ");
    } else {
        OSD_writeStringAtRow(2, 4, "                      ");
        OSD_writeStringAtRow(3, 4, "                      ");
    }

    // Display format name for SV or AV
    if (isSV) {
        // Pad format name to fixed width to overwrite previous text
        char padded[16];
        snprintf(padded, sizeof(padded), "%-15s", getVideoFormatName(SVModeOption));
        OSD_writeStringAtRow(2, 11, padded);
    } else if (isAV) {
        char padded[16];
        snprintf(padded, sizeof(padded), "%-15s", getVideoFormatName(AVModeOption));
        OSD_writeStringAtRow(3, 11, padded);
    }
}

void handle_InfoDisplay_Source(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, 'I', true);
    OSD_writeStringAtRow(1, 1, "Setting", OSD_getMenuLineColor(1));
}

void handle_ADCCalib_Display(void)
{
    OSD_drawDashRange(1, 13, 18);  // Row 1: P13-P18
    OSD_drawDashRange(2, 13, 18);  // Row 2: P13-P18
    OSD_drawDashRange(3, 13, 18);  // Row 3: P13-P18

    if (lineOption) {
        OSD_writeStringAtRow(1, 23, "2X");
    } else {
        OSD_writeStringAtRow(1, 23, "1X");
        smoothOption = false;
    }

    OSD_writeOnOff(2, smoothOption);
    OSD_displayNumber3DigitAtRow(3, brightness, 25, 24, 23, OSD_TEXT_NORMAL);
}

void handle_Restart(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '4', false);
    OSD_writeStringAtRow(1, 1, "Matched presets", OSD_getMenuLineColor(1));
}
