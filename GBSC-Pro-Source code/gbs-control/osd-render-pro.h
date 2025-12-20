// ====================================================================================
// osd-render-pro.h
// TV OSD Menu Rendering and Handlers
//
// This file contains declarations for:
// - OSD_writeChar(), OSD_writeString(), OSD_writeStringAtLine(): TV OSD display helpers
// - menuTable[]: Command dispatch table
// - OSD_handleCommand(): Menu command dispatcher
// - handle_X(): Individual menu screen rendering functions
//
// Note: MenuEntry type is defined in gbs-control-pro.h
// ====================================================================================

#pragma once

#include <Arduino.h>

// ====================================================================================
// Function Declarations - TV OSD Display Helpers
// ====================================================================================

void OSD_writeChar(const int T, const char C);
void OSD_writeString(uint8_t start, const char str[]);
void OSD_writeStringAtLine(int startPos, int row, const char *str);

// ====================================================================================
// External Variables - Menu Table
// ====================================================================================

extern const MenuEntry menuTable[];
extern const size_t menuTableSize;

// ====================================================================================
// Function Declarations - Menu Dispatcher
// ====================================================================================

void OSD_handleCommand(char incomingByte);

// ====================================================================================
// Function Declarations - Menu Handlers - Main Menu
// ====================================================================================

void handle_MainMenu_Page1(void);         // '0' - Main menu page 1 (Input/Output/Screen)
void handle_MainMenu_Page1_Update(void);  // '1' - Main menu page 1 update
void handle_MainMenu_Page2(void);         // '2' - Main menu page 2 (System/Picture/Reset)

// ====================================================================================
// Function Declarations - Menu Handlers - Output Resolution
// ====================================================================================

void handle_OutputRes_1080_1024_960(void); // '3' - 1080/1024/960
void handle_OutputRes_720_480(void);       // '4' - 720/480
void handle_OutputRes_PassThrough(void);   // '5' - Pass through

// ====================================================================================
// Function Declarations - Menu Handlers - Screen Settings
// ====================================================================================

void handle_ScreenSettings(void);          // '6' - Move/Scale/Borders
void handle_HighlightRow1(void);           // '7' - Highlight row 1
void handle_HighlightRow2(void);           // '8' - Highlight row 2
void handle_HighlightRow3(void);           // '9' - Highlight row 3
void handle_ScreenSettings_FullHeight(void); // 'o' - Full height toggle

// ====================================================================================
// Function Declarations - Menu Handlers - Color/Picture Settings
// ====================================================================================

void handle_ColorSettings_Page1(void);     // 'a' - ADC/Scanlines/LineFilter
void handle_ColorSettings_Page2(void);     // 'b' - Sharpness/Peaking/StepResponse
void handle_ColorSettings_Page3(void);     // 'c' - DefaultColor
void handle_ColorSettings_RGB_R(void);     // 'd' - RGB R value
void handle_ColorSettings_RGB_GB(void);    // 'e' - RGB G and B values
void handle_ColorSettings_Y_Gain(void);    // 'f' - Y Gain
void handle_ColorSettings_ADCGain(void);   // 'g' - ADC Gain display

// ====================================================================================
// Function Declarations - Menu Handlers - System Settings
// ====================================================================================

void handle_SysSettings_SVInput_Page1(void); // 'h' - SV/AV settings (DoubleLine/Smooth/Bright)
void handle_SysSettings_Page1(void);       // 'i' - SVAVInput/Compatibility/MatchedPresets
void handle_SysSettings_Page2(void);       // 'j' - Deinterlace/Force5060/LockMethod
void handle_SysSettings_Page3(void);       // 'k' - ADCCalib/ClockGen/FrameTimeLock
void handle_SysSettings_SVInput_Page2(void); // 'l' - Contrast/Saturation/Default

// ====================================================================================
// Function Declarations - Menu Handlers - Developer
// ====================================================================================

void handle_Developer_Memory(void);        // 'q' - Memory adjust
void handle_Developer_HSync(void);         // 'r' - HSync adjust
void handle_Developer_Debug(void);         // 's' - Debug view
void handle_Developer_Page(void);          // 't' - Debug/ADCFilter/Freeze

// ====================================================================================
// Function Declarations - Menu Handlers - Profile
// ====================================================================================

void handle_Profile_SaveLoad(void);        // 'w' - Profile save/load menu
void handle_Profile_SlotDisplay(void);     // 'x' - Profile slot display
void handle_Profile_SlotRow1(void);        // 'y' - Profile slot row 1
void handle_Profile_SlotRow2(void);        // 'z' - Profile slot row 2
void handle_Profile_SlotRow3(void);        // 'A' - Profile slot row 3

// ====================================================================================
// Function Declarations - Menu Handlers - Input
// ====================================================================================

void handle_InputMenu_Page1(void);         // '@' - RGBs/RGsB/VGA
void handle_InputMenu_Page2(void);         // '#' - YPbPr/SV/AV
void handle_InputInfo(void);               // '!' - Input info display

// ====================================================================================
// Function Declarations - Menu Handlers - Info/Status
// ====================================================================================

void handle_InfoDisplay(void);             // '$' - Resolution/Hz display
void handle_InfoDisplay_Source(void);      // '%' - Source info
void handle_ADCCalib_Running(void);        // '^' - ADC calibration in progress
void handle_ADCCalib_Display(void);        // '&' - ADC calibration display
void handle_ResetSettings(void);           // 'v' - Reset settings display
void handle_Restart(void);                 // '*' - Restart display

// ====================================================================================
// Function Declarations - Menu Handlers - Reserved
// ====================================================================================

void handle_Reserved_M(void);              // 'm' - Reserved
void handle_Reserved_N(void);              // 'n' - Reserved
void handle_Reserved_P(void);              // 'p' - Reserved
void handle_Reserved_U(void);              // 'u' - Reserved
