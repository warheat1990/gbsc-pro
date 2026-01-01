// ====================================================================================
// menu-adv.cpp
// IR Menu Handler for ADV7280 Settings (SV/AV Input)
// ====================================================================================

#include "../menu-core.h"
#include "../../drivers/ir_remote.h"

// ====================================================================================
// External References
// ====================================================================================

extern void saveUserPrefs();

// ====================================================================================
// IR_handleADVSettings - ADV7280 SV/AV Input Settings Menu
// ====================================================================================

bool IR_handleADVSettings()
{
    // OLED_SystemSettings_SVAVInput_DoubleLine (I2P)
    if (oled_menuItem == OLED_SystemSettings_SVAVInput_DoubleLine) {
        showMenuValue("M>Sys>SvAv Set", "I2P", advI2P ? "ON" : "OFF");
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_Default);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_Smooth);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    advI2P = !advI2P;
                    if(!advI2P) {
                        advSmooth = false;
                    }
                    ADV_sendI2P(advI2P);
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInputSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_Smooth
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Smooth) {
        showMenuToggle("M>Sys>SvAv Set", "Smooth", advSmooth);
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_DoubleLine);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_ACE);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    if (advI2P) {
                        advSmooth = !advSmooth;
                        ADV_sendSmooth(advSmooth);
                    }
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInputSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_ACE (Page 1, row 3)
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_ACE) {
        showMenuToggle("M>Sys>SvAv Set", "ACE", advACE);
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_Smooth);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_Bright);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    advACE = !advACE;
                    ADV_sendACE(advACE);
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInputSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_Bright (Page 2, row 1)
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Bright) {
        showMenu("M>Sys>SvAv Set", "Brightness");
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE2_VALUES);

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
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_ACE);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_Contrast);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        advBrightness = MIN(advBrightness + STEP, 254);
                        ADV_sendBCSH(0x0a, advBrightness - 128);
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        advBrightness = MAX(advBrightness - STEP, 0);
                        ADV_sendBCSH(0x0a, advBrightness - 128);
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInputSettings);
                        saveUserPrefs();
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

    // OLED_SystemSettings_SVAVInput_Contrast (Page 2, row 2)
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Contrast) {
        showMenu("M>Sys>SvAv Set", "Contrast");
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE2_VALUES);

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
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_Bright);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_Saturation);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        advContrast = MIN(advContrast + STEP, 254);
                        ADV_sendBCSH(0x08, advContrast);
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        advContrast = MAX(advContrast - STEP, 0);
                        ADV_sendBCSH(0x08, advContrast);
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInputSettings);
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

    // OLED_SystemSettings_SVAVInput_Saturation (Page 2, row 3)
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Saturation) {
        showMenu("M>Sys>SvAv Set", "Saturation");
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE2_VALUES);

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
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_Contrast);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_Default);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        advSaturation = MIN(advSaturation + STEP, 254);
                        ADV_sendBCSH(0xe3, advSaturation);
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        advSaturation = MAX(advSaturation - STEP, 0);
                        ADV_sendBCSH(0xe3, advSaturation);
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInputSettings);
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

    // OLED_SystemSettings_SVAVInput_Default (Page 3, row 1)
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Default) {
        showMenu("M>Sys>SvAv Set", "Default");
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE3_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_Saturation);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_DoubleLine);
                    break;
                case IR_KEY_OK:
                    ADV_sendBCSH('D', 'E');
                    advBrightness = 128;
                    advContrast = 128;
                    advSaturation = 128;
                    advACE = 0;
                    ADV_sendACE(false);
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInputSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}
