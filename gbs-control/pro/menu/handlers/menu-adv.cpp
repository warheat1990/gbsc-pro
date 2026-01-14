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
    // OLED_SystemSettings_SVAVInput_I2PSettings (Page 1, row 1) - Link to I2P Settings submenu
    if (oled_menuItem == OLED_SystemSettings_SVAVInput_I2PSettings) {
        showMenu("M>Sys>SvAv Set", "I2P Settings>");
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_Default);  // Page 3, row 2
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_FiltersSettings);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_OK:
                    // Enter I2P Settings submenu
                    Menu_navigateTo(OLED_I2PSettings_Enable);
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInputSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_FiltersSettings (Page 1, row 2) - Link to Filters submenu
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_FiltersSettings) {
        showMenu("M>Sys>SvAv Set", "Video Filters>");
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_I2PSettings);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_ACESettings);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_OK:
                    // Enter Video Filters submenu
                    Menu_navigateTo(OLED_VideoFiltersSettings_YFilter);
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
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_FiltersSettings);
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
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_Hue);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        uopt->advSaturation = MIN(uopt->advSaturation + STEP, 254);
                        ADV_sendBCSH(0xe3, uopt->advSaturation);  // SD_SAT_Cb
                        ADV_sendBCSH(0xe4, uopt->advSaturation);  // SD_SAT_Cr
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        uopt->advSaturation = MAX(uopt->advSaturation - STEP, 0);
                        ADV_sendBCSH(0xe3, uopt->advSaturation);  // SD_SAT_Cb
                        ADV_sendBCSH(0xe4, uopt->advSaturation);  // SD_SAT_Cr
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

    // OLED_SystemSettings_SVAVInput_Hue (Page 3, row 1)
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Hue) {
        showMenu("M>Sys>SvAv Set", "Hue");
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE3_VALUES);

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
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_Saturation);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_Default);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        uopt->advHue = MIN(uopt->advHue + STEP, 254);
                        ADV_sendBCSH(0x0b, uopt->advHue - 128);
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        uopt->advHue = MAX(uopt->advHue - STEP, 0);
                        ADV_sendBCSH(0x0b, uopt->advHue - 128);
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

    // OLED_SystemSettings_SVAVInput_Default (Page 3, row 2)
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Default) {
        showMenu("M>Sys>SvAv Set", "Default");
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE3_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_Hue);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_I2PSettings);
                    break;
                case IR_KEY_OK:
                    // Reset BCSH to defaults
                    ADV_sendBCSH('D', 'E');
                    uopt->advBrightness = 128;
                    uopt->advContrast = 128;
                    uopt->advSaturation = 128;
                    uopt->advHue = 128;
                    // Reset ACE
                    uopt->advACE = 0;
                    ADV_sendACE(false);
                    // Reset Video Filters to defaults
                    ADV_sendVideoFiltersDefaults();
                    // Reset I2P settings
                    uopt->advI2P = false;
                    uopt->advSmooth = false;
                    ADV_sendI2P(false);
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

// ====================================================================================
// IR_handleVideoFiltersSettings - Video Filters Settings Submenu (inside SV/AV Settings)
// Page 1: Y Filter, C Filter/Override, Bandwidth
// Page 2: Luma Mode, Chroma Mode, Chroma Taps
// Page 3: Default
// ====================================================================================

// Helper: Check if current signal uses NTSC comb filter (3.58 MHz subcarrier)
// Format indices: 0=Auto, 1=PAL, 2=NTSC-M, 3=PAL-60, 4=NTSC-443, 5=NTSC-J,
//   6=PAL-N, 7=PAL-M(wop), 8=PAL-M(wp), 9=PAL-Cn, 10=PAL-Cn(wp), 11=SECAM
static bool isNTSCSignal_Menu(void) {
    uint8_t format = (uopt->activeInputType == InputTypeSV) ? uopt->svVideoFormat : uopt->avVideoFormat;
    // Auto mode (0) - check actual detected signal from GBS STATUS_00 register
    if (format == 0) {
        uint8_t status00 = GBS::STATUS_00::read();
        if (!(status00 & 0x80)) return true;  // No valid signal, default to NTSC
        if (status00 & 0x60) return false;    // 0x40=576p or 0x20=288p/576i = PAL
        if (status00 & 0x18) return true;     // 0x10=480p or 0x08=240p/480i = NTSC
        return true;  // Default to NTSC
    }
    // NTSC comb: 2=NTSC-M, 3=PAL-60, 4=NTSC-443, 5=NTSC-J, 7=PAL-M(wop), 8=PAL-M(wp)
    return (format == 2 || format == 3 || format == 4 || format == 5 || format == 7 || format == 8);
}

// Helper: Get next valid Luma/Chroma mode value (0, 4, 5, 6, 7)
static uint8_t nextCombMode(uint8_t current, bool increment) {
    if (increment) {
        switch (current) {
            case 0: return 4;
            case 4: return 5;
            case 5: return 6;
            case 6: return 7;
            case 7: return 0;
            default: return 0;
        }
    } else {
        switch (current) {
            case 0: return 7;
            case 4: return 0;
            case 5: return 4;
            case 6: return 5;
            case 7: return 6;
            default: return 0;
        }
    }
}

bool IR_handleVideoFiltersSettings()
{
    bool isSV = (uopt->activeInputType == InputTypeSV);
    bool isNTSC = isNTSCSignal_Menu();

    // OLED_VideoFiltersSettings_YFilter (Page 1, row 1) - Y Filter (AV: YSFM, SV: WYSFM)
    if (oled_menuItem == OLED_VideoFiltersSettings_YFilter) {
        showMenu("M>Filters", "Y Filter");
        OSD_handleCommand(OSD_CMD_VIDEOFILTERS_PAGE1_VALUES);

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
                        Menu_navigateTo(OLED_VideoFiltersSettings_Default);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        // Row 2: C Filter (AV) or Override (SV)
                        Menu_navigateTo(isSV ? OLED_VideoFiltersSettings_SVOverride : OLED_VideoFiltersSettings_CFilter);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        if (isSV) {
                            // SV: adjust WY filter (only if override is Manual)
                            if (uopt->advFilterWYOverride) {
                                uopt->advFilterWYShaping = (uopt->advFilterWYShaping >= 19) ? 2 : uopt->advFilterWYShaping + 1;
                                ADV_sendFilterWYShaping(uopt->advFilterWYShaping);
                            }
                        } else {
                            // AV: adjust Y filter
                            uopt->advFilterYShaping = (uopt->advFilterYShaping >= 30) ? 0 : uopt->advFilterYShaping + 1;
                            ADV_sendFilterYShaping(uopt->advFilterYShaping);
                        }
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        if (isSV) {
                            if (uopt->advFilterWYOverride) {
                                uopt->advFilterWYShaping = (uopt->advFilterWYShaping <= 2) ? 19 : uopt->advFilterWYShaping - 1;
                                ADV_sendFilterWYShaping(uopt->advFilterWYShaping);
                            }
                        } else {
                            uopt->advFilterYShaping = (uopt->advFilterYShaping == 0) ? 30 : uopt->advFilterYShaping - 1;
                            ADV_sendFilterYShaping(uopt->advFilterYShaping);
                        }
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_FiltersSettings);
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

    // OLED_VideoFiltersSettings_CFilter (Page 1, row 2) - C Filter (AV only)
    else if (oled_menuItem == OLED_VideoFiltersSettings_CFilter) {
        showMenu("M>Filters", "C Filter");
        OSD_handleCommand(OSD_CMD_VIDEOFILTERS_PAGE1_VALUES);

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
                        Menu_navigateTo(OLED_VideoFiltersSettings_YFilter);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_VideoFiltersSettings_Bandwidth);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        uopt->advFilterCShaping = (uopt->advFilterCShaping >= 7) ? 0 : uopt->advFilterCShaping + 1;
                        ADV_sendFilterCShaping(uopt->advFilterCShaping);
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        uopt->advFilterCShaping = (uopt->advFilterCShaping == 0) ? 7 : uopt->advFilterCShaping - 1;
                        ADV_sendFilterCShaping(uopt->advFilterCShaping);
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_FiltersSettings);
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

    // OLED_VideoFiltersSettings_SVOverride (Page 1, row 2 for SV) - Override (Auto/Manual)
    else if (oled_menuItem == OLED_VideoFiltersSettings_SVOverride) {
        showMenu("M>Filters", "Override");
        OSD_handleCommand(OSD_CMD_VIDEOFILTERS_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_VideoFiltersSettings_YFilter);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_VideoFiltersSettings_Bandwidth);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    uopt->advFilterWYOverride = !uopt->advFilterWYOverride;
                    ADV_sendFilterWYOverride(uopt->advFilterWYOverride);
                    saveUserPrefs();
                    OSD_handleCommand(OSD_CMD_VIDEOFILTERS_PAGE1);
                    OSD_handleCommand(OSD_CMD_VIDEOFILTERS_PAGE1_VALUES);
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_FiltersSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_VideoFiltersSettings_Bandwidth (Page 1, row 3)
    else if (oled_menuItem == OLED_VideoFiltersSettings_Bandwidth) {
        showMenu("M>Filters", "Bandwidth");
        OSD_handleCommand(OSD_CMD_VIDEOFILTERS_PAGE1_VALUES);

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
                        // Row 2: C Filter (AV) or Override (SV)
                        Menu_navigateTo(isSV ? OLED_VideoFiltersSettings_SVOverride : OLED_VideoFiltersSettings_CFilter);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_VideoFiltersSettings_LumaMode);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        if (isNTSC) {
                            uopt->advFilterCombNTSC = (uopt->advFilterCombNTSC >= 3) ? 0 : uopt->advFilterCombNTSC + 1;
                            ADV_sendFilterCombNTSC(uopt->advFilterCombNTSC);
                        } else {
                            uopt->advFilterCombPAL = (uopt->advFilterCombPAL >= 3) ? 0 : uopt->advFilterCombPAL + 1;
                            ADV_sendFilterCombPAL(uopt->advFilterCombPAL);
                        }
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        if (isNTSC) {
                            uopt->advFilterCombNTSC = (uopt->advFilterCombNTSC == 0) ? 3 : uopt->advFilterCombNTSC - 1;
                            ADV_sendFilterCombNTSC(uopt->advFilterCombNTSC);
                        } else {
                            uopt->advFilterCombPAL = (uopt->advFilterCombPAL == 0) ? 3 : uopt->advFilterCombPAL - 1;
                            ADV_sendFilterCombPAL(uopt->advFilterCombPAL);
                        }
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_FiltersSettings);
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

    // OLED_VideoFiltersSettings_LumaMode (Page 2, row 1)
    else if (oled_menuItem == OLED_VideoFiltersSettings_LumaMode) {
        showMenu("M>Filters", "Luma Mode");
        OSD_handleCommand(OSD_CMD_VIDEOFILTERS_PAGE2_VALUES);

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
                        Menu_navigateTo(OLED_VideoFiltersSettings_Bandwidth);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_VideoFiltersSettings_ChromaMode);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        if (isNTSC) {
                            uopt->advCombLumaModeNTSC = nextCombMode(uopt->advCombLumaModeNTSC, true);
                            ADV_sendCombLumaModeNTSC(uopt->advCombLumaModeNTSC);
                        } else {
                            uopt->advCombLumaModePAL = nextCombMode(uopt->advCombLumaModePAL, true);
                            ADV_sendCombLumaModePAL(uopt->advCombLumaModePAL);
                        }
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        if (isNTSC) {
                            uopt->advCombLumaModeNTSC = nextCombMode(uopt->advCombLumaModeNTSC, false);
                            ADV_sendCombLumaModeNTSC(uopt->advCombLumaModeNTSC);
                        } else {
                            uopt->advCombLumaModePAL = nextCombMode(uopt->advCombLumaModePAL, false);
                            ADV_sendCombLumaModePAL(uopt->advCombLumaModePAL);
                        }
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_FiltersSettings);
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

    // OLED_VideoFiltersSettings_ChromaMode (Page 2, row 2)
    else if (oled_menuItem == OLED_VideoFiltersSettings_ChromaMode) {
        showMenu("M>Filters", "Chroma Mode");
        OSD_handleCommand(OSD_CMD_VIDEOFILTERS_PAGE2_VALUES);

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
                        Menu_navigateTo(OLED_VideoFiltersSettings_LumaMode);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_VideoFiltersSettings_ChromaTaps);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        if (isNTSC) {
                            uopt->advCombChromaModeNTSC = nextCombMode(uopt->advCombChromaModeNTSC, true);
                            ADV_sendCombChromaModeNTSC(uopt->advCombChromaModeNTSC);
                        } else {
                            uopt->advCombChromaModePAL = nextCombMode(uopt->advCombChromaModePAL, true);
                            ADV_sendCombChromaModePAL(uopt->advCombChromaModePAL);
                        }
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        if (isNTSC) {
                            uopt->advCombChromaModeNTSC = nextCombMode(uopt->advCombChromaModeNTSC, false);
                            ADV_sendCombChromaModeNTSC(uopt->advCombChromaModeNTSC);
                        } else {
                            uopt->advCombChromaModePAL = nextCombMode(uopt->advCombChromaModePAL, false);
                            ADV_sendCombChromaModePAL(uopt->advCombChromaModePAL);
                        }
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_FiltersSettings);
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

    // OLED_VideoFiltersSettings_ChromaTaps (Page 2, row 3)
    else if (oled_menuItem == OLED_VideoFiltersSettings_ChromaTaps) {
        showMenu("M>Filters", "Chroma Taps");
        OSD_handleCommand(OSD_CMD_VIDEOFILTERS_PAGE2_VALUES);

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
                        Menu_navigateTo(OLED_VideoFiltersSettings_ChromaMode);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_VideoFiltersSettings_Default);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        if (isNTSC) {
                            uopt->advCombChromaTapsNTSC = (uopt->advCombChromaTapsNTSC >= 3) ? 0 : uopt->advCombChromaTapsNTSC + 1;
                            ADV_sendCombChromaTapsNTSC(uopt->advCombChromaTapsNTSC);
                        } else {
                            uopt->advCombChromaTapsPAL = (uopt->advCombChromaTapsPAL >= 3) ? 0 : uopt->advCombChromaTapsPAL + 1;
                            ADV_sendCombChromaTapsPAL(uopt->advCombChromaTapsPAL);
                        }
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        if (isNTSC) {
                            uopt->advCombChromaTapsNTSC = (uopt->advCombChromaTapsNTSC == 0) ? 3 : uopt->advCombChromaTapsNTSC - 1;
                            ADV_sendCombChromaTapsNTSC(uopt->advCombChromaTapsNTSC);
                        } else {
                            uopt->advCombChromaTapsPAL = (uopt->advCombChromaTapsPAL == 0) ? 3 : uopt->advCombChromaTapsPAL - 1;
                            ADV_sendCombChromaTapsPAL(uopt->advCombChromaTapsPAL);
                        }
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_FiltersSettings);
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

    // OLED_VideoFiltersSettings_Default (Page 3, row 1) - Reset Filters to Defaults
    else if (oled_menuItem == OLED_VideoFiltersSettings_Default) {
        showMenu("M>Filters", "Default");
        OSD_handleCommand(OSD_CMD_VIDEOFILTERS_PAGE3_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_VideoFiltersSettings_ChromaTaps);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_VideoFiltersSettings_YFilter);
                    break;
                case IR_KEY_OK:
                    // Reset all video filter parameters to defaults
                    ADV_sendVideoFiltersDefaults();
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_FiltersSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}

// ====================================================================================
// IR_handleI2PSettings - I2P Settings Submenu (inside SV/AV Settings)
// Page 1: Enable I2P/2X, Smooth
// ====================================================================================

bool IR_handleI2PSettings()
{
    // OLED_I2PSettings_Enable (Page 1, row 1)
    if (oled_menuItem == OLED_I2PSettings_Enable) {
        showMenuToggle("M>I2P Settings", "I2P/2X", uopt->advI2P);
        OSD_handleCommand(OSD_CMD_I2P_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_I2PSettings_Smooth);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_I2PSettings_Smooth);
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
                    OSD_handleCommand(OSD_CMD_I2P_PAGE1);
                    OSD_handleCommand(OSD_CMD_I2P_PAGE1_VALUES);
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_I2PSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_I2PSettings_Smooth (Page 1, row 2)
    else if (oled_menuItem == OLED_I2PSettings_Smooth) {
        showMenuToggle("M>I2P Settings", "Smooth", uopt->advSmooth);
        OSD_handleCommand(OSD_CMD_I2P_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_I2PSettings_Enable);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_I2PSettings_Enable);
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
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_I2PSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}
