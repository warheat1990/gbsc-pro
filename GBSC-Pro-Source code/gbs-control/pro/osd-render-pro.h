// ====================================================================================
// osd-render-pro.h
// TV OSD Menu Rendering and Handlers
//
// This file contains declarations for:
// - menuTable[]: Command dispatch table
// - OSD_handleCommand(): Menu command dispatcher
// - handle_X(): Individual menu screen rendering functions
//
// Note: MenuEntry type is defined in gbs-control-pro.h
// ====================================================================================

#pragma once

#include <Arduino.h>

// ====================================================================================
// External Variables - Menu Table
// ====================================================================================

extern const MenuEntry menuTable[];
extern const size_t menuTableSize;

// ====================================================================================
// Function Declarations - Menu Dispatcher
// ====================================================================================

void OSD_handleCommand(OsdCommand cmd);

// ====================================================================================
// Function Declarations - Menu Handlers - Main Menu
// ====================================================================================

void handle_MainMenu_Page1(void);
void handle_MainMenu_Page1_Update(void);
void handle_MainMenu_Page2(void);

// ====================================================================================
// Function Declarations - Menu Handlers - Output Resolution
// ====================================================================================

void handle_OutputRes_1080_1024_960(void);
void handle_OutputRes_720_480(void);
void handle_OutputRes_PassThrough(void);

// ====================================================================================
// Function Declarations - Menu Handlers - Screen Settings
// ====================================================================================

void handle_ScreenSettings(void);
void handle_HighlightRow1(void);
void handle_HighlightRow2(void);
void handle_HighlightRow3(void);
void handle_ScreenSettings_FullHeight(void);

// ====================================================================================
// Function Declarations - Menu Handlers - Color/Picture Settings
// ====================================================================================

void handle_ColorSettings_Page1(void);
void handle_ColorSettings_Page1_Values(void);
void handle_ColorSettings_Page2(void);
void handle_ColorSettings_Page2_Values(void);
void handle_ColorSettings_Page3(void);
void handle_ColorSettings_RGB_Labels(void);
void handle_ColorSettings_RGB_Values(void);

// ====================================================================================
// Function Declarations - Menu Handlers - System Settings
// ====================================================================================

void handle_SysSettings_SVInput_Values(void);
void handle_SysSettings_Page1(void);
void handle_SysSettings_Page1_Values(void);
void handle_SysSettings_Page2(void);
void handle_SysSettings_Page2_Values(void);
void handle_SysSettings_Page4(void);
void handle_SysSettings_Page4_Values(void);
void handle_SysSettings_Page5(void);
void handle_SysSettings_Page5_Values(void);
void handle_ScreenFullHeight_Values(void);

// ====================================================================================
// Function Declarations - Menu Handlers - Developer
// ====================================================================================

void handle_Developer_Memory(void);
void handle_Developer_Memory_Values(void);
void handle_Developer_Debug(void);
void handle_Developer_Debug_Values(void);

// ====================================================================================
// Function Declarations - Menu Handlers - Profile
// ====================================================================================

void handle_Profile_SaveLoad(void);
void handle_Profile_SlotDisplay(void);
void handle_Profile_SlotRow1(void);
void handle_Profile_SlotRow2(void);
void handle_Profile_SlotRow3(void);

// ====================================================================================
// Function Declarations - Menu Handlers - Input
// ====================================================================================

void handle_InputMenu_Page1(void);
void handle_InputMenu_Page2(void);
void handle_InputInfo(void);

// ====================================================================================
// Function Declarations - Menu Handlers - Info/Status
// ====================================================================================

void handle_InfoDisplay(void);
void handle_InfoDisplay_Source(void);
void handle_ADCCalib_Running(void);
void handle_ADCCalib_Display(void);
void handle_Restart(void);

// ====================================================================================
// Function Declarations - TV OSD Feedback
// ====================================================================================

void OSD_showLimitFeedback(uint8_t row, int iterations = 400);
void OSD_showOkFeedback(uint8_t row, int iterations = 800);
void OSD_showSavingFeedback(uint8_t row, uint8_t startPos = 19, int iterations = 800);
void OSD_showAdjustArrows(uint8_t row, uint8_t dashStart = 8);
void OSD_highlightIcon(uint8_t pos);
