// ====================================================================================
// osd-render-pro.cpp
// TV OSD Menu Rendering and Handlers
//
// This file contains:
// - menuTable[]: Command dispatch table
// - OSD_handleCommand(): Menu command dispatcher
// - OSD feedback functions (limit, OK, saving, arrows, highlight)
//
// Handler functions are now in separate files under osd/ directory:
// - osd/osd-main.cpp: MainMenu, Cursor handlers
// - osd/osd-output.cpp: Output Resolution handlers
// - osd/osd-screen.cpp: Screen Settings handlers
// - osd/osd-color.cpp: Color Settings handlers
// - osd/osd-system.cpp: System Settings, Developer handlers
// - osd/osd-profile.cpp: Profile handlers
// - osd/osd-input.cpp: Input Menu, ADC Calibration handlers
// ====================================================================================

#include "osd/osd-common.h"

// ====================================================================================
// TV OSD Feedback Functions (used by menu navigation)
// ====================================================================================

// Show "limit" feedback on TV OSD row, then clear (blocking)
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
void OSD_showOkFeedback(uint8_t row, int iterations) {
    uint8_t logicalRow = OSD_bankToRow(row);
    for (int p = 0; p <= iterations; p++) {
        OSD_writeStringAtRow(logicalRow, 25, "OK", OSD_TEXT_DISABLED);
    }
    OSD_writeStringAtRow(logicalRow, 25, "OK", OSD_BACKGROUND);
}

// Show "saving" feedback on TV OSD row, then clear (blocking)
void OSD_showSavingFeedback(uint8_t row, uint8_t startPos, int iterations) {
    uint8_t logicalRow = OSD_bankToRow(row);
    for (int p = 0; p <= iterations; p++) {
        OSD_writeStringAtRow(logicalRow, startPos, "saving", OSD_TEXT_DISABLED);
    }
    OSD_writeStringAtRow(logicalRow, startPos, "saving", OSD_BACKGROUND);
}

// Show 4-direction adjustment arrows on TV OSD row
void OSD_showAdjustArrows(uint8_t row, uint8_t dashStart) {
    OSD_drawDashRange(row, dashStart, 13);
    OSD_writeCharAtRow(row, 0x03, 14, OSD_CURSOR_ACTIVE);  // up arrow
    OSD_writeCharAtRow(row, 0x08, 15, OSD_CURSOR_ACTIVE);  // left arrow
    OSD_writeCharAtRow(row, 0x18, 16, OSD_CURSOR_ACTIVE);  // right arrow
    OSD_writeCharAtRow(row, 0x13, 17, OSD_CURSOR_ACTIVE);  // down arrow
}

// Highlight menu icon at position (1=top, 2=mid, 3=bottom)
void OSD_highlightIcon(uint8_t pos) {
    OSD_writeCharAtRow(1, icon4, 0, pos == 1 ? OSD_CURSOR_ACTIVE : OSD_CURSOR_INACTIVE);
    OSD_writeCharAtRow(2, icon4, 0, pos == 2 ? OSD_CURSOR_ACTIVE : OSD_CURSOR_INACTIVE);
    OSD_writeCharAtRow(3, icon4, 0, pos == 3 ? OSD_CURSOR_ACTIVE : OSD_CURSOR_INACTIVE);
}

// Draw dashes on a row from startPos to endPos (logical positions 0-27)
void OSD_drawDashRange(uint8_t row, uint8_t startPos, uint8_t endPos) {
    for (uint8_t p = startPos; p <= endPos; p++) {
        OSD_writeCharAtRow(row, 0x3E, p, OSD_TEXT_NORMAL);
    }
}

// Write ON or OFF indicator at end of row (positions 23-25), right-aligned
void OSD_writeOnOff(uint8_t row, bool isOn) {
    OSD_writeStringAtRow(row, 23, isOn ? " ON" : "OFF");
}

// ====================================================================================
// MENU TABLE
// ====================================================================================

const MenuEntry menuTable[] = {
    // Initialization (fill background once when menu opens)
    {OSD_CMD_INIT, handle_OSD_Init, false},

    // Cursor Positioning (not saveable)
    {OSD_CMD_CURSOR_ROW1, handle_HighlightRow1, false},
    {OSD_CMD_CURSOR_ROW2, handle_HighlightRow2, false},
    {OSD_CMD_CURSOR_ROW3, handle_HighlightRow3, false},

    // Main Menu (not saveable)
    {OSD_CMD_MAIN_PAGE1, handle_MainMenu_Page1, false},
    {OSD_CMD_MAIN_PAGE2, handle_MainMenu_Page2, false},

    // Output Resolution (not saveable)
    {OSD_CMD_OUTPUT_1080_1024_960, handle_OutputRes_1080_1024_960, false},
    {OSD_CMD_OUTPUT_720_480,       handle_OutputRes_720_480,       false},
    {OSD_CMD_OUTPUT_PASSTHROUGH,   handle_OutputRes_PassThrough,   false},

    // Screen Settings
    {OSD_CMD_SCREEN_SETTINGS,        handle_ScreenSettings,          false},
    {OSD_CMD_SCREEN_FULLHEIGHT,      handle_ScreenSettings_FullHeight, true},  // saveable
    {OSD_CMD_SCREEN_FULLHEIGHT_VALUES, handle_ScreenFullHeight_Values, false},

    // Color Settings (Picture Settings menu)
    {OSD_CMD_COLOR_PAGE1,        handle_ColorSettings_Page1,        true},   // R, G, B
    {OSD_CMD_COLOR_PAGE1_VALUES, handle_ColorSettings_Page1_Values, false},
    {OSD_CMD_COLOR_PAGE2,        handle_ColorSettings_Page2,        true},   // ADC gain, Scanlines, Line filter
    {OSD_CMD_COLOR_PAGE2_VALUES, handle_ColorSettings_Page2_Values, false},
    {OSD_CMD_COLOR_PAGE3,        handle_ColorSettings_Page3,        true},   // Sharpness, Peaking, Step response
    {OSD_CMD_COLOR_PAGE3_VALUES, handle_ColorSettings_Page3_Values, false},
    {OSD_CMD_COLOR_PAGE4,        handle_ColorSettings_Page4,        true},   // Y Gain, Color, Default Color
    {OSD_CMD_COLOR_PAGE4_VALUES, handle_ColorSettings_Page4_Values, false},

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

    // SV/AV Input Settings - Page 2
    {OSD_CMD_SVAVINPUT_PAGE2,        handle_SVAVInput_Page2,        true},   // saveable
    {OSD_CMD_SVAVINPUT_PAGE2_VALUES, handle_SVAVInput_Page2_Values, false},

    // Input Menu
    {OSD_CMD_INPUT_PAGE1,   handle_InputMenu_Page1,     true},   // saveable
    {OSD_CMD_INPUT_PAGE2,   handle_InputMenu_Page2,     true},   // saveable
    {OSD_CMD_INPUT_INFO,    handle_InputInfo,           false},
    {OSD_CMD_INPUT_FORMAT,  handle_InfoDisplay,         false},
    {OSD_CMD_INPUT_SOURCE,  handle_InfoDisplay_Source,  false},

    // SV/AV Input Settings - Page 1
    {OSD_CMD_SVAVINPUT_PAGE1,        handle_SVAVInput_Page1,        true},   // saveable
    {OSD_CMD_SVAVINPUT_PAGE1_VALUES, handle_SVAVInput_Page1_Values, false},
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
