// ====================================================================================
// osd-common.h
// Shared definitions and helper functions for TV OSD rendering
// ====================================================================================

#pragma once

#include <Arduino.h>

#include "../gbs-control-pro.h"
#include "../options-pro.h"
#include "../../options.h"
#include "../../tv5725.h"

#include "../drivers/stv9426.h"

// ====================================================================================
// External References
// ====================================================================================

typedef TV5725<GBS_ADDR> GBS;
extern struct runTimeOptions *rto;
extern struct userOptions *uopt;

extern boolean areScanLinesAllowed();
extern void saveUserPrefs();
extern void applyPresets(uint8_t videoMode);

// ====================================================================================
// Helper Function Declarations
// ====================================================================================

// Display 9-character profile name at positions P15-P23
void displayProfileName(uint8_t row, uint8_t color);

// Get color for menu row (1-based)
uint8_t OSD_getMenuLineColor(uint8_t row);

// Set menu line colors based on selection
void OSD_setMenuLineColors(uint8_t selectedLine);

// Set menu line colors with custom color for a specific line
void OSD_setMenuLineColorsCustom(uint8_t selectedLine, uint8_t customRow, uint8_t customColor);

// Write page navigation icons at position P27
void OSD_writePageIcons(bool showUp, uint8_t pageChar, bool showDown);

// Unified row highlight function
void highlightRow(uint8_t row);

// Generate profile name into profileChars[]
void setProfileName(uint8_t index);

// Draw dashes on a row from startPos to endPos (logical positions 0-27)
void OSD_drawDashRange(uint8_t row, uint8_t startPos, uint8_t endPos);

// Write ON or OFF indicator at end of row (positions 23-25)
void OSD_writeOnOff(uint8_t row, bool isOn);

// ====================================================================================
// Handler Function Declarations - Initialization
// ====================================================================================

void handle_OSD_Init(void);

// ====================================================================================
// Handler Function Declarations - Main Menu
// ====================================================================================

void handle_MainMenu_Page1(void);
void handle_MainMenu_Page2(void);
void handle_HighlightRow1(void);
void handle_HighlightRow2(void);
void handle_HighlightRow3(void);

// ====================================================================================
// Handler Function Declarations - Output Resolution
// ====================================================================================

void handle_OutputRes_1080_1024_960(void);
void handle_OutputRes_720_480(void);
void handle_OutputRes_PassThrough(void);

// ====================================================================================
// Handler Function Declarations - Screen Settings
// ====================================================================================

void handle_ScreenSettings(void);
void handle_ScreenSettings_FullHeight(void);
void handle_ScreenFullHeight_Values(void);

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
void handle_SysSettings_Page5(void);
void handle_SysSettings_Page5_Values(void);
void handle_Developer_Memory(void);
void handle_Developer_Memory_Values(void);
void handle_Developer_Debug(void);
void handle_Developer_Debug_Values(void);
void handle_Restart(void);
void handle_SVAVInput_Page1(void);
void handle_SVAVInput_Page1_Values(void);
void handle_SVAVInput_Page2(void);
void handle_SVAVInput_Page2_Values(void);

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
void handle_InputInfo(void);
void handle_InfoDisplay(void);
void handle_InfoDisplay_Source(void);

// ====================================================================================
// OSD Feedback Functions and Dispatcher
// (Declared in osd-render-pro.h, included via gbs-control-pro.h)
// ====================================================================================
