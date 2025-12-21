// ====================================================================================
// menu-color.cpp
// IR Menu Handler for Color Settings
// ====================================================================================

#include "menu-common.h"
#include "../../tv5725.h"

// ====================================================================================
// External References
// ====================================================================================

typedef TV5725<GBS_ADDR> GBS;
extern struct userOptions *uopt;
extern char serialCommand;
extern char userCommand;
extern void saveUserPrefs();
extern boolean areScanLinesAllowed();

// ====================================================================================
// IR_handleColorSettings - Color Settings Menu
// ====================================================================================

bool IR_handleColorSettings()
{
    // OLED_ColorSettings_ADCGain
    if (oled_menuItem == OLED_ColorSettings_ADCGain) {
        showMenuToggle("Menu->Color", "ADC gain", uopt->enableAutoGain);

        if (results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    oled_menuItem = OLED_ColorSettings_RGB_B;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    oled_menuItem = OLED_ColorSettings_Scanlines;
                    break;
                case IR_KEY_RIGHT:
                    userCommand = 'n';
                    break;
                case IR_KEY_LEFT:
                    userCommand = 'o';
                    break;
                case IR_KEY_OK:
                    serialCommand = 'T';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_Scanlines
    else if (oled_menuItem == OLED_ColorSettings_Scanlines) {
        boolean scanlinesAllowed = areScanLinesAllowed();
        showMenuToggle("Menu->Color", "Scanlines", uopt->wantScanlines);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    oled_menuItem = OLED_ColorSettings_ADCGain;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    oled_menuItem = OLED_ColorSettings_LineFilter;
                    break;
                case IR_KEY_RIGHT:
                    if (scanlinesAllowed) {
                        userCommand = 'K';
                    }
                    break;
                case IR_KEY_LEFT:
                    if (scanlinesAllowed) {
                        userCommand = 'K';
                    }
                    break;
                case IR_KEY_OK:
                    if (scanlinesAllowed) {
                        userCommand = '7';
                    }
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_LineFilter
    else if (oled_menuItem == OLED_ColorSettings_LineFilter) {
        showMenuToggle("Menu->Color", "Line filter", uopt->wantVdsLineFilter);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    oled_menuItem = OLED_ColorSettings_Scanlines;
                    break;
                case IR_KEY_DOWN:
                    oled_menuItem = OLED_ColorSettings_Sharpness;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    break;
                case IR_KEY_OK:
                    userCommand = 'm';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_Sharpness
    else if (oled_menuItem == OLED_ColorSettings_Sharpness) {
        showMenuToggle("Menu->Color", "Sharpness", GBS::VDS_PK_LB_GAIN::read() != 0x16);

        if (results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    oled_menuItem = OLED_ColorSettings_LineFilter;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    oled_menuItem = OLED_ColorSettings_Peaking;
                    break;
                case IR_KEY_OK:
                    userCommand = 'W';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_Peaking
    else if (oled_menuItem == OLED_ColorSettings_Peaking) {
        if (isPeakingLocked()) {
            showMenuValue("Menu->Color", "Peaking", "LOCKED");
        } else {
            showMenuToggle("Menu->Color", "Peaking", uopt->wantPeaking);
        }

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    oled_menuItem = OLED_ColorSettings_Sharpness;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    oled_menuItem = OLED_ColorSettings_StepResponse;
                    break;
                case IR_KEY_OK:
                    if (!isPeakingLocked()) {
                        serialCommand = 'f';
                    }
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_StepResponse
    else if (oled_menuItem == OLED_ColorSettings_StepResponse) {
        showMenuToggle("Menu->Color", "Step response", uopt->wantStepResponse);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    oled_menuItem = OLED_ColorSettings_Peaking;
                    break;
                case IR_KEY_DOWN:
                    oled_menuItem = OLED_ColorSettings_DefaultColor;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
                    break;
                case IR_KEY_OK:
                    serialCommand = 'V';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_RGB_R
    else if (oled_menuItem == OLED_ColorSettings_RGB_R) {
        showMenu("Menu->Color", "R ");

        if (results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
        }
        OSD_handleCommand(OSD_CMD_COLOR_RGB_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    oled_menuItem = OLED_ColorSettings_RGB_G;
                    break;
                case IR_KEY_RIGHT:
                    R_VAL = MIN(R_VAL + STEP, 255);
                    applyRGBtoYUVConversion();
                    break;
                case IR_KEY_LEFT:
                    R_VAL = MAX(0, R_VAL - STEP);
                    applyRGBtoYUVConversion();
                    break;
                case IR_KEY_OK:
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_RGB_G
    else if (oled_menuItem == OLED_ColorSettings_RGB_G) {
        if (uopt->enableAutoGain == 1) {
            uopt->enableAutoGain = 0;
            saveUserPrefs();
        } else {
            uopt->enableAutoGain = 0;
        }
        showMenu("Menu->Color", "G ");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
        }
        OSD_handleCommand(OSD_CMD_COLOR_RGB_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    oled_menuItem = OLED_ColorSettings_RGB_R;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    oled_menuItem = OLED_ColorSettings_RGB_B;
                    break;
                case IR_KEY_RIGHT:
                    G_VAL = MIN(G_VAL + STEP, 255);
                    applyRGBtoYUVConversion();
                    break;
                case IR_KEY_LEFT:
                    G_VAL = MAX(0, G_VAL - STEP);
                    applyRGBtoYUVConversion();
                    break;
                case IR_KEY_OK:
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_RGB_B
    else if (oled_menuItem == OLED_ColorSettings_RGB_B) {
        showMenu("Menu->Color", "B");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
        }
        OSD_handleCommand(OSD_CMD_COLOR_RGB_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    oled_menuItem = OLED_ColorSettings_RGB_G;
                    break;
                case IR_KEY_DOWN:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    oled_menuItem = OLED_ColorSettings_ADCGain;
                    break;
                case IR_KEY_RIGHT:
                    B_VAL = MIN(B_VAL + STEP, 255);
                    applyRGBtoYUVConversion();
                    break;
                case IR_KEY_LEFT:
                    B_VAL = MAX(0, B_VAL - STEP);
                    applyRGBtoYUVConversion();
                    break;
                case IR_KEY_OK:
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_Y_Gain
    else if (oled_menuItem == OLED_ColorSettings_Y_Gain) {
        uint8_t cur = GBS::VDS_Y_GAIN::read();
        showMenu("Menu->Color", "Y gain");

        if (results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
        }

        OSD_handleCommand(OSD_CMD_SYS_SVINPUT_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    oled_menuItem = OLED_ColorSettings_RGB_B;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
                    oled_menuItem = OLED_ColorSettings_Color;
                    break;
                case IR_KEY_RIGHT:
                    cur = MIN(cur + STEP, 255);
                    GBS::VDS_Y_GAIN::write(cur);
                    break;
                case IR_KEY_LEFT:
                    cur = MAX(0, cur - STEP);
                    GBS::VDS_Y_GAIN::write(cur);
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_Color
    else if (oled_menuItem == OLED_ColorSettings_Color) {
        showMenu("Menu->Color", "Color");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
        }
        OSD_handleCommand(OSD_CMD_SYS_SVINPUT_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
                    oled_menuItem = OLED_ColorSettings_Y_Gain;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
                    oled_menuItem = OLED_ColorSettings_DefaultColor;
                    break;
                case IR_KEY_RIGHT:
                    userCommand = 'V';
                    break;
                case IR_KEY_LEFT:
                    userCommand = 'R';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_DefaultColor
    else if (oled_menuItem == OLED_ColorSettings_DefaultColor) {
        showMenu("Menu->Color", "Default Color");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    oled_menuItem = OLED_ColorSettings_StepResponse;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    break;
                case IR_KEY_OK:
                    userCommand = 'U';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}
