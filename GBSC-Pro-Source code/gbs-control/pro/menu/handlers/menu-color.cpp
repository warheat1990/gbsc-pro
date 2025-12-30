// ====================================================================================
// menu-color.cpp
// IR Menu Handler for Color Settings
// ====================================================================================

#include "../menu-core.h"
#include "../../../tv5725.h"

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
    // OLED_ColorSettings_RGB_R
    if (oled_menuItem == OLED_ColorSettings_RGB_R) {
        showMenu("Menu->Color", "R ");
        OSD_handleCommand(OSD_CMD_COLOR_PAGE1_VALUES);

        if (irDecode()) {
            uint32_t key = IR_getKeyRepeat();
            if (key) {
                switch (key) {
                    case IR_KEY_MENU:
                        IR_clearRepeatKey();
                        exitMenu();
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings_RGB_G);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        gbsColorR = MIN(gbsColorR + STEP, 255);
                        applyRGBtoYUVConversion();
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        gbsColorR = MAX(0, gbsColorR - STEP);
                        applyRGBtoYUVConversion();
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings);
                        break;
                    default:
                        IR_clearRepeatKey();
                        break;
                }
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_RGB_G
    else if (oled_menuItem == OLED_ColorSettings_RGB_G) {
        showMenu("Menu->Color", "G ");
        OSD_handleCommand(OSD_CMD_COLOR_PAGE1_VALUES);

        if (irDecode()) {
            uint32_t key = IR_getKeyRepeat();
            if (key) {
                switch (key) {
                    case IR_KEY_MENU:
                        IR_clearRepeatKey();
                        exitMenu();
                        break;
                    case IR_KEY_UP:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings_RGB_R);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings_RGB_B);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        gbsColorG = MIN(gbsColorG + STEP, 255);
                        applyRGBtoYUVConversion();
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        gbsColorG = MAX(0, gbsColorG - STEP);
                        applyRGBtoYUVConversion();
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings);
                        break;
                    default:
                        IR_clearRepeatKey();
                        break;
                }
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_RGB_B
    else if (oled_menuItem == OLED_ColorSettings_RGB_B) {
        showMenu("Menu->Color", "B");
        OSD_handleCommand(OSD_CMD_COLOR_PAGE1_VALUES);

        if (irDecode()) {
            uint32_t key = IR_getKeyRepeat();
            if (key) {
                switch (key) {
                    case IR_KEY_MENU:
                        IR_clearRepeatKey();
                        exitMenu();
                        break;
                    case IR_KEY_UP:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings_RGB_G);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings_ADCGain);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        gbsColorB = MIN(gbsColorB + STEP, 255);
                        applyRGBtoYUVConversion();
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        gbsColorB = MAX(0, gbsColorB - STEP);
                        applyRGBtoYUVConversion();
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings);
                        break;
                    default:
                        IR_clearRepeatKey();
                        break;
                }
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_ADCGain
    else if (oled_menuItem == OLED_ColorSettings_ADCGain) {
        showMenuToggle("Menu->Color", "ADC gain", uopt->enableAutoGain);
        OSD_handleCommand(OSD_CMD_COLOR_PAGE2_VALUES);

        if (irDecode()) {
            uint32_t key = IR_getKeyRepeat();
            if (key) {
                switch (key) {
                    case IR_KEY_MENU:
                        IR_clearRepeatKey();
                        exitMenu();
                        break;
                    case IR_KEY_UP:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings_RGB_B);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings_Scanlines);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        userCommand = 'n';
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        userCommand = 'o';
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        serialCommand = 'T';
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings);
                        break;
                    default:
                        IR_clearRepeatKey();
                        break;
                }
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_Scanlines
    else if (oled_menuItem == OLED_ColorSettings_Scanlines) {
        showMenuToggle("Menu->Color", "Scanlines", uopt->wantScanlines);
        OSD_handleCommand(OSD_CMD_COLOR_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_ColorSettings_ADCGain);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_ColorSettings_LineFilter);
                    break;
                case IR_KEY_RIGHT:
                    if (areScanLinesAllowed()) {
                        userCommand = 'K';
                    }
                    break;
                case IR_KEY_LEFT:
                    if (areScanLinesAllowed()) {
                        userCommand = 'K';
                    }
                    break;
                case IR_KEY_OK:
                    if (areScanLinesAllowed()) {
                        userCommand = '7';
                    }
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_ColorSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_LineFilter
    else if (oled_menuItem == OLED_ColorSettings_LineFilter) {
        showMenuToggle("Menu->Color", "Line filter", uopt->wantVdsLineFilter);
        OSD_handleCommand(OSD_CMD_COLOR_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_ColorSettings_Scanlines);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_ColorSettings_Sharpness);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    userCommand = 'm';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_ColorSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_Sharpness
    else if (oled_menuItem == OLED_ColorSettings_Sharpness) {
        showMenuToggle("Menu->Color", "Sharpness", GBS::VDS_PK_LB_GAIN::read() != 0x16);
        OSD_handleCommand(OSD_CMD_COLOR_PAGE3_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_ColorSettings_LineFilter);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_ColorSettings_Peaking);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    userCommand = 'W';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_ColorSettings);
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

        OSD_handleCommand(OSD_CMD_COLOR_PAGE3_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_ColorSettings_Sharpness);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_ColorSettings_StepResponse);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    if (!isPeakingLocked()) {
                        serialCommand = 'f';
                    }
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_ColorSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_StepResponse
    else if (oled_menuItem == OLED_ColorSettings_StepResponse) {
        showMenuToggle("Menu->Color", "Step response", uopt->wantStepResponse);
        OSD_handleCommand(OSD_CMD_COLOR_PAGE3_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_ColorSettings_Peaking);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_ColorSettings_Y_Gain);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    serialCommand = 'V';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_ColorSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_Y_Gain
    else if (oled_menuItem == OLED_ColorSettings_Y_Gain) {
        showMenu("Menu->Color", "Y gain");
        OSD_handleCommand(OSD_CMD_COLOR_PAGE4_VALUES);
        uint8_t cur = GBS::VDS_Y_GAIN::read();

        if (irDecode()) {
            uint32_t key = IR_getKeyRepeat();
            if (key) {
                switch (key) {
                    case IR_KEY_MENU:
                        IR_clearRepeatKey();
                        exitMenu();
                        break;
                    case IR_KEY_UP:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings_StepResponse);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings_Color);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        cur = MIN(cur + STEP, 255);
                        GBS::VDS_Y_GAIN::write(cur);
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        cur = MAX(0, cur - STEP);
                        GBS::VDS_Y_GAIN::write(cur);
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings);
                        break;
                    default:
                        IR_clearRepeatKey();
                        break;
                }
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_Color
    else if (oled_menuItem == OLED_ColorSettings_Color) {
        showMenu("Menu->Color", "Color");
        OSD_handleCommand(OSD_CMD_COLOR_PAGE4_VALUES);

        if (irDecode()) {
            uint32_t key = IR_getKeyRepeat();
            if (key) {
                switch (key) {
                    case IR_KEY_MENU:
                        IR_clearRepeatKey();
                        exitMenu();
                        break;
                    case IR_KEY_UP:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings_Y_Gain);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings_DefaultColor);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        userCommand = 'V';
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        userCommand = 'R';
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ColorSettings);
                        break;
                    default:
                        IR_clearRepeatKey();
                        break;
                }
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_DefaultColor
    else if (oled_menuItem == OLED_ColorSettings_DefaultColor) {
        showMenu("Menu->Color", "Default Color");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_ColorSettings_Color);
                    break;
                case IR_KEY_OK:
                    userCommand = 'U';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_ColorSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}
