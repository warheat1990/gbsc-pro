// ====================================================================================
// menu-system.cpp
// IR Menu Handler for System Settings
// ====================================================================================

#include "../menu-core.h"
#include "../../drivers/ir_remote.h"
#include "../../../tv5725.h"

// ====================================================================================
// External References
// ====================================================================================

typedef TV5725<GBS_ADDR> GBS;
extern struct userOptions *uopt;
extern char serialCommand;
extern char userCommand;
extern void saveUserPrefs();
extern void disableMotionAdaptDeinterlace();
extern void disableScanlines();

// ====================================================================================
// IR_handleSystemSettings - System Settings Menu
// ====================================================================================

bool IR_handleSystemSettings()
{
    // OLED_SystemSettings_SVAVInputSettings
    if (oled_menuItem == OLED_SystemSettings_SVAVInputSettings) {
        showMenu("Menu->System", "Sv-Av InputSet");
        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_OK:
                    if (uopt->activeInputType == InputTypeSV || uopt->activeInputType == InputTypeAV) {
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_DoubleLine);
                    }
                    break;
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_ClockGenerator);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_Compatibility);
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_Compatibility
    else if (oled_menuItem == OLED_SystemSettings_Compatibility) {
        showMenuToggle("Menu->System", "Compatibility", uopt->advCompatibility == 1);
        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInputSettings);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_MatchedPresets);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    uopt->advCompatibility = !uopt->advCompatibility;
                    if (uopt->advCompatibility > 1)
                        uopt->advCompatibility = 0;
                    ADV_sendCompatibility(uopt->advCompatibility);
                    if (GBS::ADC_INPUT_SEL::read())
                        applyVideoModePreset();
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_MatchedPresets
    else if (oled_menuItem == OLED_SystemSettings_MatchedPresets) {
        showMenuToggle("Menu->System", "Matched presets", uopt->matchPresetSource);
        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_Compatibility);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_Deinterlace);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    serialCommand = 'Z';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_Deinterlace
    else if (oled_menuItem == OLED_SystemSettings_Deinterlace) {
        showMenuValue("Menu->System", "Deinterlace", uopt->deintMode ? "Bob" : "Adaptive");
        OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_MatchedPresets);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_Force5060Hz);
                    break;
                case IR_KEY_OK:
                    if (uopt->deintMode != 1) {
                        uopt->deintMode = 1;
                        disableMotionAdaptDeinterlace();
                        if (GBS::GBS_OPTION_SCANLINES_ENABLED::read()) {
                            disableScanlines();
                        }
                        saveUserPrefs();
                    } else if (uopt->deintMode != 0) {
                        uopt->deintMode = 0;
                        saveUserPrefs();
                    }
                    applyVideoModePreset();
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_Force5060Hz
    else if (oled_menuItem == OLED_SystemSettings_Force5060Hz) {
        showMenuToggle("Menu->System", "Force 50 / 60Hz", uopt->PalForce60);
        OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_Deinterlace);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_LockMethod);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    if (uopt->PalForce60 == 0) {
                        uopt->PalForce60 = 1;
                    } else {
                        uopt->PalForce60 = 0;
                    }
                    saveUserPrefs();
                    applyVideoModePreset();
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_LockMethod
    else if (oled_menuItem == OLED_SystemSettings_LockMethod) {
        showMenuValue("Menu->System", "Lock Method", uopt->frameTimeLockMethod ? "1Vtotal only" : "0Vtotal+VSST");
        OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_Force5060Hz);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_ADCCalibration);
                    break;
                case IR_KEY_OK:
                    userCommand = 'i';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_ADCCalibration
    else if (oled_menuItem == OLED_SystemSettings_ADCCalibration) {
        showMenuToggle("Menu->System", "ADC calibration", uopt->enableCalibrationADC);
        OSD_handleCommand(OSD_CMD_SYS_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_LockMethod);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_FrameTimeLock);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    userCommand = 'w';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_FrameTimeLock
    else if (oled_menuItem == OLED_SystemSettings_FrameTimeLock) {
        showMenuToggle("Menu->System", "Frame Time Lock", uopt->enableFrameTimeLock);
        OSD_handleCommand(OSD_CMD_SYS_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_ADCCalibration);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_ClockGenerator);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    userCommand = '5';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_ClockGenerator
    else if (oled_menuItem == OLED_SystemSettings_ClockGenerator) {
        showMenuToggle("Menu->System", "Clock generator", !uopt->disableExternalClockGenerator);
        OSD_handleCommand(OSD_CMD_SYS_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_FrameTimeLock);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInputSettings);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    userCommand = 'X';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_DoubleLine
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_DoubleLine) {
        showMenuValue("M>Sys>SvAv Set", "DoubleLine", advLineDouble ? "2X" : "1X");
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
                    advLineDouble = !advLineDouble;
                    if(!advLineDouble) {
                        advSmooth = false;
                    }
                    ADV_sendLineDouble(advLineDouble);
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
                    Menu_navigateTo(OLED_SystemSettings_SVAVInput_Bright);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    if (advLineDouble) {
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

    // OLED_SystemSettings_SVAVInput_Bright
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Bright) {
        showMenu("M>Sys>SvAv Set", "Bright");
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1_VALUES);

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
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_Smooth);
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

    // OLED_SystemSettings_SVAVInput_Contrast
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

    // OLED_SystemSettings_SVAVInput_Saturation
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

    // OLED_SystemSettings_SVAVInput_Default
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Default) {
        showMenu("M>Sys>SvAv Set", "Default");
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE2_VALUES);

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
