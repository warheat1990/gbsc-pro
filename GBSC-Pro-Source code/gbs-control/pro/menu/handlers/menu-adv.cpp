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
        showMenuValue("M>Sys>SvAv Set", "I2P", uopt->advI2P ? "ON" : "OFF");
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
                    uopt->advI2P = !uopt->advI2P;
                    if(!uopt->advI2P) {
                        uopt->advSmooth = false;
                    }
                    ADV_sendI2P(uopt->advI2P);
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

    // OLED_SystemSettings_SVAVInput_Smooth
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Smooth) {
        showMenuToggle("M>Sys>SvAv Set", "Smooth", uopt->advSmooth);
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
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_ACESettings);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    if (uopt->advI2P) {
                        uopt->advSmooth = !uopt->advSmooth;
                        ADV_sendSmooth(uopt->advSmooth);
                        saveUserPrefs();
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

    // OLED_SystemSettings_SVAVInput_ACESettings (Page 1, row 3) - Link to ACE submenu
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_ACESettings) {
        showMenu("M>Sys>SvAv Set", "ACE Settings >");
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
                case IR_KEY_OK:
                    // Enter ACE Settings submenu
                    Menu_navigateTo(OLED_ACESettings_Enable);
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
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_ACESettings);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_Contrast);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        uopt->advBrightness = MIN(uopt->advBrightness + STEP, 254);
                        ADV_sendBCSH(0x0a, uopt->advBrightness - 128);
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        uopt->advBrightness = MAX(uopt->advBrightness - STEP, 0);
                        ADV_sendBCSH(0x0a, uopt->advBrightness - 128);
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
                        uopt->advContrast = MIN(uopt->advContrast + STEP, 254);
                        ADV_sendBCSH(0x08, uopt->advContrast);
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        uopt->advContrast = MAX(uopt->advContrast - STEP, 0);
                        ADV_sendBCSH(0x08, uopt->advContrast);
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
                        uopt->advSaturation = MIN(uopt->advSaturation + STEP, 254);
                        ADV_sendBCSH(0xe3, uopt->advSaturation);
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        uopt->advSaturation = MAX(uopt->advSaturation - STEP, 0);
                        ADV_sendBCSH(0xe3, uopt->advSaturation);
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
                    uopt->advBrightness = 128;
                    uopt->advContrast = 128;
                    uopt->advSaturation = 128;
                    uopt->advACE = 0;
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

// ====================================================================================
// IR_handleACESettings - ACE Settings Submenu (inside SV/AV Settings)
// ====================================================================================

bool IR_handleACESettings()
{
    // OLED_ACESettings_Enable (Page 1, row 1)
    if (oled_menuItem == OLED_ACESettings_Enable) {
        showMenuToggle("M>ACE Settings", "Enable", uopt->advACE);
        OSD_handleCommand(OSD_CMD_ACE_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_ACESettings_Default);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_ACESettings_LumaGain);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    uopt->advACE = !uopt->advACE;
                    ADV_sendACE(uopt->advACE);
                    saveUserPrefs();
                    OSD_handleCommand(OSD_CMD_ACE_PAGE1);
                    OSD_handleCommand(OSD_CMD_ACE_PAGE1_VALUES);
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_ACESettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ACESettings_LumaGain (Page 1, row 2)
    else if (oled_menuItem == OLED_ACESettings_LumaGain) {
        showMenu("M>ACE Settings", "Luma Gain");
        OSD_handleCommand(OSD_CMD_ACE_PAGE1_VALUES);

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
                        Menu_navigateTo(OLED_ACESettings_Enable);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ACESettings_ChromaGain);
                        break;
                    case IR_KEY_RIGHT:
                        if (uopt->advACE) {
                            lastMenuItemTime = millis();
                            uopt->advACELumaGain = MIN(uopt->advACELumaGain + 1, 31);
                            ADV_sendACELumaGain(uopt->advACELumaGain);
                        }
                        break;
                    case IR_KEY_LEFT:
                        if (uopt->advACE) {
                            lastMenuItemTime = millis();
                            uopt->advACELumaGain = MAX((int)uopt->advACELumaGain - 1, 0);
                            ADV_sendACELumaGain(uopt->advACELumaGain);
                        }
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_ACESettings);
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

    // OLED_ACESettings_ChromaGain (Page 1, row 3)
    else if (oled_menuItem == OLED_ACESettings_ChromaGain) {
        showMenu("M>ACE Settings", "Chroma Gain");
        OSD_handleCommand(OSD_CMD_ACE_PAGE1_VALUES);

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
                        Menu_navigateTo(OLED_ACESettings_LumaGain);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ACESettings_ChromaMax);
                        break;
                    case IR_KEY_RIGHT:
                        if (uopt->advACE) {
                            lastMenuItemTime = millis();
                            uopt->advACEChromaGain = MIN(uopt->advACEChromaGain + 1, 15);
                            ADV_sendACEChromaGain(uopt->advACEChromaGain);
                        }
                        break;
                    case IR_KEY_LEFT:
                        if (uopt->advACE) {
                            lastMenuItemTime = millis();
                            uopt->advACEChromaGain = MAX((int)uopt->advACEChromaGain - 1, 0);
                            ADV_sendACEChromaGain(uopt->advACEChromaGain);
                        }
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_ACESettings);
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

    // OLED_ACESettings_ChromaMax (Page 2, row 1)
    else if (oled_menuItem == OLED_ACESettings_ChromaMax) {
        showMenu("M>ACE Settings", "Chroma Max");
        OSD_handleCommand(OSD_CMD_ACE_PAGE2_VALUES);

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
                        Menu_navigateTo(OLED_ACESettings_ChromaGain);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ACESettings_GammaGain);
                        break;
                    case IR_KEY_RIGHT:
                        if (uopt->advACE) {
                            lastMenuItemTime = millis();
                            uopt->advACEChromaMax = MIN(uopt->advACEChromaMax + 1, 15);
                            ADV_sendACEChromaMax(uopt->advACEChromaMax);
                        }
                        break;
                    case IR_KEY_LEFT:
                        if (uopt->advACE) {
                            lastMenuItemTime = millis();
                            uopt->advACEChromaMax = MAX((int)uopt->advACEChromaMax - 1, 0);
                            ADV_sendACEChromaMax(uopt->advACEChromaMax);
                        }
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_ACESettings);
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

    // OLED_ACESettings_GammaGain (Page 2, row 2)
    else if (oled_menuItem == OLED_ACESettings_GammaGain) {
        showMenu("M>ACE Settings", "Gamma Gain");
        OSD_handleCommand(OSD_CMD_ACE_PAGE2_VALUES);

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
                        Menu_navigateTo(OLED_ACESettings_ChromaMax);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ACESettings_ResponseSpeed);
                        break;
                    case IR_KEY_RIGHT:
                        if (uopt->advACE) {
                            lastMenuItemTime = millis();
                            uopt->advACEGammaGain = MIN(uopt->advACEGammaGain + 1, 15);
                            ADV_sendACEGammaGain(uopt->advACEGammaGain);
                        }
                        break;
                    case IR_KEY_LEFT:
                        if (uopt->advACE) {
                            lastMenuItemTime = millis();
                            uopt->advACEGammaGain = MAX((int)uopt->advACEGammaGain - 1, 0);
                            ADV_sendACEGammaGain(uopt->advACEGammaGain);
                        }
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_ACESettings);
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

    // OLED_ACESettings_ResponseSpeed (Page 2, row 3)
    else if (oled_menuItem == OLED_ACESettings_ResponseSpeed) {
        showMenu("M>ACE Settings", "Response Spd");
        OSD_handleCommand(OSD_CMD_ACE_PAGE2_VALUES);

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
                        Menu_navigateTo(OLED_ACESettings_GammaGain);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ACESettings_Default);
                        break;
                    case IR_KEY_RIGHT:
                        if (uopt->advACE) {
                            lastMenuItemTime = millis();
                            uopt->advACEResponseSpeed = MIN(uopt->advACEResponseSpeed + 1, 15);
                            ADV_sendACEResponseSpeed(uopt->advACEResponseSpeed);
                        }
                        break;
                    case IR_KEY_LEFT:
                        if (uopt->advACE) {
                            lastMenuItemTime = millis();
                            uopt->advACEResponseSpeed = MAX((int)uopt->advACEResponseSpeed - 1, 0);
                            ADV_sendACEResponseSpeed(uopt->advACEResponseSpeed);
                        }
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_ACESettings);
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

    // OLED_ACESettings_Default (Page 3, row 1)
    else if (oled_menuItem == OLED_ACESettings_Default) {
        showMenu("M>ACE Settings", "Default");
        OSD_handleCommand(OSD_CMD_ACE_PAGE3_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_ACESettings_ResponseSpeed);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_ACESettings_Enable);
                    break;
                case IR_KEY_OK:
                    // Reset all ACE parameters to defaults
                    uopt->advACELumaGain = ADV_ACE_LUMA_GAIN_DEFAULT;
                    uopt->advACEChromaGain = ADV_ACE_CHROMA_GAIN_DEFAULT;
                    uopt->advACEChromaMax = ADV_ACE_CHROMA_MAX_DEFAULT;
                    uopt->advACEGammaGain = ADV_ACE_GAMMA_GAIN_DEFAULT;
                    uopt->advACEResponseSpeed = ADV_ACE_RESPONSE_SPEED_DEFAULT;
                    ADV_sendACEDefaults();
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_ACESettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}
