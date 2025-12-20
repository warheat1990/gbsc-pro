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
// Internal Variables
// ====================================================================================

static void (*osd_cx_ptr)(int, int, int) = nullptr;

// ====================================================================================
// OSD Helper Functions
// ====================================================================================

// Set menu line colors based on selection (replaces ~15 lines per handler)
static void OSD_setMenuLineColors(uint8_t selectedLine) {
    if (selectedLine == 1) {
        menuLine1Color = yellowT;
        menuLine2Color = main0;
        menuLine3Color = main0;
    } else if (selectedLine == 2) {
        menuLine1Color = main0;
        menuLine2Color = yellowT;
        menuLine3Color = main0;
    } else {
        menuLine1Color = main0;
        menuLine2Color = main0;
        menuLine3Color = yellowT;
    }
}

// Set menu line colors with custom color for line 2 (for disabled items)
static void OSD_setMenuLineColorsWithLine2(uint8_t selectedLine, uint8_t line2Color) {
    if (selectedLine == 1) {
        menuLine1Color = yellowT;
        menuLine2Color = line2Color;
        menuLine3Color = main0;
    } else if (selectedLine == 2) {
        menuLine1Color = main0;
        menuLine2Color = (line2Color == red) ? red : yellowT;
        menuLine3Color = main0;
    } else {
        menuLine1Color = main0;
        menuLine2Color = line2Color;
        menuLine3Color = yellowT;
    }
}

// Draw dashes on a row from startPos to endPos (P0=0, P1=1, etc.)
// Row: 1=OSD_writeCharRow1, 2=OSD_writeCharRow2, 3=OSD_writeCharRow3
static void OSD_drawDashRange(uint8_t row, uint8_t startPos, uint8_t endPos) {
    void (*osd_func)(int, int, int);
    if (row == 1) osd_func = OSD_writeCharRow1;
    else if (row == 2) osd_func = OSD_writeCharRow2;
    else osd_func = OSD_writeCharRow3;

    for (uint8_t p = startPos; p <= endPos; p++) {
        osd_func(0x3E, 0x01 + p * 2, main0);  // P0=0x01, P1=0x03, Pn=0x01+n*2
    }
}


// ====================================================================================
// TV OSD Display Helper Functions
// ====================================================================================

void OSD_writeChar(const int T, const char C)
{
    writeChar(T, (C * 2) + 1);
}

void OSD_writeString(uint8_t start, const char str[])
{
    static uint8_t start_last = 0;
    if (str == NULL) {
        return;
    }
    if (start == 0XFF)
        start = start_last;
    else
        start_last = start;

    for (uint8_t count = 0; str[count] != '\0'; count++) {
        start_last = count + start + 1;

        if (str[count] == ' ')
            continue;
        else if (str[count] == '=')
            OSD_writeChar(0x3D, count + start);
        else if (str[count] == '.')
            OSD_writeChar(0x2E, count + start);
        else if (str[count] == '\'')
            OSD_writeChar(0x27, count + start);
        else if (str[count] == '-')
            OSD_writeChar(0x3E, count + start);
        else if (str[count] == '/')
            OSD_writeChar(0x2F, count + start);
        else if (str[count] == ':')
            OSD_writeChar(0x3A, count + start);
        else
            OSD_writeChar(str[count], count + start);
    }
}

void OSD_writeStringAtLine(int startPos, int row, const char *str)
{
    int pos = startPos;
    while (*str != '\0') {
        if (row == 1)
            osd_cx_ptr = OSD_writeCharRow1;
        else if (row == 2)
            osd_cx_ptr = OSD_writeCharRow2;
        else if (row == 3)
            osd_cx_ptr = OSD_writeCharRow3;

        if (*str == ' ')
            osd_cx_ptr(*str, 1 + pos * 2, blue_fill);
        else if (*str == '=')
            osd_cx_ptr(0x3D, 1 + pos * 2, main0);
        else if (*str == '.')
            osd_cx_ptr(0x2E, 1 + pos * 2, main0);
        else if (*str == '\'')
            osd_cx_ptr(0x27, 1 + pos * 2, main0);
        else if (*str == '-')
            osd_cx_ptr(0x3E, 1 + pos * 2, main0);
        else if (*str == '/')
            osd_cx_ptr(0x2F, 1 + pos * 2, main0);
        else if (*str == ':')
            osd_cx_ptr(0x3A, 1 + pos * 2, main0);
        else
            osd_cx_ptr(*str, 1 + pos * 2, main0);

        pos++;
        str++;
    }
}

// ====================================================================================
// MENU TABLE
// ====================================================================================

const MenuEntry menuTable[] = {
    {'0', handle_MainMenu_Page1},
    {'1', handle_MainMenu_Page1_Update},
    {'2', handle_MainMenu_Page2},
    {'3', handle_OutputRes_1080_1024_960},
    {'4', handle_OutputRes_720_480},
    {'5', handle_OutputRes_PassThrough},
    {'6', handle_ScreenSettings},
    {'7', handle_HighlightRow1},
    {'8', handle_HighlightRow2},
    {'9', handle_HighlightRow3},
    {'a', handle_ColorSettings_Page1},
    {'b', handle_ColorSettings_Page2},
    {'c', handle_ColorSettings_Page3},
    {'d', handle_ColorSettings_RGB_R},
    {'e', handle_ColorSettings_RGB_GB},
    {'f', handle_ColorSettings_Y_Gain},
    {'g', handle_ColorSettings_ADCGain},
    {'h', handle_SysSettings_SVInput_Page1},
    {'i', handle_SysSettings_Page1},
    {'j', handle_SysSettings_Page2},
    {'k', handle_SysSettings_Page3},
    {'l', handle_SysSettings_SVInput_Page2},
    {'m', handle_Reserved_M},
    {'n', handle_Reserved_N},
    {'o', handle_ScreenSettings_FullHeight},
    {'p', handle_Reserved_P},
    {'q', handle_Developer_Memory},
    {'r', handle_Developer_HSync},
    {'s', handle_Developer_Debug},
    {'t', handle_Developer_Page},
    {'u', handle_Reserved_U},
    {'v', handle_ResetSettings},
    {'w', handle_Profile_SaveLoad},
    {'x', handle_Profile_SlotDisplay},
    {'y', handle_Profile_SlotRow1},
    {'z', handle_Profile_SlotRow2},
    {'A', handle_Profile_SlotRow3},
    {'^', handle_ADCCalib_Running},
    {'@', handle_InputMenu_Page1},
    {'!', handle_InputInfo},
    {'#', handle_InputMenu_Page2},
    {'$', handle_InfoDisplay},
    {'%', handle_InfoDisplay_Source},
    {'&', handle_ADCCalib_Display},
    {'*', handle_Restart}
};

const size_t menuTableSize = sizeof(menuTable) / sizeof(menuTable[0]);

// ====================================================================================
// MENU DISPATCHER
// ====================================================================================

static bool OSD_isMainMenuCommand(char cmd) {
    static const char mainMenuCommands[] = "abcdikmowz@#^";
    return strchr(mainMenuCommands, cmd) != nullptr;
}

void OSD_handleCommand(char incomingByte)
{
    const unsigned char key = (unsigned char)incomingByte;

    for (size_t i = 0; i < menuTableSize; i++) {
        if (menuTable[i].key == key) {
            // Save only main menu commands that calculate colors, not update commands
            if (OSD_isMainMenuCommand(key)) {
                lastOsdCommand = incomingByte;
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

    // OSD_writeCharRow2(0x15, P9 , blue_fill);
    // OSD_writeCharRow3(0x15, P18, blue_fill);

    OSD_fillBackground();
    currentColor = blue_fill;
    currentRow = ROW_2;
    writeChar(icon4, _0);
    currentRow = ROW_3;
    writeChar(icon4, _0);
    currentColor = yellow;
    currentRow = ROW_1;
    writeChar(icon4, _0);

    currentColor = blue;

    // currentRow = ROW_1;
    // writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('1', _27);
    currentRow = ROW_3;
    writeChar(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "1 Input");
    OSD_writeCharRow1(0x15, P8, yellowT);

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
    OSD_writeCharRow1(0x15, P8, (selectedMenuLine == 1) ? yellowT : blue_fill);
    OSD_writeCharRow2(0x15, P20, (selectedMenuLine == 2) ? yellowT : blue_fill);
    OSD_writeCharRow3(0x15, P18, (selectedMenuLine == 3) ? yellowT : blue_fill);

    currentColor = blue;
    // currentRow = ROW_1;
    // writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('1', _27);
    currentRow = ROW_3;
    writeChar(icon6, _27);

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
    OSD_writeCharRow1(0x15, P18, (selectedMenuLine == 1) ? yellowT : blue_fill);
    OSD_writeCharRow2(0x15, P19, (selectedMenuLine == 2) ? yellowT : blue_fill);

    currentColor = blue;
    currentRow = ROW_1;
    writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('2', _27);
    // currentRow = ROW_3;
    // writeChar(icon6, _27);

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

    currentColor = blue;
    // currentRow = ROW_1;
    // writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('1', _27);
    currentRow = ROW_3;
    writeChar(icon6, _27);

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

    currentColor = blue;

    currentRow = ROW_1;
    writeChar(icon5, _27);

    currentRow = ROW_2;
    writeChar('2', _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "1280x720");

    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(1, "480p/576p");
    // currentColor = menuLine3Color;
    // currentRow = ROW_3;
    // writeChar(D, _1), __(o, _2), __(w, _3), __(n, _4), __(s, _5), __(c, _6), __(a, _7), __(l, _8), __(e, _9), __(n1, _11), __(n5, _12), __(K, _13), __(H, _14), __(z, _15);
};
void handle_OutputRes_PassThrough(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = blue;
    currentRow = ROW_1;
    writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar(I, _27);
    currentRow = ROW_3;
    writeChar(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "Pass through");
};
void handle_ScreenSettings(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = blue;

    // currentRow = ROW_1;
    // writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('1', _27);
    currentRow = ROW_3;
    writeChar(icon6, _27);

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
    OSD_writeCharRow1(icon4, P0, yellow);
    OSD_writeCharRow2(icon4, P0, blue_fill);
    OSD_writeCharRow3(icon4, P0, blue_fill);
    selectedMenuLine = 1;
};
void handle_HighlightRow2(void)
{
    OSD_fillBackground();
    OSD_writeCharRow1(icon4, P0, blue_fill);
    OSD_writeCharRow2(icon4, P0, yellow);
    OSD_writeCharRow3(icon4, P0, blue_fill);
    selectedMenuLine = 2;
};
void handle_HighlightRow3(void)
{
    OSD_fillBackground();
    OSD_writeCharRow1(icon4, P0, blue_fill);
    OSD_writeCharRow2(icon4, P0, blue_fill);
    OSD_writeCharRow3(icon4, P0, yellow);
    selectedMenuLine = 3;
};
void handle_ColorSettings_Page1(void)
{
    // Line 2 (Scanlines) disabled when not allowed
    uint8_t line2Color = areScanLinesAllowed() ? main0 : red;
    OSD_setMenuLineColorsWithLine2(selectedMenuLine, line2Color);

    currentColor = blue;

    currentRow = ROW_1;
    writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('2', _27);
    currentRow = ROW_3;
    writeChar(icon6, _27);

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

    currentColor = blue;
    currentRow = ROW_1;
    writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('3', _27);
    currentRow = ROW_3;
    writeChar(icon6, _27);

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

    currentColor = blue;
    currentRow = ROW_1;
    writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('4', _27);
    // currentRow = ROW_3;
    // writeChar(icon6, _27);

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
void handle_ColorSettings_RGB_R(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = blue;
    // currentRow = ROW_1;
    // writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('1', _27);
    currentRow = ROW_3;
    writeChar(icon6, _27);

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
void handle_ColorSettings_RGB_GB(void)
{
    OSD_writeCharRow1(0x3E, P9, main0);
    OSD_writeCharRow1(0x3E, P10, main0);
    OSD_writeCharRow1(0x3E, P11, main0);
    OSD_writeCharRow1(0x3E, P12, main0);
    OSD_writeCharRow1(0x3E, P13, main0);
    OSD_writeCharRow1(0x3E, P14, main0);
    OSD_writeCharRow1(0x3E, P15, main0);
    OSD_writeCharRow1(0x3E, P16, main0);
    OSD_writeCharRow1(0x3E, P17, main0);
    OSD_writeCharRow1(0x3E, P18, main0);
    OSD_writeCharRow1(0x3E, P22, main0);

    // Only show dashes for scanlines if they are available
    if (areScanLinesAllowed()) {
        OSD_writeCharRow2(0x3E, P10, main0);
        OSD_writeCharRow2(0x3E, P11, main0);
        OSD_writeCharRow2(0x3E, P12, main0);
        OSD_writeCharRow2(0x3E, P13, main0);
        OSD_writeCharRow2(0x3E, P14, main0);
        OSD_writeCharRow2(0x3E, P15, main0);
        OSD_writeCharRow2(0x3E, P16, main0);
        OSD_writeCharRow2(0x3E, P17, main0);
        OSD_writeCharRow2(0x3E, P18, main0);
        OSD_writeCharRow2(0x3E, P19, main0);
        OSD_writeCharRow2(0x3E, P22, main0);

        if (uopt->wantScanlines) {
            OSD_writeCharRow2(O, P23, main0);
            OSD_writeCharRow2(N, P24, main0);
            OSD_writeCharRow2(F, P25, blue_fill);
        } else {
            OSD_writeCharRow2(O, P23, main0);
            OSD_writeCharRow2(F, P24, main0);
            OSD_writeCharRow2(F, P25, main0);
        }

        osdDisplayValue = uopt->scanlineStrength;
        if (osdDisplayValue == 0x00) {
            OSD_writeCharRow2(n0, P21, main0);
            OSD_writeCharRow2(n0, P20, main0);
        } else if (osdDisplayValue == 0x10) {
            OSD_writeCharRow2(n0, P21, main0);
            OSD_writeCharRow2(n1, P20, main0);
        } else if (osdDisplayValue == 0x20) {
            OSD_writeCharRow2(n0, P21, main0);
            OSD_writeCharRow2(n2, P20, main0);
        } else if (osdDisplayValue == 0x30) {
            OSD_writeCharRow2(n0, P21, main0);
            OSD_writeCharRow2(n3, P20, main0);
        } else if (osdDisplayValue == 0x40) {
            OSD_writeCharRow2(n0, P21, main0);
            OSD_writeCharRow2(n4, P20, main0);
        } else if (osdDisplayValue == 0x50) {
            OSD_writeCharRow2(n0, P21, main0);
            OSD_writeCharRow2(n5, P20, main0);
        }
    }

    OSD_writeCharRow3(0x3E, P12, main0);
    OSD_writeCharRow3(0x3E, P13, main0);
    OSD_writeCharRow3(0x3E, P14, main0);
    OSD_writeCharRow3(0x3E, P15, main0);
    OSD_writeCharRow3(0x3E, P16, main0);
    OSD_writeCharRow3(0x3E, P17, main0);
    OSD_writeCharRow3(0x3E, P18, main0);
    OSD_writeCharRow3(0x3E, P19, main0);
    OSD_writeCharRow3(0x3E, P20, main0);
    OSD_writeCharRow3(0x3E, P21, main0);
    OSD_writeCharRow3(0x3E, P22, main0);

    if (uopt->wantVdsLineFilter) {
        OSD_writeCharRow3(O, P23, main0);
        OSD_writeCharRow3(N, P24, main0);
        OSD_writeCharRow3(F, P25, blue_fill);
    } else {
        OSD_writeCharRow3(O, P23, main0);
        OSD_writeCharRow3(F, P24, main0);
        OSD_writeCharRow3(F, P25, main0);
    }
    osdDisplayValue = GBS::ADC_RGCTRL::read();
    displayNumber3DigitInverted(osdDisplayValue);

    if (uopt->enableAutoGain == 0) {
        OSD_writeCharRow1(O, P23, main0);
        OSD_writeCharRow1(F, P24, main0);
        OSD_writeCharRow1(F, P25, main0);
    } else {
        OSD_writeCharRow1(O, P23, main0);
        OSD_writeCharRow1(N, P24, main0);
        OSD_writeCharRow1(0x3E, P25, blue_fill);
    }
};
void handle_ColorSettings_Y_Gain(void)
{
    OSD_writeCharRow1(0x3E, P10, main0);
    OSD_writeCharRow1(0x3E, P11, main0);
    OSD_writeCharRow1(0x3E, P12, main0);
    OSD_writeCharRow1(0x3E, P13, main0);
    OSD_writeCharRow1(0x3E, P14, main0);
    OSD_writeCharRow1(0x3E, P15, main0);
    OSD_writeCharRow1(0x3E, P16, main0);
    OSD_writeCharRow1(0x3E, P17, main0);
    OSD_writeCharRow1(0x3E, P18, main0);
    OSD_writeCharRow1(0x3E, P19, main0);
    OSD_writeCharRow1(0x3E, P20, main0);
    OSD_writeCharRow1(0x3E, P21, main0);
    OSD_writeCharRow1(0x3E, P22, main0);
    OSD_writeCharRow2(0x3E, P8, main0);
    OSD_writeCharRow2(0x3E, P9, main0);
    OSD_writeCharRow2(0x3E, P10, main0);
    OSD_writeCharRow2(0x3E, P11, main0);
    OSD_writeCharRow2(0x3E, P12, main0);
    OSD_writeCharRow2(0x3E, P13, main0);
    OSD_writeCharRow2(0x3E, P14, main0);
    OSD_writeCharRow2(0x3E, P15, main0);
    OSD_writeCharRow2(0x3E, P16, main0);
    OSD_writeCharRow2(0x3E, P17, main0);
    OSD_writeCharRow2(0x3E, P18, main0);
    OSD_writeCharRow2(0x3E, P19, main0);
    if (!isPeakingLocked()) {
        OSD_writeCharRow2(0x3E, P20, main0);
        OSD_writeCharRow2(0x3E, P21, main0);
        OSD_writeCharRow2(0x3E, P22, main0);
    }
    OSD_writeCharRow3(0x3E, P14, main0);
    OSD_writeCharRow3(0x3E, P15, main0);
    OSD_writeCharRow3(0x3E, P16, main0);
    OSD_writeCharRow3(0x3E, P17, main0);
    OSD_writeCharRow3(0x3E, P18, main0);
    OSD_writeCharRow3(0x3E, P19, main0);
    OSD_writeCharRow3(0x3E, P20, main0);
    OSD_writeCharRow3(0x3E, P21, main0);
    OSD_writeCharRow3(0x3E, P22, main0);

    if (GBS::VDS_PK_LB_GAIN::read() == 0x16) {
        OSD_writeCharRow1(O, P23, main0);
        OSD_writeCharRow1(F, P24, main0);
        OSD_writeCharRow1(F, P25, main0);
    } else {
        OSD_writeCharRow1(O, P23, main0);
        OSD_writeCharRow1(N, P24, main0);
        OSD_writeCharRow1(F, P25, blue_fill);
    }

    if (isPeakingLocked()) {
        // Locked state - overwrite dashes and ON/OFF with LOCKED
        OSD_writeCharRow2(L, P20, main0);
        OSD_writeCharRow2(O, P21, main0);
        OSD_writeCharRow2(C, P22, main0);
        OSD_writeCharRow2(K, P23, main0);
        OSD_writeCharRow2(E, P24, main0);
        OSD_writeCharRow2(D, P25, main0);
    } else {
        if (uopt->wantPeaking == 0) {
            OSD_writeCharRow2(O, P23, main0);
            OSD_writeCharRow2(F, P24, main0);
            OSD_writeCharRow2(F, P25, main0);
        } else {
            OSD_writeCharRow2(O, P23, main0);
            OSD_writeCharRow2(N, P24, main0);
            OSD_writeCharRow2(F, P25, blue_fill);
        }
    }

    if (uopt->wantStepResponse) {
        OSD_writeCharRow3(O, P23, main0);
        OSD_writeCharRow3(N, P24, main0);
        OSD_writeCharRow3(F, P25, blue_fill);
    } else {
        OSD_writeCharRow3(O, P23, main0);
        OSD_writeCharRow3(F, P24, main0);
        OSD_writeCharRow3(F, P25, main0);
    }
};
void handle_ColorSettings_ADCGain(void)
{
    OSD_drawDashRange(1, 5, 22);  // Row 1: P5-P22
    OSD_drawDashRange(2, 5, 22);  // Row 2: P5-P22
    OSD_drawDashRange(3, 5, 22);  // Row 3: P5-P22

    // osdDisplayValue = (128 + GBS::VDS_Y_OFST::read());  //R
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() + 1.402 * ((signed char)GBS::VDS_V_OFST::read()-128));  //R
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() + 1.5 * ((signed char)GBS::VDS_V_OFST::read()));  //R
    // osdDisplayValue= (signed char)GBS::VDS_Y_OFST::read()+1.402*((signed char)GBS::VDS_V_OFST::read()-128);
    // osdDisplayValue = R_VAL;
    currentColor = main0;
    currentRow = ROW_1;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    // displayNumber3Digit(((signed char)((signed char)GBS::VDS_Y_OFST::read()) +(float)( 1.402     * (signed char)((signed char)GBS::VDS_V_OFST::read()) )) + 128);
    displayNumber3Digit(R_VAL);
    // osdDisplayValue = (128 + GBS::VDS_U_OFST::read());  //G
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() - 0.88 * ((signed char)GBS::VDS_U_OFST::read()) - 0.764 * ((signed char)GBS::VDS_V_OFST::read()));  //G
    // osdDisplayValue = (signed char)GBS::VDS_Y_OFST::read()-0.344136*((signed char)GBS::VDS_U_OFST::read()-128)-0.714136*((signed char)GBS::VDS_V_OFST::read()-128);
    // osdDisplayValue = G_VAL;
    currentColor = main0;
    currentRow = ROW_2;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    // displayNumber3Digit(((signed char)((signed char)GBS::VDS_Y_OFST::read()) -(float)( 0.344136  * (signed char)((signed char)GBS::VDS_U_OFST::read()) )- 0.714136 * (signed char)((signed char)GBS::VDS_V_OFST::read()) ) + 128);
    displayNumber3Digit(G_VAL);

    // osdDisplayValue = (128 + GBS::VDS_V_OFST::read());  //B
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() + 2 * ((signed char)GBS::VDS_U_OFST::read()));  //B
    // osdDisplayValue = (signed char)GBS::VDS_Y_OFST::read()+1.772*((signed char)GBS::VDS_U_OFST::read()-128);
    // osdDisplayValue = B_VAL;
    currentColor = main0;
    currentRow = ROW_3;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    // displayNumber3Digit(((signed char)((signed char)GBS::VDS_Y_OFST::read()) +(float)( 1.772     * (signed char)((signed char)GBS::VDS_U_OFST::read()) )) + 128);
    displayNumber3Digit(B_VAL);
};
void handle_SysSettings_SVInput_Page1(void)
{
    OSD_drawDashRange(1, 7, 22);  // Row 1: P7-P22
    OSD_drawDashRange(2, 6, 22);  // Row 2: P6-P22

    osdDisplayValue = GBS::VDS_Y_GAIN::read();
    currentColor = main0;
    currentRow = ROW_1;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    displayNumber3Digit(osdDisplayValue);
    osdDisplayValue = GBS::VDS_VCOS_GAIN::read();
    currentColor = main0;
    currentRow = ROW_2;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    displayNumber3Digit(osdDisplayValue);
};
void handle_SysSettings_Page1(void)
{
    // Line 1 (SV/AV Input Settings) disabled when not SV/AV input
    bool isSvAvInput = (inputType == InputTypeSV) || (inputType == InputTypeAV);
    OSD_setMenuLineColors(selectedMenuLine);
    if (!isSvAvInput) menuLine1Color = red;  // Override line 1 color if disabled
    OSD_writeCharRow1(0x15, P21, (selectedMenuLine == 1) ? yellowT : blue_fill);

    currentColor = blue;
    // currentRow = ROW_1;
    // writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('1', _27);
    currentRow = ROW_3;
    writeChar(icon6, _27);

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
void handle_SysSettings_Page2(void)
{
    // OSD_writeCharRow2(0x3E, P16, main0);
    // OSD_writeCharRow2(0x3E, P17, main0);
    // OSD_writeCharRow2(0x3E, P18, main0);
    OSD_writeCharRow2(0x3E, P19, main0);
    OSD_writeCharRow2(0x3E, P20, main0);
    OSD_writeCharRow2(0x3E, P21, main0);
    OSD_writeCharRow2(0x3E, P22, main0);
    if (rgbComponentMode == 1) {
        OSD_writeCharRow2(O, P23, main0);
        OSD_writeCharRow2(N, P24, main0);
        OSD_writeCharRow2(F, P25, blue_fill);
    } else {
        OSD_writeCharRow2(O, P23, main0);
        OSD_writeCharRow2(F, P24, main0);
        OSD_writeCharRow2(F, P25, main0);
    }
    OSD_writeCharRow3(0x3E, P16, main0);
    OSD_writeCharRow3(0x3E, P17, main0);
    OSD_writeCharRow3(0x3E, P18, main0);
    OSD_writeCharRow3(0x3E, P19, main0);
    OSD_writeCharRow3(0x3E, P20, main0);
    OSD_writeCharRow3(0x3E, P21, main0);
    OSD_writeCharRow3(0x3E, P22, main0);
    if (uopt->matchPresetSource) {
        OSD_writeCharRow3(O, P23, main0);
        OSD_writeCharRow3(N, P24, main0);     // ON
        OSD_writeCharRow3(F, P25, blue_fill); // ON
    } else {
        OSD_writeCharRow3(O, P23, main0);
        OSD_writeCharRow3(F, P24, main0);
        OSD_writeCharRow3(F, P25, main0); // OFF
    }
    /*
    upscaling
        // OSD_writeCharRow3(0x3E, P21, main0);
        // OSD_writeCharRow3(0x3E, P22, main0);
        // if (uopt->preferScalingRgbhv)
        // {
        //   OSD_writeCharRow3(O, P23, main0);
        //   OSD_writeCharRow3(N, P24, main0);
        //   OSD_writeCharRow3(F, P25, blue_fill);
        // }
        // else
        // {
        //   OSD_writeCharRow3(O, P23, main0);
        //   OSD_writeCharRow3(F, P24, main0);
        //   OSD_writeCharRow3(F, P25, main0);
        // }
    */
};
void handle_SysSettings_Page3(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = blue;
    currentRow = ROW_1;
    writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('2', _27);
    currentRow = ROW_3;
    writeChar(icon6, _27);

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
void handle_SysSettings_SVInput_Page2(void)
{
    OSD_writeCharRow1(0x3E, P12, main0);
    OSD_writeCharRow1(0x3E, P13, main0);
    OSD_writeCharRow1(0x3E, P14, main0);
    OSD_writeCharRow1(0x3E, P15, main0);
    OSD_writeCharRow1(0x3E, P16, main0);
    OSD_writeCharRow1(0x3E, P17, main0);
    if (uopt->deintMode == 0) {
        OSD_writeCharRow1(A, P18, main0);
        OSD_writeCharRow1(d, P19, main0);
        OSD_writeCharRow1(a, P20, main0);
        OSD_writeCharRow1(p, P21, main0);
        OSD_writeCharRow1(t, P22, main0);
        OSD_writeCharRow1(i, P23, main0);
        OSD_writeCharRow1(v, P24, main0);
        OSD_writeCharRow1(e, P25, main0);
    } else {
        OSD_writeCharRow1(0x3E, P18, main0);
        OSD_writeCharRow1(0x3E, P19, main0);
        OSD_writeCharRow1(0x3E, P20, main0);
        OSD_writeCharRow1(0x3E, P21, main0);
        OSD_writeCharRow1(0x3E, P22, main0);
        OSD_writeCharRow1(B, P23, main0);
        OSD_writeCharRow1(o, P24, main0);
        OSD_writeCharRow1(b, P25, main0);
    }

    // OSD_writeCharRow1(0x3E, P21, main0);
    // OSD_writeCharRow1(0x3E, P22, main0);
    OSD_writeCharRow2(0x3E, P19, main0);
    OSD_writeCharRow2(0x3E, P20, main0);
    OSD_writeCharRow2(0x3E, P21, main0);
    OSD_writeCharRow2(0x3E, P22, main0);
    // OSD_writeCharRow3(0x3E, P16, main0);
    // OSD_writeCharRow3(0x3E, P17, main0);
    // OSD_writeCharRow3(0x3E, P18, main0);
    // OSD_writeCharRow3(0x3E, P19, main0);
    // OSD_writeCharRow3(0x3E, P20, main0);
    // OSD_writeCharRow3(0x3E, P21, main0);
    // OSD_writeCharRow3(0x3E, P22, main0);
    OSD_writeCharRow3(0x3E, P12, main0);
    OSD_writeCharRow3(0x3E, P13, main0);

    // if (uopt->wantOutputComponent)
    // {
    //     OSD_writeCharRow1(O, P23, main0);
    //     OSD_writeCharRow1(N, P24, main0);
    //     OSD_writeCharRow1(F, P25, blue_fill);
    // }
    // else
    // {
    //     OSD_writeCharRow1(O, P23, main0);
    //     OSD_writeCharRow1(F, P24, main0);
    //     OSD_writeCharRow1(F, P25, main0);
    // }

    if (uopt->PalForce60) {
        OSD_writeCharRow2(O, P23, main0);
        OSD_writeCharRow2(N, P24, main0);
        OSD_writeCharRow2(F, P25, blue_fill);
    } else {
        OSD_writeCharRow2(O, P23, main0);
        OSD_writeCharRow2(F, P24, main0);
        OSD_writeCharRow2(F, P25, main0);
    }

    // if (uopt->disableExternalClockGenerator)
    // {
    //   OSD_writeCharRow3(O, P23, main0);
    //   OSD_writeCharRow3(F, P24, main0);
    //   OSD_writeCharRow3(F, P25, main0);
    // }
    // else
    // {
    //   OSD_writeCharRow3(O, P23, main0);
    //   OSD_writeCharRow3(N, P24, main0);
    //   OSD_writeCharRow3(F, P25, blue_fill);
    // }

    if (uopt->frameTimeLockMethod == 0) {
        OSD_writeCharRow3(n0, P14, main0);
        OSD_writeCharRow3(V, P15, main0);
        OSD_writeCharRow3(t, P16, main0);
        OSD_writeCharRow3(o, P17, main0);
        OSD_writeCharRow3(t, P18, main0);
        OSD_writeCharRow3(a, P19, main0);
        OSD_writeCharRow3(l, P20, main0);
        OSD_writeCharRow3(0x3C, P21, main0);
        OSD_writeCharRow3(V, P22, main0);
        OSD_writeCharRow3(S, P23, main0);
        OSD_writeCharRow3(S, P24, main0);
        OSD_writeCharRow3(T, P25, main0);
    } else {
        OSD_writeCharRow3(n1, P14, main0);
        OSD_writeCharRow3(V, P15, main0);
        OSD_writeCharRow3(t, P16, main0);
        OSD_writeCharRow3(o, P17, main0);
        OSD_writeCharRow3(t, P18, main0);
        OSD_writeCharRow3(a, P19, main0);
        OSD_writeCharRow3(l, P20, main0);
        OSD_writeCharRow3(o, P22, main0);
        OSD_writeCharRow3(n, P23, main0);
        OSD_writeCharRow3(l, P24, main0);
        OSD_writeCharRow3(y, P25, main0);
        OSD_writeCharRow3(F, P21, blue_fill);
    }
};
void handle_Reserved_M(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = blue;
    currentRow = ROW_1;
    writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('3', _27);
    // currentRow = ROW_3;
    // writeChar(icon6, _27);

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
void handle_Reserved_N(void)
{
    OSD_drawDashRange(1, 16, 22);  // Row 1: P16-P22
    OSD_drawDashRange(2, 16, 22);  // Row 2: P16-P22
    OSD_drawDashRange(3, 20, 22);  // Row 3: P20-P22

    if (uopt->enableCalibrationADC) {
        OSD_writeCharRow1(O, P23, main0);
        OSD_writeCharRow1(N, P24, main0);
        OSD_writeCharRow1(F, P25, blue_fill);
    } else {
        OSD_writeCharRow1(O, P23, main0);
        OSD_writeCharRow1(F, P24, main0);
        OSD_writeCharRow1(F, P25, main0);
    }

    if (uopt->enableFrameTimeLock) {
        OSD_writeCharRow2(O, P23, main0);
        OSD_writeCharRow2(N, P24, main0);
        OSD_writeCharRow2(F, P25, blue_fill);
    } else {
        OSD_writeCharRow2(O, P23, main0);
        OSD_writeCharRow2(F, P24, main0);
        OSD_writeCharRow2(F, P25, main0);
    }

    if (uopt->disableExternalClockGenerator) {
        OSD_writeCharRow3(O, P23, main0);
        OSD_writeCharRow3(F, P24, main0);
        OSD_writeCharRow3(F, P25, main0);
    } else {
        OSD_writeCharRow3(O, P23, main0);
        OSD_writeCharRow3(N, P24, main0);
        OSD_writeCharRow3(F, P25, blue_fill);
    }
};
void handle_ScreenSettings_FullHeight(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = blue;
    currentRow = ROW_1;
    writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('2', _27);
    // currentRow = ROW_3;
    // writeChar(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "Full height");

    // currentColor = menuLine2Color;
    // currentRow = ROW_2;
    // writeChar(M, _1), __(a, _2), __(t, _3), __(c, _4), __(h, _5), __(e, _6), __(d, _7), __(p, _9), __(r, _10), __(e, _11), __(s, _12), __(e, _13), __(t, _14), __(s, _15);
};
void handle_Reserved_P(void)
{
    OSD_writeCharRow1(0x3E, P12, main0);
    OSD_writeCharRow1(0x3E, P13, main0);
    OSD_writeCharRow1(0x3E, P14, main0);
    OSD_writeCharRow1(0x3E, P15, main0);
    OSD_writeCharRow1(0x3E, P16, main0);
    OSD_writeCharRow1(0x3E, P17, main0);
    OSD_writeCharRow1(0x3E, P18, main0);
    OSD_writeCharRow1(0x3E, P19, main0);
    OSD_writeCharRow1(0x3E, P20, main0);
    OSD_writeCharRow1(0x3E, P21, main0);
    OSD_writeCharRow1(0x3E, P22, main0);
    // OSD_writeCharRow3(0x3E, P22, main0);

    if (uopt->wantFullHeight) {
        OSD_writeCharRow1(O, P23, main0);
        OSD_writeCharRow1(N, P24, main0);
        OSD_writeCharRow1(F, P25, blue_fill);
    } else {
        OSD_writeCharRow1(O, P23, main0);
        OSD_writeCharRow1(F, P24, main0);
        OSD_writeCharRow1(F, P25, main0);
    }
};
void handle_Developer_Memory(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = blue;
    currentRow = ROW_1;
    writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar(I, _27);
    currentRow = ROW_3;
    writeChar(icon6, _27);

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
void handle_Developer_HSync(void)
{
    OSD_drawDashRange(1, 15, 22);  // Row 1: P15-P22
    OSD_writeCharRow1(0x03, P23, yellow);
    OSD_writeCharRow1(0x13, P24, yellow);
    OSD_drawDashRange(2, 14, 22);  // Row 2: P14-P22
    OSD_writeCharRow2(0x03, P23, yellow);
    OSD_writeCharRow2(0x13, P24, yellow);
    OSD_drawDashRange(3, 7, 22);   // Row 3: P7-P22
    osdDisplayValue = GBS::VDS_HSYNC_RST::read();
    currentColor = main0;
    currentRow = ROW_3;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    displayNumber3Digit(osdDisplayValue);
};
void handle_Developer_Debug(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = blue;
    currentRow = ROW_1;
    writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar(I, _27);
    currentRow = ROW_3;
    writeChar(icon6, _27);

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
void handle_Developer_Page(void)
{
    OSD_drawDashRange(1, 11, 22);  // Row 1: P11-P22
    OSD_drawDashRange(2, 11, 22);  // Row 2: P11-P22
    currentColor = main0;
    currentRow = ROW_3;
    writeChar(0x3E, _15), writeChar(0x3E, _16), writeChar(0x3E, _17), writeChar(0x3E, _18), writeChar(0x3E, _19), writeChar(0x3E, _20), writeChar(0x3E, _21), writeChar(0x3E, _22);

    if (GBS::ADC_UNUSED_62::read() == 0x00) {
        OSD_writeCharRow1(O, P23, main0);
        OSD_writeCharRow1(F, P24, main0);
        OSD_writeCharRow1(F, P25, main0);
    } else {
        OSD_writeCharRow1(O, P23, main0);
        OSD_writeCharRow1(N, P24, main0);
        OSD_writeCharRow1(F, P25, blue_fill);
    }

    if (GBS::ADC_FLTR::read() > 0) {
        OSD_writeCharRow2(O, P23, main0);
        OSD_writeCharRow2(N, P24, main0);
        OSD_writeCharRow2(F, P25, blue_fill);
    } else {
        OSD_writeCharRow2(O, P23, main0);
        OSD_writeCharRow2(F, P24, main0);
        OSD_writeCharRow2(F, P25, main0);
    }

    if (GBS::CAPTURE_ENABLE::read() > 0) {
        OSD_writeCharRow3(O, P23, main0);
        OSD_writeCharRow3(F, P24, main0);
        OSD_writeCharRow3(F, P25, main0);
    } else {
        OSD_writeCharRow3(O, P23, main0);
        OSD_writeCharRow3(N, P24, main0);
        OSD_writeCharRow3(F, P25, blue_fill);
    }
};
void handle_Reserved_U(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = blue;
    currentRow = ROW_1;
    writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar(I, _27);
    currentRow = ROW_3;
    writeChar(icon6, _27);

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
void handle_ResetSettings(void)
{
    OSD_writeCharRow1(0x3E, P11, main0);
    OSD_writeCharRow1(0x3E, P12, main0);
    OSD_writeCharRow1(0x3E, P13, main0);
    OSD_writeCharRow1(0x3E, P14, main0);
    OSD_writeCharRow1(0x3E, P15, main0);
    OSD_writeCharRow1(0x3E, P16, main0);
    OSD_writeCharRow1(0x3E, P17, main0);
    OSD_writeCharRow1(0x3E, P18, main0);
    OSD_writeCharRow1(0x3E, P19, main0);
    OSD_writeCharRow1(0x3E, P20, main0);
    OSD_writeCharRow1(0x3E, P21, main0);
    OSD_writeCharRow1(0x3E, P22, main0);

    if (rto->allowUpdatesOTA) {
        OSD_writeCharRow1(O, P23, main0);
        OSD_writeCharRow1(N, P24, main0);
        OSD_writeCharRow1(F, P25, blue_fill);
    } else {
        OSD_writeCharRow1(O, P23, main0);
        OSD_writeCharRow1(F, P24, main0);
        OSD_writeCharRow1(F, P25, main0);
    }
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

    currentColor = yellowT;
    currentRow = ROW_3;
    OSD_writeString(1, "Active save:");
};
void handle_Profile_SlotDisplay(void)
{
    if (oled_menuItem == OLED_Profile) {
        currentColor = main0;
        currentRow = ROW_1;
        name_1();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_SaveConfirm) {
        currentColor = main0;
        currentRow = ROW_1;
        name_2();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Save) {
        currentColor = main0;
        currentRow = ROW_1;
        name_3();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Load) {
        currentColor = main0;
        currentRow = ROW_1;
        name_4();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Operation1) {
        currentColor = main0;
        currentRow = ROW_1;
        name_5();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Operation2) {
        currentColor = main0;
        currentRow = ROW_1;
        name_6();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Operation3) {
        currentColor = main0;
        currentRow = ROW_1;
        name_7();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot7) {
        currentColor = main0;
        currentRow = ROW_1;
        name_8();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot8) {
        currentColor = main0;
        currentRow = ROW_1;
        name_9();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot9) {
        currentColor = main0;
        currentRow = ROW_1;
        name_10();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot10) {
        currentColor = main0;
        currentRow = ROW_1;
        name_11();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot11) {
        currentColor = main0;
        currentRow = ROW_1;
        name_12();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot12) {
        currentColor = main0;
        currentRow = ROW_1;
        name_13();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot13) {
        currentColor = main0;
        currentRow = ROW_1;
        name_14();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot14) {
        currentColor = main0;
        currentRow = ROW_1;
        name_15();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot15) {
        currentColor = main0;
        currentRow = ROW_1;
        name_16();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot16) {
        currentColor = main0;
        currentRow = ROW_1;
        name_17();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot17) {
        currentColor = main0;
        currentRow = ROW_1;
        name_18();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot18) {
        currentColor = main0;
        currentRow = ROW_1;
        name_19();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot19) {
        currentColor = main0;
        currentRow = ROW_1;
        name_20();
        displayProfileName();
    }

    if (uopt->presetSlot == 'A') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_1();
        displayProfileName();
    } else if (uopt->presetSlot == 'B') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_2();
        displayProfileName();
    } else if (uopt->presetSlot == 'C') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_3();
        displayProfileName();
    } else if (uopt->presetSlot == 'D') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_4();
        displayProfileName();
    } else if (uopt->presetSlot == 'E') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_5();
        displayProfileName();
    } else if (uopt->presetSlot == 'F') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_6();
        displayProfileName();
    } else if (uopt->presetSlot == 'G') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_7();
        displayProfileName();
    } else if (uopt->presetSlot == 'H') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_8();
        displayProfileName();
    } else if (uopt->presetSlot == 'I') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_9();
        displayProfileName();
    } else if (uopt->presetSlot == 'J') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_10();
        displayProfileName();
    } else if (uopt->presetSlot == 'K') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_11();
        displayProfileName();
    } else if (uopt->presetSlot == 'L') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_12();
        displayProfileName();
    } else if (uopt->presetSlot == 'M') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_13();
        displayProfileName();
    } else if (uopt->presetSlot == 'N') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_14();
        displayProfileName();
    } else if (uopt->presetSlot == 'O') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_15();
        displayProfileName();
    } else if (uopt->presetSlot == 'P') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_16();
        displayProfileName();
    } else if (uopt->presetSlot == 'Q') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_17();
        displayProfileName();
    } else if (uopt->presetSlot == 'R') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_18();
        displayProfileName();
    } else if (uopt->presetSlot == 'S') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_19();
        displayProfileName();
    } else if (uopt->presetSlot == 'T') {
        currentColor = yellowT;
        currentRow = ROW_3;
        name_20();
        displayProfileName();
    }

    if (oled_menuItem == OLED_Profile_SelectSlot) {
        currentColor = main0;
        currentRow = ROW_2;
        name_1();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot1) {
        currentColor = main0;
        currentRow = ROW_2;
        name_2();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot2) {
        currentColor = main0;
        currentRow = ROW_2;
        name_3();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot3) {
        currentColor = main0;
        currentRow = ROW_2;
        name_4();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot4) {
        currentColor = main0;
        currentRow = ROW_2;
        name_5();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot5) {
        currentColor = main0;
        currentRow = ROW_2;
        name_6();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Slot6) {
        currentColor = main0;
        currentRow = ROW_2;
        name_7();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_SelectPreset) {
        currentColor = main0;
        currentRow = ROW_2;
        name_8();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Preset1) {
        currentColor = main0;
        currentRow = ROW_2;
        name_9();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Preset2) {
        currentColor = main0;
        currentRow = ROW_2;
        name_10();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Preset3) {
        currentColor = main0;
        currentRow = ROW_2;
        name_11();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Preset4) {
        currentColor = main0;
        currentRow = ROW_2;
        name_12();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Preset5) {
        currentColor = main0;
        currentRow = ROW_2;
        name_13();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Preset6) {
        currentColor = main0;
        currentRow = ROW_2;
        name_14();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Preset7) {
        currentColor = main0;
        currentRow = ROW_2;
        name_15();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Preset8) {
        currentColor = main0;
        currentRow = ROW_2;
        name_16();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Preset9) {
        currentColor = main0;
        currentRow = ROW_2;
        name_17();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Preset10) {
        currentColor = main0;
        currentRow = ROW_2;
        name_18();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Preset11) {
        currentColor = main0;
        currentRow = ROW_2;
        name_19();
        displayProfileName();
    } else if (oled_menuItem == OLED_Profile_Preset12) {
        currentColor = main0;
        currentRow = ROW_2;
        name_20();
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

    currentColor = main0;
    currentRow = ROW_1;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    displayNumber3Digit(contrast);

    OSD_drawDashRange(2, 13, 18);  // Row 2: P13-P18


    currentColor = main0;
    currentRow = ROW_2;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    displayNumber3Digit(saturation);
};


void handle_ADCCalib_Running(void)
{
    // Line 2 (Smooth) disabled when lineOption is false
    uint8_t line2Color = lineOption ? main0 : red;
    OSD_setMenuLineColorsWithLine2(selectedMenuLine, line2Color);

    // currentColor = blue;
    // currentRow = ROW_1;
    // writeChar(icon5, _27);
    // currentRow = ROW_2;
    // writeChar(I, _27);
    // currentRow = ROW_3;
    // writeChar(icon6, _27);

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

    currentColor = blue;
    // currentRow = ROW_1;
    // writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('1', _27);
    currentRow = ROW_3;
    writeChar(icon6, _27);

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
    menuLine1Color = main0;
    menuLine2Color = main0;
    menuLine3Color = main0;

    currentColor = blue;
    // currentRow = ROW_1;
    // writeChar(icon5, _27);
    // currentRow = ROW_2;
    // writeChar('1', _27);
    // currentRow = ROW_3;
    // writeChar(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(0, "Whether to keep the settings");

    currentColor = menuLine2Color;
    currentRow = ROW_2;
    OSD_writeString(0, "Restore in ");

    if (keepSettings) {
        currentColor = yellowT;
        OSD_writeCharRow3(0x15, P2, yellowT);
    } else {
        currentColor = menuLine3Color;
        OSD_writeCharRow3(0x15, P2, blue_fill);
    }
    currentRow = ROW_3;
    OSD_writeString(3, "Changes");

    if (!keepSettings) {
        currentColor = yellowT;
        OSD_writeCharRow3(0x15, P13, yellowT);
    } else {
        currentColor = menuLine3Color;
        OSD_writeCharRow3(0x15, P13, blue_fill);
    }
    OSD_writeString(0xff, "    Recover");

    // OSD_writeCharRow3(0x15, P2, blue_fill);
};
void handle_InputMenu_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = blue;
    currentRow = ROW_1;
    writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('2', _27);
    // currentRow = ROW_3;
    // writeChar(icon6, _27);

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
    if (oled_menuItem == OLED_Input_SV) {
        OSD_writeStringAtLine(4, 2, "Format:");
        OSD_writeStringAtLine(4, 3, "                      ");
    } else if (oled_menuItem == OLED_Input_AV) {
        OSD_writeStringAtLine(4, 3, "Format:");
        OSD_writeStringAtLine(4, 2, "                      ");
    } else {
        OSD_writeStringAtLine(4, 2, "                      ");
        OSD_writeStringAtLine(4, 3, "                      ");
    }
    switch (SVModeOption) {
        case 0: {
            if (oled_menuItem == OLED_Input_SV)
                OSD_writeStringAtLine(11, 2, "Auto           ");
        } break;
        case 1: {
            if (oled_menuItem == OLED_Input_SV)
                OSD_writeStringAtLine(11, 2, "PAL            ");
        } break;
        case 2: {
            if (oled_menuItem == OLED_Input_SV)
                OSD_writeStringAtLine(11, 2, "NTSC-M         ");
        } break;
        case 3: {
            if (oled_menuItem == OLED_Input_SV)
                OSD_writeStringAtLine(11, 2, "PAL-60         ");
        } break;
        case 4: {
            if (oled_menuItem == OLED_Input_SV)
                OSD_writeStringAtLine(11, 2, "NTSC443        ");
        } break;
        case 5: {
            if (oled_menuItem == OLED_Input_SV)
                OSD_writeStringAtLine(11, 2, "NTSC-J          ");
        } break;
        case 6: {
            if (oled_menuItem == OLED_Input_SV)
                OSD_writeStringAtLine(11, 2, "PAL-N w/ p      ");
        } break;
        case 7: {
            if (oled_menuItem == OLED_Input_SV)
                OSD_writeStringAtLine(11, 2, "PAL-M w/o p    ");
        } break;
        case 8: {
            if (oled_menuItem == OLED_Input_SV)
                OSD_writeStringAtLine(11, 2, "PAL-M          ");
        } break;
        case 9: {
            if (oled_menuItem == OLED_Input_SV)
                OSD_writeStringAtLine(11, 2, "PAL Cmb -N     ");
        } break;
        case 10: {
            if (oled_menuItem == OLED_Input_SV)
                OSD_writeStringAtLine(11, 2, "PAL Cmb -N w/ p");
        } break;
        case 11: {
            if (oled_menuItem == OLED_Input_SV)
                OSD_writeStringAtLine(11, 2, "SECAM          ");
        } break;
        default: {
            if (oled_menuItem == OLED_Input_SV)
                OSD_writeStringAtLine(11, 2, "               ");
        } break;
    }

    switch (AVModeOption) {
        case 0: {
            if (oled_menuItem == OLED_Input_AV)
                OSD_writeStringAtLine(11, 3, "Auto           ");
        } break;
        case 1: {
            if (oled_menuItem == OLED_Input_AV)
                OSD_writeStringAtLine(11, 3, "PAL            ");
        } break;
        case 2: {
            if (oled_menuItem == OLED_Input_AV)
                OSD_writeStringAtLine(11, 3, "NTSC-M         ");
        } break;
        case 3: {
            if (oled_menuItem == OLED_Input_AV)
                OSD_writeStringAtLine(11, 3, "PAL-60         ");
        } break;
        case 4: {
            if (oled_menuItem == OLED_Input_AV)
                OSD_writeStringAtLine(11, 3, "NTSC443        ");
        } break;
        case 5: {
            if (oled_menuItem == OLED_Input_AV)
                OSD_writeStringAtLine(11, 3, "NTSC-J          ");
        } break;
        case 6: {
            if (oled_menuItem == OLED_Input_AV)
                OSD_writeStringAtLine(11, 3, "PAL-N w/ p      ");
        } break;
        case 7: {
            if (oled_menuItem == OLED_Input_AV)
                OSD_writeStringAtLine(11, 3, "PAL-M w/o p    ");
        } break;
        case 8: {
            if (oled_menuItem == OLED_Input_AV)
                OSD_writeStringAtLine(11, 3, "PAL-M          ");
        } break;
        case 9: {
            if (oled_menuItem == OLED_Input_AV)
                OSD_writeStringAtLine(11, 3, "PAL Cmb -N     ");
        } break;
        case 10: {
            if (oled_menuItem == OLED_Input_AV)
                OSD_writeStringAtLine(11, 3, "PAL Cmb -N w/ p");
        } break;
        case 11: {
            if (oled_menuItem == OLED_Input_AV)
                OSD_writeStringAtLine(11, 3, "SECAM          ");
        } break;
        default: {
            if (oled_menuItem == OLED_Input_AV)
                OSD_writeStringAtLine(11, 3, "               ");
        } break;
    }
};
void handle_InfoDisplay_Source(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = blue;
    currentRow = ROW_1;
    writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar(I, _27);
    currentRow = ROW_3;
    writeChar(icon6, _27);

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
        OSD_writeCharRow1(n2, P23, main0);
        OSD_writeCharRow1(X, P24, main0);
    } else {
        OSD_writeCharRow1(n1, P23, main0);
        OSD_writeCharRow1(X, P24, main0);
        smoothOption = false;
    }
    if (smoothOption) {
        OSD_writeCharRow2(O, P23, main0);
        OSD_writeCharRow2(N, P24, main0);
        OSD_writeCharRow2(F, P25, blue_fill);
    } else {
        OSD_writeCharRow2(O, P23, main0);
        OSD_writeCharRow2(F, P24, main0);
        OSD_writeCharRow2(F, P25, main0);
    }

    currentColor = main0;
    currentRow = ROW_3;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = _23;
    displayNumber3Digit(brightness);


    // currentColor = main0;
    // currentRow = ROW_3;
    // digitPos1 = _25;
    // digitPos2 = _24;
    // digitPos3 = _23;
    // displayNumber3Digit(contrast);
};
void handle_Restart(void)
{
    OSD_setMenuLineColors(selectedMenuLine);

    currentColor = blue;
    currentRow = ROW_1;
    writeChar(icon5, _27);
    currentRow = ROW_2;
    writeChar('4', _27);
    // currentRow = ROW_3;
    // writeChar(icon6, _27);

    currentColor = menuLine1Color;
    currentRow = ROW_1;
    OSD_writeString(1, "Matched presets");
    // currentColor = menuLine2Color;
    // currentRow = ROW_2;
};
