// ====================================================================================
// osd-registry.h
// OSD Menu System Registry - Single Source of Truth
//
// This file contains:
// - OsdCommand enum: All OSD command identifiers
// - MenuEntry struct: Dispatch table entry type
// - OSD_DISPATCH_ENTRIES: X-macro for generating osdDispatchTable[]
// - Handler function declarations
//
// To add a new OSD command:
// 1. Add enum value to OsdCommand
// 2. Add handler declaration
// 3. Add entry to OSD_DISPATCH_ENTRIES X-macro
// 4. Implement handler in appropriate osd-*.cpp file
// ====================================================================================

#pragma once

#include <Arduino.h>

// ====================================================================================
// Constants - Timing
// ====================================================================================

#define OSD_CLOSE_TIME 16000            // 16 sec
#define OSD_MUTE_CLOSE_TIME 3000        // 3 sec (mute display timeout)
#define OSD_VOLUME_CLOSE_TIME 3000      // 3 sec (volume adjustment timeout)
#define OSD_RESOLUTION_UP_TIME 1000     // 1 sec
#define OSD_RESOLUTION_CLOSE_TIME 20000 // 20 sec

// ====================================================================================
// Constants - Menu Rows
// ====================================================================================

// Maximum menu rows supported (can expand 3->6 in future)
#ifndef OSD_MAX_MENU_ROWS
#define OSD_MAX_MENU_ROWS 3
#endif

// ====================================================================================
// OsdCommand Enum - All OSD Command Identifiers
// ====================================================================================

// TV OSD command enum for type-safe menu dispatch
// Naming convention:
//   - Labels handlers: show menu text on the left (e.g., OSD_CMD_COLOR_PAGE1)
//   - Values handlers: show ON/OFF or numeric values on the right (e.g., OSD_CMD_COLOR_PAGE1_VALUES)
typedef enum : uint8_t {
    OSD_CMD_NONE = 0,

    // Initialization (fill background once when menu opens)
    OSD_CMD_INIT,

    // Page Change (fill background + select row)
    OSD_CMD_PAGE_CHANGE_ROW1,
    OSD_CMD_PAGE_CHANGE_ROW2,
    OSD_CMD_PAGE_CHANGE_ROW3,

    // Main Menu
    OSD_CMD_MAIN_PAGE1,
    OSD_CMD_MAIN_PAGE2,
    OSD_CMD_MAIN_PAGE3,
    OSD_CMD_MAIN_PAGE4,

    // Preferences Menu
    OSD_CMD_PREFERENCES_PAGE1,
    OSD_CMD_PREFERENCES_PAGE1_VALUES,

    // Output Resolution
    OSD_CMD_OUTPUT_1080_1024_960,
    OSD_CMD_OUTPUT_720_480,
    // OSD_CMD_OUTPUT_PASSTHROUGH,

    // Screen Settings
    OSD_CMD_SCREEN_PAGE1,
    OSD_CMD_SCREEN_PAGE1_VALUES,
    OSD_CMD_SCREEN_PAGE2,
    OSD_CMD_SCREEN_PAGE2_VALUES,

    // Color Settings (Picture Settings menu)
    OSD_CMD_COLOR_PAGE1,         // R, G, B
    OSD_CMD_COLOR_PAGE1_VALUES,
    OSD_CMD_COLOR_PAGE2,         // ADC gain, Scanlines, Line filter
    OSD_CMD_COLOR_PAGE2_VALUES,
    OSD_CMD_COLOR_PAGE3,         // Sharpness, Peaking, Step response
    OSD_CMD_COLOR_PAGE3_VALUES,
    OSD_CMD_COLOR_PAGE4,         // Y Gain, Color, Default Color
    OSD_CMD_COLOR_PAGE4_VALUES,

    // System Settings - General
    OSD_CMD_SYS_PAGE1,
    OSD_CMD_SYS_PAGE1_VALUES,
    OSD_CMD_SYS_PAGE2,
    OSD_CMD_SYS_PAGE2_VALUES,
    OSD_CMD_SYS_PAGE4,
    OSD_CMD_SYS_PAGE4_VALUES,

    // Developer (6 pages)
    OSD_CMD_DEV_PAGE1,          // MEM, HS, HTotal
    OSD_CMD_DEV_PAGE1_VALUES,
    OSD_CMD_DEV_PAGE2,          // Debug, ADC, Freeze
    OSD_CMD_DEV_PAGE2_VALUES,
    OSD_CMD_DEV_PAGE3,          // Resync, SDRAM, PLL
    OSD_CMD_DEV_PAGE3_VALUES,
    OSD_CMD_DEV_PAGE4,          // Invert, SyncWatch, SyncProc
    OSD_CMD_DEV_PAGE4_VALUES,
    OSD_CMD_DEV_PAGE5,          // Oversamp, SnapFR, IFOffset
    OSD_CMD_DEV_PAGE5_VALUES,
    OSD_CMD_DEV_PAGE6,          // SOG, Reset
    OSD_CMD_DEV_PAGE6_VALUES,

    // Profile
    OSD_CMD_PROFILE_SAVELOAD,
    OSD_CMD_PROFILE_SLOTDISPLAY,
    OSD_CMD_PROFILE_SLOTROW1,

    // Input Menu
    OSD_CMD_INPUT_PAGE1,
    OSD_CMD_INPUT_PAGE2,
    OSD_CMD_INPUT_PAGE2_VALUES,
    OSD_CMD_INPUT_INFO,
    OSD_CMD_INPUT_SOURCE,

    // SV/AV Input Settings
    OSD_CMD_SVAVINPUT_PAGE1,
    OSD_CMD_SVAVINPUT_PAGE1_VALUES,
    OSD_CMD_SVAVINPUT_PAGE2,
    OSD_CMD_SVAVINPUT_PAGE2_VALUES,

    // Firmware Version
    OSD_CMD_FIRMWARE_VERSION,

    // Factory Reset Confirmation
    OSD_CMD_FACTORY_RESET_CONFIRM,

    OSD_CMD_COUNT
} OsdCommand;

// ====================================================================================
// MenuEntry Struct - Dispatch Table Entry Type
// ====================================================================================

typedef struct {
    OsdCommand cmd;
    void (*handler)(void);
} MenuEntry;

// ====================================================================================
// Handler Function Declarations - Initialization
// ====================================================================================

void handle_OSD_Init(void);

// ====================================================================================
// Handler Function Declarations - Main Menu
// ====================================================================================

void handle_MainMenu_Page1(void);
void handle_MainMenu_Page2(void);
void handle_MainMenu_Page3(void);
void handle_MainMenu_Page4(void);
void handle_HighlightRow1(void);
void handle_HighlightRow2(void);
void handle_HighlightRow3(void);

// ====================================================================================
// Handler Function Declarations - Preferences Menu
// ====================================================================================

void handle_Preferences_Page1(void);
void handle_Preferences_Page1_Values(void);

// ====================================================================================
// Handler Function Declarations - Output Resolution
// ====================================================================================

void handle_OutputRes_1080_1024_960(void);
void handle_OutputRes_720_480(void);
// void handle_OutputRes_PassThrough(void);

// ====================================================================================
// Handler Function Declarations - Screen Settings
// ====================================================================================

void handle_ScreenSettings_Page1(void);
void handle_ScreenSettings_Page1_Values(void);
void handle_ScreenSettings_Page2(void);
void handle_ScreenSettings_Page2_Values(void);


// ====================================================================================
// Handler Function Declarations - Color Settings
// ====================================================================================

void handle_ColorSettings_Page1(void);        // R, G, B
void handle_ColorSettings_Page1_Values(void);
void handle_ColorSettings_Page2(void);        // ADC gain, Scanlines, Line filter
void handle_ColorSettings_Page2_Values(void);
void handle_ColorSettings_Page3(void);        // Sharpness, Peaking, Step response
void handle_ColorSettings_Page3_Values(void);
void handle_ColorSettings_Page4(void);        // Y Gain, Color, Default Color
void handle_ColorSettings_Page4_Values(void);

// ====================================================================================
// Handler Function Declarations - System Settings
// ====================================================================================

void handle_SysSettings_Page1(void);
void handle_SysSettings_Page1_Values(void);
void handle_SysSettings_Page2(void);
void handle_SysSettings_Page2_Values(void);
void handle_SysSettings_Page4(void);
void handle_SysSettings_Page4_Values(void);
void handle_SVAVInput_Page1(void);
void handle_SVAVInput_Page1_Values(void);
void handle_SVAVInput_Page2(void);
void handle_SVAVInput_Page2_Values(void);

// ====================================================================================
// Handler Function Declarations - Developer
// ====================================================================================

void handle_Developer_Page1(void);
void handle_Developer_Page1_Values(void);
void handle_Developer_Page2(void);
void handle_Developer_Page2_Values(void);
void handle_Developer_Page3(void);
void handle_Developer_Page3_Values(void);
void handle_Developer_Page4(void);
void handle_Developer_Page4_Values(void);
void handle_Developer_Page5(void);
void handle_Developer_Page5_Values(void);
void handle_Developer_Page6(void);
void handle_Developer_Page6_Values(void);

// ====================================================================================
// Handler Function Declarations - Profile
// ====================================================================================

void handle_Profile_SaveLoad(void);
void handle_Profile_SlotDisplay(void);
void handle_Profile_SlotRow1(void);

// ====================================================================================
// Handler Function Declarations - Input Menu
// ====================================================================================

void handle_InputMenu_Page1(void);
void handle_InputMenu_Page2(void);
void handle_InputMenu_Page2_Values(void);
void handle_InputInfo(void);
void handle_InfoDisplay_Source(void);

// ====================================================================================
// Handler Function Declarations - Firmware Version
// ====================================================================================

void handle_FirmwareVersion(void);

// ====================================================================================
// Handler Function Declarations - Factory Reset Confirmation
// ====================================================================================

void handle_FactoryResetConfirm(void);

// ====================================================================================
// OSD Dispatch Table X-Macro
// Format: DISPATCH_ENTRY(command, handler)
// ====================================================================================

#define OSD_DISPATCH_ENTRIES \
    /* Initialization */ \
    DISPATCH_ENTRY(OSD_CMD_INIT, handle_OSD_Init) \
    \
    /* Page Change - fill background + select row */ \
    DISPATCH_ENTRY(OSD_CMD_PAGE_CHANGE_ROW1, handle_HighlightRow1) \
    DISPATCH_ENTRY(OSD_CMD_PAGE_CHANGE_ROW2, handle_HighlightRow2) \
    DISPATCH_ENTRY(OSD_CMD_PAGE_CHANGE_ROW3, handle_HighlightRow3) \
    \
    /* Main Menu */ \
    DISPATCH_ENTRY(OSD_CMD_MAIN_PAGE1, handle_MainMenu_Page1) \
    DISPATCH_ENTRY(OSD_CMD_MAIN_PAGE2, handle_MainMenu_Page2) \
    DISPATCH_ENTRY(OSD_CMD_MAIN_PAGE3, handle_MainMenu_Page3) \
    DISPATCH_ENTRY(OSD_CMD_MAIN_PAGE4, handle_MainMenu_Page4) \
    \
    /* Output Resolution */ \
    DISPATCH_ENTRY(OSD_CMD_OUTPUT_1080_1024_960, handle_OutputRes_1080_1024_960) \
    DISPATCH_ENTRY(OSD_CMD_OUTPUT_720_480,       handle_OutputRes_720_480) \
    \
    /* Screen Settings */ \
    DISPATCH_ENTRY(OSD_CMD_SCREEN_PAGE1,        handle_ScreenSettings_Page1) \
    DISPATCH_ENTRY(OSD_CMD_SCREEN_PAGE1_VALUES, handle_ScreenSettings_Page1_Values) \
    DISPATCH_ENTRY(OSD_CMD_SCREEN_PAGE2,        handle_ScreenSettings_Page2) \
    DISPATCH_ENTRY(OSD_CMD_SCREEN_PAGE2_VALUES, handle_ScreenSettings_Page2_Values) \
    \
    /* Color Settings (Picture Settings menu) */ \
    DISPATCH_ENTRY(OSD_CMD_COLOR_PAGE1,        handle_ColorSettings_Page1) \
    DISPATCH_ENTRY(OSD_CMD_COLOR_PAGE1_VALUES, handle_ColorSettings_Page1_Values) \
    DISPATCH_ENTRY(OSD_CMD_COLOR_PAGE2,        handle_ColorSettings_Page2) \
    DISPATCH_ENTRY(OSD_CMD_COLOR_PAGE2_VALUES, handle_ColorSettings_Page2_Values) \
    DISPATCH_ENTRY(OSD_CMD_COLOR_PAGE3,        handle_ColorSettings_Page3) \
    DISPATCH_ENTRY(OSD_CMD_COLOR_PAGE3_VALUES, handle_ColorSettings_Page3_Values) \
    DISPATCH_ENTRY(OSD_CMD_COLOR_PAGE4,        handle_ColorSettings_Page4) \
    DISPATCH_ENTRY(OSD_CMD_COLOR_PAGE4_VALUES, handle_ColorSettings_Page4_Values) \
    \
    /* System Settings */ \
    DISPATCH_ENTRY(OSD_CMD_SYS_PAGE1,        handle_SysSettings_Page1) \
    DISPATCH_ENTRY(OSD_CMD_SYS_PAGE1_VALUES, handle_SysSettings_Page1_Values) \
    DISPATCH_ENTRY(OSD_CMD_SYS_PAGE2,        handle_SysSettings_Page2) \
    DISPATCH_ENTRY(OSD_CMD_SYS_PAGE2_VALUES, handle_SysSettings_Page2_Values) \
    DISPATCH_ENTRY(OSD_CMD_SYS_PAGE4,        handle_SysSettings_Page4) \
    DISPATCH_ENTRY(OSD_CMD_SYS_PAGE4_VALUES, handle_SysSettings_Page4_Values) \
    \
    /* Preferences Menu (Theme, Volume, Mute) */ \
    DISPATCH_ENTRY(OSD_CMD_PREFERENCES_PAGE1,        handle_Preferences_Page1) \
    DISPATCH_ENTRY(OSD_CMD_PREFERENCES_PAGE1_VALUES, handle_Preferences_Page1_Values) \
    \
    /* Developer (6 pages) */ \
    DISPATCH_ENTRY(OSD_CMD_DEV_PAGE1,        handle_Developer_Page1) \
    DISPATCH_ENTRY(OSD_CMD_DEV_PAGE1_VALUES, handle_Developer_Page1_Values) \
    DISPATCH_ENTRY(OSD_CMD_DEV_PAGE2,        handle_Developer_Page2) \
    DISPATCH_ENTRY(OSD_CMD_DEV_PAGE2_VALUES, handle_Developer_Page2_Values) \
    DISPATCH_ENTRY(OSD_CMD_DEV_PAGE3,        handle_Developer_Page3) \
    DISPATCH_ENTRY(OSD_CMD_DEV_PAGE3_VALUES, handle_Developer_Page3_Values) \
    DISPATCH_ENTRY(OSD_CMD_DEV_PAGE4,        handle_Developer_Page4) \
    DISPATCH_ENTRY(OSD_CMD_DEV_PAGE4_VALUES, handle_Developer_Page4_Values) \
    DISPATCH_ENTRY(OSD_CMD_DEV_PAGE5,        handle_Developer_Page5) \
    DISPATCH_ENTRY(OSD_CMD_DEV_PAGE5_VALUES, handle_Developer_Page5_Values) \
    DISPATCH_ENTRY(OSD_CMD_DEV_PAGE6,        handle_Developer_Page6) \
    DISPATCH_ENTRY(OSD_CMD_DEV_PAGE6_VALUES, handle_Developer_Page6_Values) \
    \
    /* Profile */ \
    DISPATCH_ENTRY(OSD_CMD_PROFILE_SAVELOAD,    handle_Profile_SaveLoad) \
    DISPATCH_ENTRY(OSD_CMD_PROFILE_SLOTDISPLAY, handle_Profile_SlotDisplay) \
    DISPATCH_ENTRY(OSD_CMD_PROFILE_SLOTROW1,    handle_Profile_SlotRow1) \
    \
    /* SV/AV Input Settings - Page 2 */ \
    DISPATCH_ENTRY(OSD_CMD_SVAVINPUT_PAGE2,        handle_SVAVInput_Page2) \
    DISPATCH_ENTRY(OSD_CMD_SVAVINPUT_PAGE2_VALUES, handle_SVAVInput_Page2_Values) \
    \
    /* Input Menu */ \
    DISPATCH_ENTRY(OSD_CMD_INPUT_PAGE1,        handle_InputMenu_Page1) \
    DISPATCH_ENTRY(OSD_CMD_INPUT_PAGE2,        handle_InputMenu_Page2) \
    DISPATCH_ENTRY(OSD_CMD_INPUT_PAGE2_VALUES, handle_InputMenu_Page2_Values) \
    DISPATCH_ENTRY(OSD_CMD_INPUT_INFO,         handle_InputInfo) \
    DISPATCH_ENTRY(OSD_CMD_INPUT_SOURCE,       handle_InfoDisplay_Source) \
    \
    /* SV/AV Input Settings - Page 1 */ \
    DISPATCH_ENTRY(OSD_CMD_SVAVINPUT_PAGE1,        handle_SVAVInput_Page1) \
    DISPATCH_ENTRY(OSD_CMD_SVAVINPUT_PAGE1_VALUES, handle_SVAVInput_Page1_Values) \
    \
    /* Firmware Version */ \
    DISPATCH_ENTRY(OSD_CMD_FIRMWARE_VERSION, handle_FirmwareVersion) \
    \
    /* Factory Reset Confirmation */ \
    DISPATCH_ENTRY(OSD_CMD_FACTORY_RESET_CONFIRM, handle_FactoryResetConfirm)

// ====================================================================================
// Dispatch Table and Command Handler Declarations
// ====================================================================================

extern const MenuEntry osdDispatchTable[];
extern const size_t osdDispatchTableSize;

void OSD_handleCommand(OsdCommand cmd);

// ====================================================================================
// Misc OSD Display Functions (not part of dispatch table)
// ====================================================================================

// Render mute status on TV OSD (non-blocking, called each frame)
void OSD_renderMuteDisplay(bool muted);

// Display resolution confirmation countdown timer on TV OSD
void OSD_renderResolutionCountdown(uint8_t secondsRemaining);

// Display volume value on TV OSD (called during volume adjustment)
void OSD_updateVolumeDisplay(uint8_t volumeValue);

// Render info display on TV OSD
void OSD_renderInfoDisplay(uint8_t isInfoDisplayActive);

