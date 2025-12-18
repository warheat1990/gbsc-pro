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
// TV OSD Display Helper Functions
// ====================================================================================

void OSD_writeChar(const int T, const char C)
{
    __(T, (C * 2) + 1);
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
            osd_cx_ptr = OSD_c1;
        else if (row == 2)
            osd_cx_ptr = OSD_c2;
        else if (row == 3)
            osd_cx_ptr = OSD_c3;

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
    {'0', handle_0},
    {'1', handle_1},
    {'2', handle_2},
    {'3', handle_3},
    {'4', handle_4},
    {'5', handle_5},
    {'6', handle_6},
    {'7', handle_7},
    {'8', handle_8},
    {'9', handle_9},
    {'a', handle_a},
    {'b', handle_b},
    {'c', handle_c},
    {'d', handle_d},
    {'e', handle_e},
    {'f', handle_f},
    {'g', handle_g},
    {'h', handle_h},
    {'i', handle_i},
    {'j', handle_j},
    {'k', handle_k},
    {'l', handle_l},
    {'m', handle_m},
    {'n', handle_n},
    {'o', handle_o},
    {'p', handle_p},
    {'q', handle_q},
    {'r', handle_r},
    {'s', handle_s},
    {'t', handle_t},
    {'u', handle_u},
    {'v', handle_v},
    {'w', handle_w},
    {'x', handle_x},
    {'y', handle_y},
    {'z', handle_z},
    {'A', handle_A},
    {'^', handle_caret},
    {'@', handle_at},
    {'!', handle_exclamation},
    {'#', handle_hash},
    {'$', handle_dollar},
    {'%', handle_percent},
    {'&', handle_ampersand},
    {'*', handle_asterisk}
};

const size_t menuTableSize = sizeof(menuTable) / sizeof(menuTable[0]);

// ====================================================================================
// MENU DISPATCHER
// ====================================================================================

static bool isMainMenuCommand(char cmd) {
    static const char mainMenuCommands[] = "abcdikmowz@#^";
    return strchr(mainMenuCommands, cmd) != nullptr;
}

void OSD_handleCommand(char incomingByte)
{
    const unsigned char key = (unsigned char)incomingByte;

    for (size_t i = 0; i < menuTableSize; i++) {
        if (menuTable[i].key == key) {
            // Save only main menu commands that calculate colors, not update commands
            if (isMainMenuCommand(key)) {
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

void handle_0(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    // OSD_c2(0x15, P9 , blue_fill);
    // OSD_c3(0x15, P18, blue_fill);

    OSD_background();
    colour1 = blue_fill;
    number_stroca = stroca2;
    __(icon4, _0);
    number_stroca = stroca3;
    __(icon4, _0);
    colour1 = yellow;
    number_stroca = stroca1;
    __(icon4, _0);

    colour1 = blue;

    // number_stroca = stroca1;
    // __(icon5, _27);
    number_stroca = stroca2;
    __('1', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "1 Input");
    OSD_c1(0x15, P8, yellowT);

    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "2 Output Resolution");

    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "3 Screen Settings");
};
void handle_1(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;

        OSD_c1(0x15, P8, yellowT);
        OSD_c2(0x15, P20, blue_fill);
        OSD_c3(0x15, P18, blue_fill);
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;

        OSD_c1(0x15, P8, blue_fill);
        OSD_c2(0x15, P20, yellowT);
        OSD_c3(0x15, P18, blue_fill);
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;

        OSD_c1(0x15, P8, blue_fill);
        OSD_c2(0x15, P20, blue_fill);
        OSD_c3(0x15, P18, yellowT);
    }

    colour1 = blue;
    // number_stroca = stroca1;
    // __(icon5, _27);
    number_stroca = stroca2;
    __('1', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "1 Input");

    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "2 Output Resolution"); //__(0X15, _9);

    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "3 Screen Settings"); //__(0X15, _18);
};
void handle_2(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;

        OSD_c1(0x15, P18, yellowT);
        OSD_c2(0x15, P19, blue_fill);
        // OSD_c3(0x15, P15 , blue_fill  );
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;

        OSD_c1(0x15, P18, blue_fill);
        OSD_c2(0x15, P19, yellowT);
        // OSD_c3(0x15, P15 , blue_fill  );
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;

        OSD_c1(0x15, P18, blue_fill);
        OSD_c2(0x15, P19, blue_fill);
        // OSD_c3(0x15, P15 , blue_fill  );
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('2', _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "4 System Settings");
    colour1 = A2_main0;
    number_stroca = stroca2;
    // OSD_writeString(1, "5 Color Settings");
    OSD_writeString(1, "5 Picture Settings");
    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "6 Reset Settings");
};
void handle_3(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    // number_stroca = stroca1;
    // __(icon5, _27);
    number_stroca = stroca2;
    __('1', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "1920x1080");
    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "1280x1024");
    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "1280x960");
};
void handle_4(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    }

    colour1 = blue;

    number_stroca = stroca1;
    __(icon5, _27);

    number_stroca = stroca2;
    __('2', _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "1280x720");

    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "480p/576p");
    // colour1 = A3_main0;
    // number_stroca = stroca3;
    // __(D, _1), __(o, _2), __(w, _3), __(n, _4), __(s, _5), __(c, _6), __(a, _7), __(l, _8), __(e, _9), __(n1, _11), __(n5, _12), __(K, _13), __(H, _14), __(z, _15);
};
void handle_5(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __(I, _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "Pass through");
};
void handle_6(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;

    // number_stroca = stroca1;
    // __(icon5, _27);
    number_stroca = stroca2;
    __('1', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "Move");
    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "Scale");
    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "Borders");
};
void handle_7(void)
{
    OSD_background();
    OSD_c1(icon4, P0, yellow);
    OSD_c2(icon4, P0, blue_fill);
    OSD_c3(icon4, P0, blue_fill);
    selectedMenuLine = 1;
};
void handle_8(void)
{
    OSD_background();
    OSD_c1(icon4, P0, blue_fill);
    OSD_c2(icon4, P0, yellow);
    OSD_c3(icon4, P0, blue_fill);
    selectedMenuLine = 2;
};
void handle_9(void)
{
    OSD_background();
    OSD_c1(icon4, P0, blue_fill);
    OSD_c2(icon4, P0, blue_fill);
    OSD_c3(icon4, P0, yellow);
    selectedMenuLine = 3;
};
void handle_a(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        // Check if scanlines are allowed for line 2
        if (!areScanLinesAllowed()) {
            A2_main0 = red;  // Disabled color
        } else {
            A2_main0 = main0;
        }
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        // Check if scanlines are allowed when selected
        if (!areScanLinesAllowed()) {
            A2_main0 = red;  // Disabled color
        } else {
            A2_main0 = yellowT;
        }
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        // Check if scanlines are allowed for line 2
        if (!areScanLinesAllowed()) {
            A2_main0 = red;  // Disabled color
        } else {
            A2_main0 = main0;
        }
        A3_main0 = yellowT;
    }

    colour1 = blue;

    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('2', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "ADC gain");
    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "Scanlines");
    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "Line filter");
};
void handle_b(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('3', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "Sharpness");
    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "Peaking");
    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "Step response");
};
void handle_c(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('4', _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "Default Color");
    // OSD_writeString(1, "Y gain");
    // colour1 = A2_main0;
    // number_stroca = stroca2;
    // OSD_writeString(1, "Color");
    // colour1 = A3_main0;
    // number_stroca = stroca3;
};
void handle_d(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    // number_stroca = stroca1;
    // __(icon5, _27);
    number_stroca = stroca2;
    __('1', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "R");
    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "G");
    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "B");
};
void handle_e(void)
{
    OSD_c1(0x3E, P9, main0);
    OSD_c1(0x3E, P10, main0);
    OSD_c1(0x3E, P11, main0);
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P22, main0);

    // Only show dashes for scanlines if they are available
    if (areScanLinesAllowed()) {
        OSD_c2(0x3E, P10, main0);
        OSD_c2(0x3E, P11, main0);
        OSD_c2(0x3E, P12, main0);
        OSD_c2(0x3E, P13, main0);
        OSD_c2(0x3E, P14, main0);
        OSD_c2(0x3E, P15, main0);
        OSD_c2(0x3E, P16, main0);
        OSD_c2(0x3E, P17, main0);
        OSD_c2(0x3E, P18, main0);
        OSD_c2(0x3E, P19, main0);
        OSD_c2(0x3E, P22, main0);

        if (uopt->wantScanlines) {
            OSD_c2(O, P23, main0);
            OSD_c2(N, P24, main0);
            OSD_c2(F, P25, blue_fill);
        } else {
            OSD_c2(O, P23, main0);
            OSD_c2(F, P24, main0);
            OSD_c2(F, P25, main0);
        }

        osdDisplayValue = uopt->scanlineStrength;
        if (osdDisplayValue == 0x00) {
            OSD_c2(n0, P21, main0);
            OSD_c2(n0, P20, main0);
        } else if (osdDisplayValue == 0x10) {
            OSD_c2(n0, P21, main0);
            OSD_c2(n1, P20, main0);
        } else if (osdDisplayValue == 0x20) {
            OSD_c2(n0, P21, main0);
            OSD_c2(n2, P20, main0);
        } else if (osdDisplayValue == 0x30) {
            OSD_c2(n0, P21, main0);
            OSD_c2(n3, P20, main0);
        } else if (osdDisplayValue == 0x40) {
            OSD_c2(n0, P21, main0);
            OSD_c2(n4, P20, main0);
        } else if (osdDisplayValue == 0x50) {
            OSD_c2(n0, P21, main0);
            OSD_c2(n5, P20, main0);
        }
    }

    OSD_c3(0x3E, P12, main0);
    OSD_c3(0x3E, P13, main0);
    OSD_c3(0x3E, P14, main0);
    OSD_c3(0x3E, P15, main0);
    OSD_c3(0x3E, P16, main0);
    OSD_c3(0x3E, P17, main0);
    OSD_c3(0x3E, P18, main0);
    OSD_c3(0x3E, P19, main0);
    OSD_c3(0x3E, P20, main0);
    OSD_c3(0x3E, P21, main0);
    OSD_c3(0x3E, P22, main0);

    if (uopt->wantVdsLineFilter) {
        OSD_c3(O, P23, main0);
        OSD_c3(N, P24, main0);
        OSD_c3(F, P25, blue_fill);
    } else {
        OSD_c3(O, P23, main0);
        OSD_c3(F, P24, main0);
        OSD_c3(F, P25, main0);
    }
    osdDisplayValue = GBS::ADC_RGCTRL::read();
    Type4(osdDisplayValue);

    if (uopt->enableAutoGain == 0) {
        OSD_c1(O, P23, main0);
        OSD_c1(F, P24, main0);
        OSD_c1(F, P25, main0);
    } else {
        OSD_c1(O, P23, main0);
        OSD_c1(N, P24, main0);
        OSD_c1(0x3E, P25, blue_fill);
    }
};
void handle_f(void)
{
    OSD_c1(0x3E, P10, main0);
    OSD_c1(0x3E, P11, main0);
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);
    OSD_c2(0x3E, P8, main0);
    OSD_c2(0x3E, P9, main0);
    OSD_c2(0x3E, P10, main0);
    OSD_c2(0x3E, P11, main0);
    OSD_c2(0x3E, P12, main0);
    OSD_c2(0x3E, P13, main0);
    OSD_c2(0x3E, P14, main0);
    OSD_c2(0x3E, P15, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);
    OSD_c2(0x3E, P19, main0);
    if (!isPeakingLocked()) {
        OSD_c2(0x3E, P20, main0);
        OSD_c2(0x3E, P21, main0);
        OSD_c2(0x3E, P22, main0);
    }
    OSD_c3(0x3E, P14, main0);
    OSD_c3(0x3E, P15, main0);
    OSD_c3(0x3E, P16, main0);
    OSD_c3(0x3E, P17, main0);
    OSD_c3(0x3E, P18, main0);
    OSD_c3(0x3E, P19, main0);
    OSD_c3(0x3E, P20, main0);
    OSD_c3(0x3E, P21, main0);
    OSD_c3(0x3E, P22, main0);

    if (GBS::VDS_PK_LB_GAIN::read() == 0x16) {
        OSD_c1(O, P23, main0);
        OSD_c1(F, P24, main0);
        OSD_c1(F, P25, main0);
    } else {
        OSD_c1(O, P23, main0);
        OSD_c1(N, P24, main0);
        OSD_c1(F, P25, blue_fill);
    }

    if (isPeakingLocked()) {
        // Locked state - overwrite dashes and ON/OFF with LOCKED
        OSD_c2(L, P20, main0);
        OSD_c2(O, P21, main0);
        OSD_c2(C, P22, main0);
        OSD_c2(K, P23, main0);
        OSD_c2(E, P24, main0);
        OSD_c2(D, P25, main0);
    } else {
        if (uopt->wantPeaking == 0) {
            OSD_c2(O, P23, main0);
            OSD_c2(F, P24, main0);
            OSD_c2(F, P25, main0);
        } else {
            OSD_c2(O, P23, main0);
            OSD_c2(N, P24, main0);
            OSD_c2(F, P25, blue_fill);
        }
    }

    if (uopt->wantStepResponse) {
        OSD_c3(O, P23, main0);
        OSD_c3(N, P24, main0);
        OSD_c3(F, P25, blue_fill);
    } else {
        OSD_c3(O, P23, main0);
        OSD_c3(F, P24, main0);
        OSD_c3(F, P25, main0);
    }
};
void handle_g(void)
{ // OSD_c1(0x3E, P2, main0);
    // OSD_c1(0x3E, P3, main0);
    // OSD_c1(0x3E, P4, main0);
    OSD_c1(0x3E, P5, main0);
    OSD_c1(0x3E, P6, main0);
    OSD_c1(0x3E, P7, main0);
    OSD_c1(0x3E, P8, main0);
    OSD_c1(0x3E, P9, main0);
    OSD_c1(0x3E, P10, main0);
    OSD_c1(0x3E, P11, main0);
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);

    // OSD_c2(0x3E, P2, main0);
    // OSD_c2(0x3E, P3, main0);
    // OSD_c2(0x3E, P4, main0);
    OSD_c2(0x3E, P5, main0);
    OSD_c2(0x3E, P6, main0);
    OSD_c2(0x3E, P7, main0);
    OSD_c2(0x3E, P8, main0);
    OSD_c2(0x3E, P9, main0);
    OSD_c2(0x3E, P10, main0);
    OSD_c2(0x3E, P11, main0);
    OSD_c2(0x3E, P12, main0);
    OSD_c2(0x3E, P13, main0);
    OSD_c2(0x3E, P14, main0);
    OSD_c2(0x3E, P15, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P20, main0);
    OSD_c2(0x3E, P21, main0);
    OSD_c2(0x3E, P22, main0);

    // OSD_c3(0x3E, P2, main0);
    // OSD_c3(0x3E, P3, main0);
    // OSD_c3(0x3E, P4, main0);
    OSD_c3(0x3E, P5, main0);
    OSD_c3(0x3E, P6, main0);
    OSD_c3(0x3E, P7, main0);
    OSD_c3(0x3E, P8, main0);
    OSD_c3(0x3E, P9, main0);
    OSD_c3(0x3E, P10, main0);
    OSD_c3(0x3E, P11, main0);
    OSD_c3(0x3E, P12, main0);
    OSD_c3(0x3E, P13, main0);
    OSD_c3(0x3E, P14, main0);
    OSD_c3(0x3E, P15, main0);
    OSD_c3(0x3E, P16, main0);
    OSD_c3(0x3E, P17, main0);
    OSD_c3(0x3E, P18, main0);
    OSD_c3(0x3E, P19, main0);
    OSD_c3(0x3E, P20, main0);
    OSD_c3(0x3E, P21, main0);
    OSD_c3(0x3E, P22, main0);

    // osdDisplayValue = (128 + GBS::VDS_Y_OFST::read());  //R
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() + 1.402 * ((signed char)GBS::VDS_V_OFST::read()-128));  //R
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() + 1.5 * ((signed char)GBS::VDS_V_OFST::read()));  //R
    // osdDisplayValue= (signed char)GBS::VDS_Y_OFST::read()+1.402*((signed char)GBS::VDS_V_OFST::read()-128);
    // osdDisplayValue = R_VAL;
    colour1 = main0;
    number_stroca = stroca1;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    // Typ(((signed char)((signed char)GBS::VDS_Y_OFST::read()) +(float)( 1.402     * (signed char)((signed char)GBS::VDS_V_OFST::read()) )) + 128);
    Typ(R_VAL);
    // osdDisplayValue = (128 + GBS::VDS_U_OFST::read());  //G
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() - 0.88 * ((signed char)GBS::VDS_U_OFST::read()) - 0.764 * ((signed char)GBS::VDS_V_OFST::read()));  //G
    // osdDisplayValue = (signed char)GBS::VDS_Y_OFST::read()-0.344136*((signed char)GBS::VDS_U_OFST::read()-128)-0.714136*((signed char)GBS::VDS_V_OFST::read()-128);
    // osdDisplayValue = G_VAL;
    colour1 = main0;
    number_stroca = stroca2;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    // Typ(((signed char)((signed char)GBS::VDS_Y_OFST::read()) -(float)( 0.344136  * (signed char)((signed char)GBS::VDS_U_OFST::read()) )- 0.714136 * (signed char)((signed char)GBS::VDS_V_OFST::read()) ) + 128);
    Typ(G_VAL);

    // osdDisplayValue = (128 + GBS::VDS_V_OFST::read());  //B
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() + 2 * ((signed char)GBS::VDS_U_OFST::read()));  //B
    // osdDisplayValue = (signed char)GBS::VDS_Y_OFST::read()+1.772*((signed char)GBS::VDS_U_OFST::read()-128);
    // osdDisplayValue = B_VAL;
    colour1 = main0;
    number_stroca = stroca3;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    // Typ(((signed char)((signed char)GBS::VDS_Y_OFST::read()) +(float)( 1.772     * (signed char)((signed char)GBS::VDS_U_OFST::read()) )) + 128);
    Typ(B_VAL);
};
void handle_h(void)
{
    OSD_c1(0x3E, P7, main0);
    OSD_c1(0x3E, P8, main0);
    OSD_c1(0x3E, P9, main0);
    OSD_c1(0x3E, P10, main0);
    OSD_c1(0x3E, P11, main0);
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);
    OSD_c2(0x3E, P6, main0);
    OSD_c2(0x3E, P7, main0);
    OSD_c2(0x3E, P8, main0);
    OSD_c2(0x3E, P9, main0);
    OSD_c2(0x3E, P10, main0);
    OSD_c2(0x3E, P11, main0);
    OSD_c2(0x3E, P12, main0);
    OSD_c2(0x3E, P13, main0);
    OSD_c2(0x3E, P14, main0);
    OSD_c2(0x3E, P15, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P20, main0);
    OSD_c2(0x3E, P21, main0);
    OSD_c2(0x3E, P22, main0);

    osdDisplayValue = GBS::VDS_Y_GAIN::read();
    colour1 = main0;
    number_stroca = stroca1;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    Typ(osdDisplayValue);
    osdDisplayValue = GBS::VDS_VCOS_GAIN::read();
    colour1 = main0;
    number_stroca = stroca2;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    Typ(osdDisplayValue);
};
void handle_i(void)
{
    if (selectedMenuLine == 1) {
        if ((inputType != InputTypeSV) && (inputType != InputTypeAV)) {
            A1_yellow = red;  // Disabled color
        } else {
            A1_yellow = yellowT;
        }

        A2_main0 = main0;
        A3_main0 = main0;

        OSD_c1(0x15, P21, yellowT);
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;

        OSD_c1(0x15, P21, blue_fill);
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;

        OSD_c1(0x15, P21, blue_fill);
    }

    colour1 = blue;
    // number_stroca = stroca1;
    // __(icon5, _27);
    number_stroca = stroca2;
    __('1', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "SV/AV Input Settings");

    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "Compatibility Mode");

    colour1 = A3_main0;
    number_stroca = stroca3;
    // OSD_writeString(1, "Lowres:use upscaling");
    OSD_writeString(1, "Matched presets");
};
void handle_j(void)
{
    // OSD_c2(0x3E, P16, main0);
    // OSD_c2(0x3E, P17, main0);
    // OSD_c2(0x3E, P18, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P20, main0);
    OSD_c2(0x3E, P21, main0);
    OSD_c2(0x3E, P22, main0);
    if (rgbComponentMode == 1) {
        OSD_c2(O, P23, main0);
        OSD_c2(N, P24, main0);
        OSD_c2(F, P25, blue_fill);
    } else {
        OSD_c2(O, P23, main0);
        OSD_c2(F, P24, main0);
        OSD_c2(F, P25, main0);
    }
    OSD_c3(0x3E, P16, main0);
    OSD_c3(0x3E, P17, main0);
    OSD_c3(0x3E, P18, main0);
    OSD_c3(0x3E, P19, main0);
    OSD_c3(0x3E, P20, main0);
    OSD_c3(0x3E, P21, main0);
    OSD_c3(0x3E, P22, main0);
    if (uopt->matchPresetSource) {
        OSD_c3(O, P23, main0);
        OSD_c3(N, P24, main0);     // ON
        OSD_c3(F, P25, blue_fill); // ON
    } else {
        OSD_c3(O, P23, main0);
        OSD_c3(F, P24, main0);
        OSD_c3(F, P25, main0); // OFF
    }
    /*
    upscaling
        // OSD_c3(0x3E, P21, main0);
        // OSD_c3(0x3E, P22, main0);
        // if (uopt->preferScalingRgbhv)
        // {
        //   OSD_c3(O, P23, main0);
        //   OSD_c3(N, P24, main0);
        //   OSD_c3(F, P25, blue_fill);
        // }
        // else
        // {
        //   OSD_c3(O, P23, main0);
        //   OSD_c3(F, P24, main0);
        //   OSD_c3(F, P25, main0);
        // }
    */
};
void handle_k(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('2', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "Deinterlace");
    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "Force:50Hz to 60Hz");
    colour1 = A3_main0;
    number_stroca = stroca3;
    // OSD_writeString(1, "Clock generator");
    OSD_writeString(1, "Lock method");
};
void handle_l(void)
{
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    if (uopt->deintMode == 0) {
        OSD_c1(A, P18, main0);
        OSD_c1(d, P19, main0);
        OSD_c1(a, P20, main0);
        OSD_c1(p, P21, main0);
        OSD_c1(t, P22, main0);
        OSD_c1(i, P23, main0);
        OSD_c1(v, P24, main0);
        OSD_c1(e, P25, main0);
    } else {
        OSD_c1(0x3E, P18, main0);
        OSD_c1(0x3E, P19, main0);
        OSD_c1(0x3E, P20, main0);
        OSD_c1(0x3E, P21, main0);
        OSD_c1(0x3E, P22, main0);
        OSD_c1(B, P23, main0);
        OSD_c1(o, P24, main0);
        OSD_c1(b, P25, main0);
    }

    // OSD_c1(0x3E, P21, main0);
    // OSD_c1(0x3E, P22, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P20, main0);
    OSD_c2(0x3E, P21, main0);
    OSD_c2(0x3E, P22, main0);
    // OSD_c3(0x3E, P16, main0);
    // OSD_c3(0x3E, P17, main0);
    // OSD_c3(0x3E, P18, main0);
    // OSD_c3(0x3E, P19, main0);
    // OSD_c3(0x3E, P20, main0);
    // OSD_c3(0x3E, P21, main0);
    // OSD_c3(0x3E, P22, main0);
    OSD_c3(0x3E, P12, main0);
    OSD_c3(0x3E, P13, main0);

    // if (uopt->wantOutputComponent)
    // {
    //     OSD_c1(O, P23, main0);
    //     OSD_c1(N, P24, main0);
    //     OSD_c1(F, P25, blue_fill);
    // }
    // else
    // {
    //     OSD_c1(O, P23, main0);
    //     OSD_c1(F, P24, main0);
    //     OSD_c1(F, P25, main0);
    // }

    if (uopt->PalForce60) {
        OSD_c2(O, P23, main0);
        OSD_c2(N, P24, main0);
        OSD_c2(F, P25, blue_fill);
    } else {
        OSD_c2(O, P23, main0);
        OSD_c2(F, P24, main0);
        OSD_c2(F, P25, main0);
    }

    // if (uopt->disableExternalClockGenerator)
    // {
    //   OSD_c3(O, P23, main0);
    //   OSD_c3(F, P24, main0);
    //   OSD_c3(F, P25, main0);
    // }
    // else
    // {
    //   OSD_c3(O, P23, main0);
    //   OSD_c3(N, P24, main0);
    //   OSD_c3(F, P25, blue_fill);
    // }

    if (uopt->frameTimeLockMethod == 0) {
        OSD_c3(n0, P14, main0);
        OSD_c3(V, P15, main0);
        OSD_c3(t, P16, main0);
        OSD_c3(o, P17, main0);
        OSD_c3(t, P18, main0);
        OSD_c3(a, P19, main0);
        OSD_c3(l, P20, main0);
        OSD_c3(0x3C, P21, main0);
        OSD_c3(V, P22, main0);
        OSD_c3(S, P23, main0);
        OSD_c3(S, P24, main0);
        OSD_c3(T, P25, main0);
    } else {
        OSD_c3(n1, P14, main0);
        OSD_c3(V, P15, main0);
        OSD_c3(t, P16, main0);
        OSD_c3(o, P17, main0);
        OSD_c3(t, P18, main0);
        OSD_c3(a, P19, main0);
        OSD_c3(l, P20, main0);
        OSD_c3(o, P22, main0);
        OSD_c3(n, P23, main0);
        OSD_c3(l, P24, main0);
        OSD_c3(y, P25, main0);
        OSD_c3(F, P21, blue_fill);
    }
};
void handle_m(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('3', _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "ADC calibration");
    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "Frame Time lock");
    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "EnableFrameTimeLock");
};
void handle_n(void)
{
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P20, main0);
    OSD_c2(0x3E, P21, main0);
    OSD_c2(0x3E, P22, main0);
    // OSD_c3(0x3E, P16, main0);
    // OSD_c3(0x3E, P17, main0);
    // OSD_c3(0x3E, P18, main0);
    // OSD_c3(0x3E, P19, main0);
    OSD_c3(0x3E, P20, main0);
    OSD_c3(0x3E, P21, main0);
    OSD_c3(0x3E, P22, main0);

    if (uopt->enableCalibrationADC) {
        OSD_c1(O, P23, main0);
        OSD_c1(N, P24, main0);
        OSD_c1(F, P25, blue_fill);
    } else {
        OSD_c1(O, P23, main0);
        OSD_c1(F, P24, main0);
        OSD_c1(F, P25, main0);
    }

    if (uopt->enableFrameTimeLock) {
        OSD_c2(O, P23, main0);
        OSD_c2(N, P24, main0);
        OSD_c2(F, P25, blue_fill);
    } else {
        OSD_c2(O, P23, main0);
        OSD_c2(F, P24, main0);
        OSD_c2(F, P25, main0);
    }

    if (uopt->disableExternalClockGenerator) {
        OSD_c3(O, P23, main0);
        OSD_c3(F, P24, main0);
        OSD_c3(F, P25, main0);
    } else {
        OSD_c3(O, P23, main0);
        OSD_c3(N, P24, main0);
        OSD_c3(F, P25, blue_fill);
    }
};
void handle_o(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('2', _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "Full height");

    // colour1 = A2_main0;
    // number_stroca = stroca2;
    // __(M, _1), __(a, _2), __(t, _3), __(c, _4), __(h, _5), __(e, _6), __(d, _7), __(p, _9), __(r, _10), __(e, _11), __(s, _12), __(e, _13), __(t, _14), __(s, _15);
};
void handle_p(void)
{
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);
    // OSD_c3(0x3E, P22, main0);

    if (uopt->wantFullHeight) {
        OSD_c1(O, P23, main0);
        OSD_c1(N, P24, main0);
        OSD_c1(F, P25, blue_fill);
    } else {
        OSD_c1(O, P23, main0);
        OSD_c1(F, P24, main0);
        OSD_c1(F, P25, main0);
    }
};
void handle_q(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __(I, _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "MEM left/right");
    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "HS left/right");
    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "HTotal");
};
void handle_r(void)
{
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);
    OSD_c1(0x03, P23, yellow);
    OSD_c1(0x13, P24, yellow);
    OSD_c2(0x3E, P14, main0);
    OSD_c2(0x3E, P15, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P20, main0);
    OSD_c2(0x3E, P21, main0);
    OSD_c2(0x3E, P22, main0);
    OSD_c2(0x03, P23, yellow);
    OSD_c2(0x13, P24, yellow);
    OSD_c3(0x3E, P7, main0);
    OSD_c3(0x3E, P8, main0);
    OSD_c3(0x3E, P9, main0);
    OSD_c3(0x3E, P10, main0);
    OSD_c3(0x3E, P11, main0);
    OSD_c3(0x3E, P12, main0);
    OSD_c3(0x3E, P13, main0);
    OSD_c3(0x3E, P14, main0);
    OSD_c3(0x3E, P15, main0);
    OSD_c3(0x3E, P16, main0);
    OSD_c3(0x3E, P17, main0);
    OSD_c3(0x3E, P18, main0);
    OSD_c3(0x3E, P19, main0);
    OSD_c3(0x3E, P20, main0);
    OSD_c3(0x3E, P21, main0);
    OSD_c3(0x3E, P22, main0);
    osdDisplayValue = GBS::VDS_HSYNC_RST::read();
    colour1 = main0;
    number_stroca = stroca3;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    Typ(osdDisplayValue);
};
void handle_s(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __(I, _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "Debug view");
    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "ADC filter");
    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "Freeze capture");
};
void handle_t(void)
{
    OSD_c1(0x3E, P11, main0);
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);
    OSD_c2(0x3E, P11, main0);
    OSD_c2(0x3E, P12, main0);
    OSD_c2(0x3E, P13, main0);
    OSD_c2(0x3E, P14, main0);
    OSD_c2(0x3E, P15, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P20, main0);
    OSD_c2(0x3E, P21, main0);
    OSD_c2(0x3E, P22, main0);
    colour1 = main0;
    number_stroca = stroca3;
    __(0x3E, _15), __(0x3E, _16), __(0x3E, _17), __(0x3E, _18), __(0x3E, _19), __(0x3E, _20), __(0x3E, _21), __(0x3E, _22);

    if (GBS::ADC_UNUSED_62::read() == 0x00) {
        OSD_c1(O, P23, main0);
        OSD_c1(F, P24, main0);
        OSD_c1(F, P25, main0);
    } else {
        OSD_c1(O, P23, main0);
        OSD_c1(N, P24, main0);
        OSD_c1(F, P25, blue_fill);
    }

    if (GBS::ADC_FLTR::read() > 0) {
        OSD_c2(O, P23, main0);
        OSD_c2(N, P24, main0);
        OSD_c2(F, P25, blue_fill);
    } else {
        OSD_c2(O, P23, main0);
        OSD_c2(F, P24, main0);
        OSD_c2(F, P25, main0);
    }

    if (GBS::CAPTURE_ENABLE::read() > 0) {
        OSD_c3(O, P23, main0);
        OSD_c3(F, P24, main0);
        OSD_c3(F, P25, main0);
    } else {
        OSD_c3(O, P23, main0);
        OSD_c3(N, P24, main0);
        OSD_c3(F, P25, blue_fill);
    }
};
void handle_u(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __(I, _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "Enable OTA");
    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "Restart");
    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "Reset defaults");
};
void handle_v(void)
{
    OSD_c1(0x3E, P11, main0);
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);

    if (rto->allowUpdatesOTA) {
        OSD_c1(O, P23, main0);
        OSD_c1(N, P24, main0);
        OSD_c1(F, P25, blue_fill);
    } else {
        OSD_c1(O, P23, main0);
        OSD_c1(F, P24, main0);
        OSD_c1(F, P25, main0);
    }
};
void handle_w(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    }
    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "Loadprofile:");

    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "Saveprofile:");

    colour1 = yellowT;
    number_stroca = stroca3;
    OSD_writeString(1, "Active save:");
};
void handle_x(void)
{
    if (oled_menuItem == OLED_Profile) {
        colour1 = main0;
        number_stroca = stroca1;
        sending1();
        nameP();
    } else if (oled_menuItem == OLED_Profile_SaveConfirm) {
        colour1 = main0;
        number_stroca = stroca1;
        sending2();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Save) {
        colour1 = main0;
        number_stroca = stroca1;
        sending3();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Load) {
        colour1 = main0;
        number_stroca = stroca1;
        sending4();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Operation1) {
        colour1 = main0;
        number_stroca = stroca1;
        sending5();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Operation2) {
        colour1 = main0;
        number_stroca = stroca1;
        sending6();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Operation3) {
        colour1 = main0;
        number_stroca = stroca1;
        sending7();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot7) {
        colour1 = main0;
        number_stroca = stroca1;
        sending8();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot8) {
        colour1 = main0;
        number_stroca = stroca1;
        sending9();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot9) {
        colour1 = main0;
        number_stroca = stroca1;
        sending10();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot10) {
        colour1 = main0;
        number_stroca = stroca1;
        sending11();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot11) {
        colour1 = main0;
        number_stroca = stroca1;
        sending12();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot12) {
        colour1 = main0;
        number_stroca = stroca1;
        sending13();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot13) {
        colour1 = main0;
        number_stroca = stroca1;
        sending14();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot14) {
        colour1 = main0;
        number_stroca = stroca1;
        sending15();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot15) {
        colour1 = main0;
        number_stroca = stroca1;
        sending16();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot16) {
        colour1 = main0;
        number_stroca = stroca1;
        sending17();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot17) {
        colour1 = main0;
        number_stroca = stroca1;
        sending18();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot18) {
        colour1 = main0;
        number_stroca = stroca1;
        sending19();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot19) {
        colour1 = main0;
        number_stroca = stroca1;
        sending20();
        nameP();
    }

    if (uopt->presetSlot == 'A') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending1a();
        nameP();
    } else if (uopt->presetSlot == 'B') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending2a();
        nameP();
    } else if (uopt->presetSlot == 'C') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending3a();
        nameP();
    } else if (uopt->presetSlot == 'D') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending4a();
        nameP();
    } else if (uopt->presetSlot == 'E') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending5a();
        nameP();
    } else if (uopt->presetSlot == 'F') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending6a();
        nameP();
    } else if (uopt->presetSlot == 'G') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending7a();
        nameP();
    } else if (uopt->presetSlot == 'H') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending8a();
        nameP();
    } else if (uopt->presetSlot == 'I') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending9a();
        nameP();
    } else if (uopt->presetSlot == 'J') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending10a();
        nameP();
    } else if (uopt->presetSlot == 'K') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending11a();
        nameP();
    } else if (uopt->presetSlot == 'L') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending12a();
        nameP();
    } else if (uopt->presetSlot == 'M') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending13a();
        nameP();
    } else if (uopt->presetSlot == 'N') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending14a();
        nameP();
    } else if (uopt->presetSlot == 'O') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending15a();
        nameP();
    } else if (uopt->presetSlot == 'P') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending16a();
        nameP();
    } else if (uopt->presetSlot == 'Q') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending17a();
        nameP();
    } else if (uopt->presetSlot == 'R') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending18a();
        nameP();
    } else if (uopt->presetSlot == 'S') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending19a();
        nameP();
    } else if (uopt->presetSlot == 'T') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending20a();
        nameP();
    }

    if (oled_menuItem == OLED_Profile_SelectSlot) {
        colour1 = main0;
        number_stroca = stroca2;
        sending1b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot1) {
        colour1 = main0;
        number_stroca = stroca2;
        sending2b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot2) {
        colour1 = main0;
        number_stroca = stroca2;
        sending3b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot3) {
        colour1 = main0;
        number_stroca = stroca2;
        sending4b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot4) {
        colour1 = main0;
        number_stroca = stroca2;
        sending5b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot5) {
        colour1 = main0;
        number_stroca = stroca2;
        sending6b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Slot6) {
        colour1 = main0;
        number_stroca = stroca2;
        sending7b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_SelectPreset) {
        colour1 = main0;
        number_stroca = stroca2;
        sending8b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Preset1) {
        colour1 = main0;
        number_stroca = stroca2;
        sending9b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Preset2) {
        colour1 = main0;
        number_stroca = stroca2;
        sending10b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Preset3) {
        colour1 = main0;
        number_stroca = stroca2;
        sending11b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Preset4) {
        colour1 = main0;
        number_stroca = stroca2;
        sending12b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Preset5) {
        colour1 = main0;
        number_stroca = stroca2;
        sending13b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Preset6) {
        colour1 = main0;
        number_stroca = stroca2;
        sending14b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Preset7) {
        colour1 = main0;
        number_stroca = stroca2;
        sending15b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Preset8) {
        colour1 = main0;
        number_stroca = stroca2;
        sending16b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Preset9) {
        colour1 = main0;
        number_stroca = stroca2;
        sending17b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Preset10) {
        colour1 = main0;
        number_stroca = stroca2;
        sending18b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Preset11) {
        colour1 = main0;
        number_stroca = stroca2;
        sending19b();
        nameP();
    } else if (oled_menuItem == OLED_Profile_Preset12) {
        colour1 = main0;
        number_stroca = stroca2;
        sending20b();
        nameP();
    }
};
void handle_y(void)
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

void handle_z(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "Contrast");
    // OSD_writeString(1, "Saturation");

    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "Saturation");

    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "Default");
}

void handle_A(void)
{


    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);


    colour1 = main0;
    number_stroca = stroca1;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    Typ(contrast);


    OSD_c2(0x3E, P13, main0);
    OSD_c2(0x3E, P14, main0);
    OSD_c2(0x3E, P15, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);


    colour1 = main0;
    number_stroca = stroca2;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    Typ(saturation);
};


void handle_caret(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        if (!lineOption)
            A2_main0 = red;  // Disabled color
        else
            A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    // colour1 = blue;
    // number_stroca = stroca1;
    // __(icon5, _27);
    // number_stroca = stroca2;
    // __(I, _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "DoubleLine");
    // OSD_writeString(1, "Smooth");
    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "Smooth");
    // OSD_writeString(1, "Bright");

    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "Bright");
    // OSD_writeString(1, "Contrast");
};
void handle_at(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    // number_stroca = stroca1;
    // __(icon5, _27);
    number_stroca = stroca2;
    __('1', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "RGBs");
    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "RGsB");
    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "VGA");
};
void handle_exclamation(void)
{
    A1_yellow = main0;
    A2_main0 = main0;
    A3_main0 = main0;

    colour1 = blue;
    // number_stroca = stroca1;
    // __(icon5, _27);
    // number_stroca = stroca2;
    // __('1', _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(0, "Whether to keep the settings");

    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(0, "Restore in ");

    if (keepSettings) {
        colour1 = yellowT;
        OSD_c3(0x15, P2, yellowT);
    } else {
        colour1 = A3_main0;
        OSD_c3(0x15, P2, blue_fill);
    }
    number_stroca = stroca3;
    OSD_writeString(3, "Changes");

    if (!keepSettings) {
        colour1 = yellowT;
        OSD_c3(0x15, P13, yellowT);
    } else {
        colour1 = A3_main0;
        OSD_c3(0x15, P13, blue_fill);
    }
    OSD_writeString(0xff, "    Recover");

    // OSD_c3(0x15, P2, blue_fill);
};
void handle_hash(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('2', _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "YPBPR");
    colour1 = A2_main0;
    number_stroca = stroca2;
    OSD_writeString(1, "SV");
    colour1 = A3_main0;
    number_stroca = stroca3;
    OSD_writeString(1, "AV");
};
void handle_dollar(void)
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
void handle_percent(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __(I, _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "Setting");
};
void handle_ampersand(void)
{

    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);

    OSD_c2(0x3E, P13, main0);
    OSD_c2(0x3E, P14, main0);
    OSD_c2(0x3E, P15, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);


    OSD_c3(0x3E, P13, main0);
    OSD_c3(0x3E, P14, main0);
    OSD_c3(0x3E, P15, main0);
    OSD_c3(0x3E, P16, main0);
    OSD_c3(0x3E, P17, main0);
    OSD_c3(0x3E, P18, main0);

    if (lineOption) {
        OSD_c1(n2, P23, main0);
        OSD_c1(X, P24, main0);
    } else {
        OSD_c1(n1, P23, main0);
        OSD_c1(X, P24, main0);
        smoothOption = false;
    }
    if (smoothOption) {
        OSD_c2(O, P23, main0);
        OSD_c2(N, P24, main0);
        OSD_c2(F, P25, blue_fill);
    } else {
        OSD_c2(O, P23, main0);
        OSD_c2(F, P24, main0);
        OSD_c2(F, P25, main0);
    }

    colour1 = main0;
    number_stroca = stroca3;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    Typ(brightness);


    // colour1 = main0;
    // number_stroca = stroca3;
    // sequence_number1 = _25;
    // sequence_number2 = _24;
    // sequence_number3 = _23;
    // Typ(contrast);
};
void handle_asterisk(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('4', _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    OSD_writeString(1, "Matched presets");
    // colour1 = A2_main0;
    // number_stroca = stroca2;
};
