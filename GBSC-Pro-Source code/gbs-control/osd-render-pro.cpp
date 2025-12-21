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
#include "OLEDMenuImplementation-pro.h"
#include "options.h"
#include "tv5725.h"

#include "OSD_TV/OSD_stv9426.h"
#include "OSD_TV/profile_name.h"

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
static void displayProfileName()
{
    for (uint8_t i = 0; i < 9; ++i) {
        OSD_writeCharAt(profileChars[i], 0x1F + i * 2);  // _15=0x1F, _16=0x21, ...
    }
}

// Set menu line colors based on selection (replaces ~15 lines per handler)
static void OSD_setMenuLineColors(uint8_t selectedLine) {
    if (selectedLine == 1) {
        menuLine1Color = OSD_TEXT_SELECTED;
        menuLine2Color = OSD_TEXT_NORMAL;
        menuLine3Color = OSD_TEXT_NORMAL;
    } else if (selectedLine == 2) {
        menuLine1Color = OSD_TEXT_NORMAL;
        menuLine2Color = OSD_TEXT_SELECTED;
        menuLine3Color = OSD_TEXT_NORMAL;
    } else {
        menuLine1Color = OSD_TEXT_NORMAL;
        menuLine2Color = OSD_TEXT_NORMAL;
        menuLine3Color = OSD_TEXT_SELECTED;
    }
}

// Set menu line colors with custom color for line 2 (for disabled items)
static void OSD_setMenuLineColorsWithLine2(uint8_t selectedLine, uint8_t line2Color) {
    if (selectedLine == 1) {
        menuLine1Color = OSD_TEXT_SELECTED;
        menuLine2Color = line2Color;
        menuLine3Color = OSD_TEXT_NORMAL;
    } else if (selectedLine == 2) {
        menuLine1Color = OSD_TEXT_NORMAL;
        menuLine2Color = (line2Color == OSD_TEXT_DISABLED) ? OSD_TEXT_DISABLED : OSD_TEXT_SELECTED;
        menuLine3Color = OSD_TEXT_NORMAL;
    } else {
        menuLine1Color = OSD_TEXT_NORMAL;
        menuLine2Color = line2Color;
        menuLine3Color = OSD_TEXT_SELECTED;
    }
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

    // OSD_writeCharRow2(0x15, P9 , OSD_BACKGROUND);
    // OSD_writeCharRow3(0x15, P18, OSD_BACKGROUND);

    OSD_fillBackground();
    currentColor = OSD_CURSOR_INACTIVE;
    currentRow = ROW_2;
    OSD_writeCharAt(icon4, _0);
    currentRow = ROW_3;
    OSD_writeCharAt(icon4, _0);
    currentColor = OSD_CURSOR_ACTIVE;
    currentRow = ROW_1;
    OSD_writeCharAt(icon4, _0);

    currentColor = OSD_ICON_PAGE;

    // currentRow = ROW_1;
    // OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('1', _27);
    currentRow = ROW_3;
    OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "1 Input");
    OSD_writeCharRow1(0x15, P8, OSD_TEXT_SELECTED);

    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "2 Output Resolution");

    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "3 Screen Settings");
};
void handle_MainMenu_Page1_Update(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writeCharRow1(0x15, P8, (selectedMenuLine == 1) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeCharRow2(0x15, P20, (selectedMenuLine == 2) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeCharRow3(0x15, P18, (selectedMenuLine == 3) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);

    currentColor = OSD_ICON_PAGE;
    // currentRow = ROW_1;
    // OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('1', _27);
    currentRow = ROW_3;
    OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "1 Input");

    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "2 Output Resolution"); //__(0X15, _9);

    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "3 Screen Settings"); //__(0X15, _18);
};
void handle_MainMenu_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writeCharRow1(0x15, P18, (selectedMenuLine == 1) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeCharRow2(0x15, P19, (selectedMenuLine == 2) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);

    currentColor = OSD_ICON_PAGE;
    currentRow = ROW_1;
    OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('2', _27);
    // currentRow = ROW_3;
    // OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "4 System Settings");
    currentColor = menuLine2Color;
    currentRow = ROW_2;
    // OSD_writeString(1, "5 Color Settings");
    OSD_writeString(1, "5 Picture Settings");
    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "6 Reset Settings");
};
void handle_OutputRes_1080_1024_960(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;
    // currentRow = ROW_1;
    // OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('1', _27);
    currentRow = ROW_3;
    OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "1920x1080");
    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "1280x1024");
    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "1280x960");
};
void handle_OutputRes_720_480(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;

    currentRow = ROW_1;
    OSD_writeCharAt(icon5, _27);

    currentRow = ROW_2;
    OSD_writeCharAt('2', _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "1280x720");

    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "480p/576p");
    // currentColor = menuLine3Color;
    // currentRow = ROW_3;
    // OSD_writeCharAt(D, _1), __(o, _2), __(w, _3), __(n, _4), __(s, _5), __(c, _6), __(a, _7), __(l, _8), __(e, _9), __(n1, _11), __(n5, _12), __(K, _13), __(H, _14), __(z, _15);
};
void handle_OutputRes_PassThrough(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;
    currentRow = ROW_1;
    OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt(I, _27);
    currentRow = ROW_3;
    OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "Pass through");
};
void handle_ScreenSettings(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;

    // currentRow = ROW_1;
    // OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('1', _27);
    currentRow = ROW_3;
    OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "Move");
    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "Scale");
    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "Borders");
};
void handle_HighlightRow1(void)
{
    OSD_fillBackground();
    OSD_writeCharRow1(icon4, P0, OSD_CURSOR_ACTIVE);
    OSD_writeCharRow2(icon4, P0, OSD_BACKGROUND);
    OSD_writeCharRow3(icon4, P0, OSD_BACKGROUND);
    selectedMenuLine = 1;
};
void handle_HighlightRow2(void)
{
    OSD_fillBackground();
    OSD_writeCharRow1(icon4, P0, OSD_BACKGROUND);
    OSD_writeCharRow2(icon4, P0, OSD_CURSOR_ACTIVE);
    OSD_writeCharRow3(icon4, P0, OSD_BACKGROUND);
    selectedMenuLine = 2;
};
void handle_HighlightRow3(void)
{
    OSD_fillBackground();
    OSD_writeCharRow1(icon4, P0, OSD_BACKGROUND);
    OSD_writeCharRow2(icon4, P0, OSD_BACKGROUND);
    OSD_writeCharRow3(icon4, P0, OSD_CURSOR_ACTIVE);
    selectedMenuLine = 3;
};
void handle_ColorSettings_Page1(void)
{
    // Line 2 (Scanlines) disabled when not allowed
    uint8_t line2Color = areScanLinesAllowed() ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED;
    OSD_setMenuLineColorsWithLine2(selectedMenuLine, line2Color);

    currentColor = OSD_ICON_PAGE;

    currentRow = ROW_1;
    OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('2', _27);
    currentRow = ROW_3;
    OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "ADC gain");
    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "Scanlines");
    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "Line filter");
};
void handle_ColorSettings_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;
    currentRow = ROW_1;
    OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('3', _27);
    currentRow = ROW_3;
    OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "Sharpness");
    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "Peaking");
    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "Step response");
};
void handle_ColorSettings_Page3(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;
    currentRow = ROW_1;
    OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('4', _27);
    // currentRow = ROW_3;
    // OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "Default Color");
    // OSD_writeString(1, "Y gain");
    // currentColor = menuLine2Color;
    // currentRow = ROW_2;
    // OSD_writeString(1, "Color");
    // currentColor = menuLine3Color;
    // currentRow = ROW_3;
};
void handle_ColorSettings_RGB_Labels(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;
    // currentRow = ROW_1;
    // OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('1', _27);
    currentRow = ROW_3;
    OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "R");
    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "G");
    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "B");
};
void handle_ColorSettings_Page1_Values(void)
{
    OSD_drawDashRange(1, 9, 18);   // Row 1: P9-P18
    OSD_writeCharRow1(0x3E, P22, OSD_TEXT_NORMAL);

    // Only show dashes for scanlines if they are available
    if (areScanLinesAllowed()) {
        OSD_drawDashRange(2, 10, 19);  // Row 2: P10-P19
        OSD_writeCharRow2(0x3E, P22, OSD_TEXT_NORMAL);

        OSD_writeOnOff(2, uopt->wantScanlines);

        // Display scanline strength (0x00-0x50 → 00-05)
        osdDisplayValue = uopt->scanlineStrength;
        OSD_writeCharRow2(n0, P21, OSD_TEXT_NORMAL);
        OSD_writeCharRow2(n0 + (osdDisplayValue >> 4), P20, OSD_TEXT_NORMAL);
    }

    OSD_drawDashRange(3, 12, 22);  // Row 3: P12-P22
    OSD_writeOnOff(3, uopt->wantVdsLineFilter);
    osdDisplayValue = GBS::ADC_RGCTRL::read();
    OSD_displayNumber3DigitInverted(osdDisplayValue);

    OSD_writeOnOff(1, uopt->enableAutoGain != 0);
};
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
        OSD_writeStringAtLine(2, 20, "LOCKED");
    } else {
        OSD_writeOnOff(2, uopt->wantPeaking != 0);
    }

    OSD_writeOnOff(3, uopt->wantStepResponse);
};
void handle_ColorSettings_RGB_Values(void)
{
    OSD_drawDashRange(1, 5, 22);  // Row 1: P5-P22
    OSD_drawDashRange(2, 5, 22);  // Row 2: P5-P22
    OSD_drawDashRange(3, 5, 22);  // Row 3: P5-P22

    // osdDisplayValue = (128 + GBS::VDS_Y_OFST::read());  //R
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() + 1.402 * ((signed char)GBS::VDS_V_OFST::read()-128));  //R
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() + 1.5 * ((signed char)GBS::VDS_V_OFST::read()));  //R
    // osdDisplayValue= (signed char)GBS::VDS_Y_OFST::read()+1.402*((signed char)GBS::VDS_V_OFST::read()-128);
    // osdDisplayValue = R_VAL;
    currentColor = OSD_TEXT_NORMAL;
    currentRow = ROW_1;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    // OSD_displayNumber3Digit(((signed char)((signed char)GBS::VDS_Y_OFST::read()) +(float)( 1.402     * (signed char)((signed char)GBS::VDS_V_OFST::read()) )) + 128);
    OSD_displayNumber3Digit(R_VAL);
    // osdDisplayValue = (128 + GBS::VDS_U_OFST::read());  //G
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() - 0.88 * ((signed char)GBS::VDS_U_OFST::read()) - 0.764 * ((signed char)GBS::VDS_V_OFST::read()));  //G
    // osdDisplayValue = (signed char)GBS::VDS_Y_OFST::read()-0.344136*((signed char)GBS::VDS_U_OFST::read()-128)-0.714136*((signed char)GBS::VDS_V_OFST::read()-128);
    // osdDisplayValue = G_VAL;
    currentColor = OSD_TEXT_NORMAL;
    currentRow = ROW_2;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    // OSD_displayNumber3Digit(((signed char)((signed char)GBS::VDS_Y_OFST::read()) -(float)( 0.344136  * (signed char)((signed char)GBS::VDS_U_OFST::read()) )- 0.714136 * (signed char)((signed char)GBS::VDS_V_OFST::read()) ) + 128);
    OSD_displayNumber3Digit(G_VAL);

    // osdDisplayValue = (128 + GBS::VDS_V_OFST::read());  //B
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() + 2 * ((signed char)GBS::VDS_U_OFST::read()));  //B
    // osdDisplayValue = (signed char)GBS::VDS_Y_OFST::read()+1.772*((signed char)GBS::VDS_U_OFST::read()-128);
    // osdDisplayValue = B_VAL;
    currentColor = OSD_TEXT_NORMAL;
    currentRow = ROW_3;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    // OSD_displayNumber3Digit(((signed char)((signed char)GBS::VDS_Y_OFST::read()) +(float)( 1.772     * (signed char)((signed char)GBS::VDS_U_OFST::read()) )) + 128);
    OSD_displayNumber3Digit(B_VAL);
};
void handle_SysSettings_SVInput_Values(void)
{
    OSD_drawDashRange(1, 7, 22);  // Row 1: P7-P22
    OSD_drawDashRange(2, 6, 22);  // Row 2: P6-P22

    osdDisplayValue = GBS::VDS_Y_GAIN::read();
    currentColor = OSD_TEXT_NORMAL;
    currentRow = ROW_1;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    OSD_displayNumber3Digit(osdDisplayValue);
    osdDisplayValue = GBS::VDS_VCOS_GAIN::read();
    currentColor = OSD_TEXT_NORMAL;
    currentRow = ROW_2;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    OSD_displayNumber3Digit(osdDisplayValue);
};
void handle_SysSettings_Page1(void)
{
    // Line 1 (SV/AV Input Settings) disabled when not SV/AV input
    bool isSvAvInput = (inputType == InputTypeSV) || (inputType == InputTypeAV);
    OSD_setMenuLineColors(selectedMenuLine);
    if (!isSvAvInput) menuLine1Color = OSD_TEXT_DISABLED;  // Override line 1 color if disabled
    OSD_writeCharRow1(0x15, P21, (selectedMenuLine == 1) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);

    currentColor = OSD_ICON_PAGE;
    // currentRow = ROW_1;
    // OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('1', _27);
    currentRow = ROW_3;
    OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "SV/AV Input Settings");

    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "Compatibility Mode");

    currentColor = menuLine3Color;
    currentRow = ROW_3;
    // OSD_writeString(1, "Lowres:use upscaling");
    OSD_writeString(1, "Matched presets");
};
void handle_SysSettings_Page1_Values(void)
{
    // OSD_writeCharRow2(0x3E, P16, OSD_TEXT_NORMAL);
    // OSD_writeCharRow2(0x3E, P17, OSD_TEXT_NORMAL);
    // OSD_writeCharRow2(0x3E, P18, OSD_TEXT_NORMAL);
    OSD_drawDashRange(2, 19, 22);  // Row 2: P19-P22
    OSD_writeOnOff(2, rgbComponentMode == 1);

    OSD_drawDashRange(3, 16, 22);  // Row 3: P16-P22
    OSD_writeOnOff(3, uopt->matchPresetSource);
    /*
    upscaling
        // OSD_writeCharRow3(0x3E, P21, OSD_TEXT_NORMAL);
        // OSD_writeCharRow3(0x3E, P22, OSD_TEXT_NORMAL);
        // if (uopt->preferScalingRgbhv)
        // {
        //   OSD_writeCharRow3(O, P23, OSD_TEXT_NORMAL);
        //   OSD_writeCharRow3(N, P24, OSD_TEXT_NORMAL);
        //   OSD_writeCharRow3(F, P25, OSD_BACKGROUND);
        // }
        // else
        // {
        //   OSD_writeCharRow3(O, P23, OSD_TEXT_NORMAL);
        //   OSD_writeCharRow3(F, P24, OSD_TEXT_NORMAL);
        //   OSD_writeCharRow3(F, P25, OSD_TEXT_NORMAL);
        // }
    */
};
void handle_SysSettings_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;
    currentRow = ROW_1;
    OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('2', _27);
    currentRow = ROW_3;
    OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "Deinterlace");
    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "Force:50Hz to 60Hz");
    currentColor = menuLine3Color;
    currentRow = ROW_3;
    // OSD_writeString(1, "Clock generator");
    OSD_writeString(1, "Lock method");
};
void handle_SysSettings_Page2_Values(void)
{
    OSD_drawDashRange(1, 12, 17);  // Row 1: P12-P17
    if (uopt->deintMode == 0) {
        OSD_writeStringAtLine(1, 18, "Adaptive");
    } else {
        OSD_drawDashRange(1, 18, 22);  // Row 1: P18-P22
        OSD_writeStringAtLine(1, 23, "Bob");
    }

    OSD_drawDashRange(2, 19, 22);  // Row 2: P19-P22
    OSD_drawDashRange(3, 12, 13);  // Row 3: P12-P13

    // if (uopt->wantOutputComponent)
    // {
    //     OSD_writeCharRow1(O, P23, OSD_TEXT_NORMAL);
    //     OSD_writeCharRow1(N, P24, OSD_TEXT_NORMAL);
    //     OSD_writeCharRow1(F, P25, OSD_BACKGROUND);
    // }
    // else
    // {
    //     OSD_writeCharRow1(O, P23, OSD_TEXT_NORMAL);
    //     OSD_writeCharRow1(F, P24, OSD_TEXT_NORMAL);
    //     OSD_writeCharRow1(F, P25, OSD_TEXT_NORMAL);
    // }

    OSD_writeOnOff(2, uopt->PalForce60);

    // if (uopt->disableExternalClockGenerator)
    // {
    //   OSD_writeCharRow3(O, P23, OSD_TEXT_NORMAL);
    //   OSD_writeCharRow3(F, P24, OSD_TEXT_NORMAL);
    //   OSD_writeCharRow3(F, P25, OSD_TEXT_NORMAL);
    // }
    // else
    // {
    //   OSD_writeCharRow3(O, P23, OSD_TEXT_NORMAL);
    //   OSD_writeCharRow3(N, P24, OSD_TEXT_NORMAL);
    //   OSD_writeCharRow3(F, P25, OSD_BACKGROUND);
    // }

    if (uopt->frameTimeLockMethod == 0) {
        OSD_writeStringAtLine(3, 14, "0Vtotal<VSST");
    } else {
        OSD_writeStringAtLine(3, 14, "1Vtotal only");
    }
};
void handle_SysSettings_Page4(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;
    currentRow = ROW_1;
    OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('3', _27);
    // currentRow = ROW_3;
    // OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "ADC calibration");
    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "Frame Time lock");
    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "EnableFrameTimeLock");
};
void handle_SysSettings_Page4_Values(void)
{
    OSD_drawDashRange(1, 16, 22);  // Row 1: P16-P22
    OSD_drawDashRange(2, 16, 22);  // Row 2: P16-P22
    OSD_drawDashRange(3, 20, 22);  // Row 3: P20-P22

    OSD_writeOnOff(1, uopt->enableCalibrationADC);
    OSD_writeOnOff(2, uopt->enableFrameTimeLock);
    OSD_writeOnOff(3, !uopt->disableExternalClockGenerator);
};
void handle_ScreenSettings_FullHeight(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;
    currentRow = ROW_1;
    OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('2', _27);
    // currentRow = ROW_3;
    // OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "Full height");

    // currentColor = menuLine2Color;
    // currentRow = ROW_2;
    // OSD_writeCharAt(M, _1), __(a, _2), __(t, _3), __(c, _4), __(h, _5), __(e, _6), __(d, _7), __(p, _9), __(r, _10), __(e, _11), __(s, _12), __(e, _13), __(t, _14), __(s, _15);
};
void handle_ScreenFullHeight_Values(void)
{
    OSD_drawDashRange(1, 12, 22);  // Row 1: P12-P22
    OSD_writeOnOff(1, uopt->wantFullHeight);
};
void handle_Developer_Memory(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;
    currentRow = ROW_1;
    OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt(I, _27);
    currentRow = ROW_3;
    OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "MEM left/right");
    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "HS left/right");
    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "HTotal");
};
void handle_Developer_Memory_Values(void)
{
    OSD_drawDashRange(1, 15, 22);  // Row 1: P15-P22
    OSD_writeCharRow1(0x03, P23, OSD_CURSOR_ACTIVE);
    OSD_writeCharRow1(0x13, P24, OSD_CURSOR_ACTIVE);
    OSD_drawDashRange(2, 14, 22);  // Row 2: P14-P22
    OSD_writeCharRow2(0x03, P23, OSD_CURSOR_ACTIVE);
    OSD_writeCharRow2(0x13, P24, OSD_CURSOR_ACTIVE);
    OSD_drawDashRange(3, 7, 22);   // Row 3: P7-P22
    osdDisplayValue = GBS::VDS_HSYNC_RST::read();
    currentColor = OSD_TEXT_NORMAL;
    currentRow = ROW_3;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    OSD_displayNumber3Digit(osdDisplayValue);
};
void handle_Developer_Debug(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;
    currentRow = ROW_1;
    OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt(I, _27);
    currentRow = ROW_3;
    OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "Debug view");
    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "ADC filter");
    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "Freeze capture");
};
void handle_Developer_Debug_Values(void)
{
    OSD_drawDashRange(1, 11, 22);  // Row 1: P11-P22
    OSD_drawDashRange(2, 11, 22);  // Row 2: P11-P22
    currentColor = OSD_TEXT_NORMAL;
    currentRow = ROW_3;
    OSD_writeCharAt(0x3E, _15), OSD_writeCharAt(0x3E, _16), OSD_writeCharAt(0x3E, _17), OSD_writeCharAt(0x3E, _18), OSD_writeCharAt(0x3E, _19), OSD_writeCharAt(0x3E, _20), OSD_writeCharAt(0x3E, _21), OSD_writeCharAt(0x3E, _22);

    OSD_writeOnOff(1, GBS::ADC_UNUSED_62::read() != 0x00);
    OSD_writeOnOff(2, GBS::ADC_FLTR::read() > 0);
    OSD_writeOnOff(3, GBS::CAPTURE_ENABLE::read() == 0);  // Inverted: ON when frozen (capture disabled)
};
void handle_SysSettings_Page5(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;
    currentRow = ROW_1;
    OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt(I, _27);
    currentRow = ROW_3;
    OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "Enable OTA");
    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "Restart");
    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "Reset defaults");
};
void handle_SysSettings_Page5_Values(void)
{
    OSD_drawDashRange(1, 11, 22);  // Row 1: P11-P22
    OSD_writeOnOff(1, rto->allowUpdatesOTA);
};
void handle_Profile_SaveLoad(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "Loadprofile:");

    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "Saveprofile:");

    currentColor = OSD_TEXT_SELECTED;
    currentRow = ROW_3;
    OSD_writeString(1, "Active save:");
};
// Helper: call name_N() by index (1-20)
static void callProfileName(uint8_t index) {
    switch (index) {
        case 1:  name_1();  break; case 2:  name_2();  break; case 3:  name_3();  break;
        case 4:  name_4();  break; case 5:  name_5();  break; case 6:  name_6();  break;
        case 7:  name_7();  break; case 8:  name_8();  break; case 9:  name_9();  break;
        case 10: name_10(); break; case 11: name_11(); break; case 12: name_12(); break;
        case 13: name_13(); break; case 14: name_14(); break; case 15: name_15(); break;
        case 16: name_16(); break; case 17: name_17(); break; case 18: name_18(); break;
        case 19: name_19(); break; case 20: name_20(); break;
    }
}

void handle_Profile_SlotDisplay(void)
{
    // Row 1: Load profile display (Load1-Load20 → index 1-20)
    int loadIdx = oled_menuItem - OLED_Profile_Load1;
    if (loadIdx >= 0 && loadIdx < 20) {
        currentColor = OSD_TEXT_NORMAL;
        currentRow = ROW_1;
        callProfileName(loadIdx + 1);
        displayProfileName();
    }

    // Row 3: Active save slot (presetSlot 'A'-'T' → index 1-20)
    if (uopt->presetSlot >= 'A' && uopt->presetSlot <= 'T') {
        currentColor = OSD_TEXT_SELECTED;
        currentRow = ROW_3;
        callProfileName(uopt->presetSlot - 'A' + 1);
        displayProfileName();
    }

    // Row 2: Save profile display (Save1-Save20 → index 1-20)
    int saveIdx = oled_menuItem - OLED_Profile_Save1;
    if (saveIdx >= 0 && saveIdx < 20) {
        currentColor = OSD_TEXT_NORMAL;
        currentRow = ROW_2;
        callProfileName(saveIdx + 1);
        displayProfileName();
    }
};
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
};

void handle_Profile_SlotRow2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "Contrast");
    // OSD_writeString(1, "Saturation");

    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "Saturation");

    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "Default");
}

void handle_Profile_SlotRow3(void)
{
    OSD_drawDashRange(1, 13, 18);  // Row 1: P13-P18

    currentColor = OSD_TEXT_NORMAL;
    currentRow = ROW_1;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    OSD_displayNumber3Digit(contrast);

    OSD_drawDashRange(2, 13, 18);  // Row 2: P13-P18


    currentColor = OSD_TEXT_NORMAL;
    currentRow = ROW_2;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    OSD_displayNumber3Digit(saturation);
};


void handle_ADCCalib_Running(void)
{
    // Line 2 (Smooth) disabled when lineOption is false
    uint8_t line2Color = lineOption ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED;
    OSD_setMenuLineColorsWithLine2(selectedMenuLine, line2Color);

    // currentColor = OSD_ICON_PAGE;
    // currentRow = ROW_1;
    // OSD_writeCharAt(icon5, _27);
    // currentRow = ROW_2;
    // OSD_writeCharAt(I, _27);
    // currentRow = ROW_3;
    // OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "DoubleLine");
    // OSD_writeString(1, "Smooth");
    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "Smooth");
    // OSD_writeString(1, "Bright");

    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "Bright");
    // OSD_writeString(1, "Contrast");
};
void handle_InputMenu_Page1(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;
    // currentRow = ROW_1;
    // OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('1', _27);
    currentRow = ROW_3;
    OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "RGBs");
    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "RGsB");
    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "VGA");
};
void handle_InputInfo(void)
{
    menuLine1Color = OSD_TEXT_NORMAL;
    menuLine2Color = OSD_TEXT_NORMAL;
    menuLine3Color = OSD_TEXT_NORMAL;

    currentColor = OSD_ICON_PAGE;
    // currentRow = ROW_1;
    // OSD_writeCharAt(icon5, _27);
    // currentRow = ROW_2;
    // OSD_writeCharAt('1', _27);
    // currentRow = ROW_3;
    // OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(0, "Whether to keep the settings");

    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(0, "Restore in ");

    if (keepSettings) {
        currentColor = OSD_TEXT_SELECTED;
        OSD_writeCharRow3(0x15, P2, OSD_TEXT_SELECTED);
    } else {
        currentColor = menuLine3Color;
        OSD_writeCharRow3(0x15, P2, OSD_BACKGROUND);
    }
    currentRow = ROW_3;
    OSD_writeString(3, "Changes");

    if (!keepSettings) {
        currentColor = OSD_TEXT_SELECTED;
        OSD_writeCharRow3(0x15, P13, OSD_TEXT_SELECTED);
    } else {
        currentColor = menuLine3Color;
        OSD_writeCharRow3(0x15, P13, OSD_BACKGROUND);
    }
    OSD_writeString(0xff, "    Recover");

    // OSD_writeCharRow3(0x15, P2, OSD_BACKGROUND);
};
void handle_InputMenu_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;
    currentRow = ROW_1;
    OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('2', _27);
    // currentRow = ROW_3;
    // OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "YPBPR");
    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "SV");
    currentColor = menuLine3Color;
    currentRow = ROW_3;
    OSD_writeString(1, "AV");
};

void handle_InfoDisplay(void)
{
    bool isSV = (oled_menuItem == OLED_Input_SV);
    bool isAV = (oled_menuItem == OLED_Input_AV);

    // Clear or show "Format:" label
    if (isSV) {
        OSD_writeStringAtLine(2, 4, "Format:");
        OSD_writeStringAtLine(3, 4, "                      ");
    } else if (isAV) {
        OSD_writeStringAtLine(3, 4, "Format:");
        OSD_writeStringAtLine(2, 4, "                      ");
    } else {
        OSD_writeStringAtLine(2, 4, "                      ");
        OSD_writeStringAtLine(3, 4, "                      ");
    }

    // Display format name for SV or AV
    // Clear area first with spaces in the string write (OSD_writeStringAtLine handles spaces)
    if (isSV) {
        // Pad format name to fixed width to overwrite previous text
        char padded[16];
        snprintf(padded, sizeof(padded), "%-15s", getVideoFormatName(SVModeOption));
        OSD_writeStringAtLine(2, 11, padded);
    } else if (isAV) {
        char padded[16];
        snprintf(padded, sizeof(padded), "%-15s", getVideoFormatName(AVModeOption));
        OSD_writeStringAtLine(3, 11, padded);
    }
};
void handle_InfoDisplay_Source(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;
    currentRow = ROW_1;
    OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt(I, _27);
    currentRow = ROW_3;
    OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "Setting");
};
void handle_ADCCalib_Display(void)
{
    OSD_drawDashRange(1, 13, 18);  // Row 1: P13-P18
    OSD_drawDashRange(2, 13, 18);  // Row 2: P13-P18
    OSD_drawDashRange(3, 13, 18);  // Row 3: P13-P18

    if (lineOption) {
        OSD_writeStringAtLine(1, 23, "2X");
    } else {
        OSD_writeStringAtLine(1, 23, "1X");
        smoothOption = false;
    }
    OSD_writeOnOff(2, smoothOption);

    currentColor = OSD_TEXT_NORMAL;
    currentRow = ROW_3;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    OSD_displayNumber3Digit(brightness);


    // currentColor = OSD_TEXT_NORMAL;
    // currentRow = ROW_3;
    // digitPos1 = _25;
    // digitPos2 = _24;
    // digitPos3 = _23;
    // OSD_displayNumber3Digit(contrast);
};
void handle_Restart(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = OSD_ICON_PAGE;
    currentRow = ROW_1;
    OSD_writeCharAt(icon5, _27);
    currentRow = ROW_2;
    OSD_writeCharAt('4', _27);
    // currentRow = ROW_3;
    // OSD_writeCharAt(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "Matched presets");
    // currentColor = menuLine2Color;
    // currentRow = ROW_2;
};
