// ====================================================================================
// osd-render-pro.cpp
// TV OSD Menu Table and Command Dispatcher
//
// This file contains:
// - menuTable[]: Command dispatch table
// - OSD_handleCommand(): Menu command dispatcher
//
// Helper functions are in osd/osd-common.cpp
// Handler functions are in separate files under osd/ directory
// ====================================================================================

#include "osd/osd-common.h"

// ====================================================================================
// MENU TABLE
// ====================================================================================

const MenuEntry menuTable[] = {
    // Initialization (fill background once when menu opens)
    {OSD_CMD_INIT, handle_OSD_Init, false},

    // Page Change - fill background + select row (not saveable)
    {OSD_CMD_PAGE_CHANGE_ROW1, handle_HighlightRow1, false},
    {OSD_CMD_PAGE_CHANGE_ROW2, handle_HighlightRow2, false},
    {OSD_CMD_PAGE_CHANGE_ROW3, handle_HighlightRow3, false},

    // Main Menu (not saveable)
    {OSD_CMD_MAIN_PAGE1, handle_MainMenu_Page1, false},
    {OSD_CMD_MAIN_PAGE2, handle_MainMenu_Page2, false},

    // Output Resolution (not saveable)
    {OSD_CMD_OUTPUT_1080_1024_960, handle_OutputRes_1080_1024_960, false},
    {OSD_CMD_OUTPUT_720_480,       handle_OutputRes_720_480,       false},
    // {OSD_CMD_OUTPUT_PASSTHROUGH,   handle_OutputRes_PassThrough,   false},

    // Screen Settings
    {OSD_CMD_SCREEN_PAGE1,        handle_ScreenSettings_Page1,        false},
    {OSD_CMD_SCREEN_PAGE2,        handle_ScreenSettings_Page2,        true},   // saveable
    {OSD_CMD_SCREEN_PAGE2_VALUES, handle_ScreenSettings_Page2_Values, false},

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
    {OSD_CMD_INPUT_PAGE1,        handle_InputMenu_Page1,        true},   // saveable
    {OSD_CMD_INPUT_PAGE2,        handle_InputMenu_Page2,        true},   // saveable
    {OSD_CMD_INPUT_PAGE2_VALUES, handle_InputMenu_Page2_Values, false},
    {OSD_CMD_INPUT_INFO,         handle_InputInfo,              false},
    {OSD_CMD_INPUT_SOURCE,       handle_InfoDisplay_Source,     false},

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
