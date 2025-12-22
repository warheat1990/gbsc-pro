// ====================================================================================
// menu-system.cpp
// IR Menu Handler for System Settings
// ====================================================================================

#include "menu-common.h"
#include "../osd-render-pro.h"
#include "../drivers/ir_remote.h"
#include "../../tv5725.h"

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
    // OLED_SystemSettings_MatchedPresets
    if (oled_menuItem == OLED_SystemSettings_MatchedPresets) {
        showMenuToggle("Menu->System", "Matched presets", uopt->matchPresetSource);

        if (results.value == IR_KEY_UP || results.value == IR_KEY_DOWN) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_SYS_PAGE1);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_Compatibility;
                    break;
                case IR_KEY_DOWN:
                    oled_menuItem = OLED_SystemSettings_Deinterlace;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    break;
                case IR_KEY_OK:
                    serialCommand = 'Z';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_UseUpscaling
    else if (oled_menuItem == OLED_SystemSettings_UseUpscaling) {
        showMenuToggle("Menu->System", "Use upscaling", uopt->preferScalingRgbhv);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_SYS_PAGE1);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_Compatibility;
                    break;
                case IR_KEY_DOWN:
                    oled_menuItem = OLED_SystemSettings_Deinterlace;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_Force5060Hz
    else if (oled_menuItem == OLED_SystemSettings_Force5060Hz) {
        showMenuToggle("Menu->System", "Force 50 / 60Hz", uopt->PalForce60);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    oled_menuItem = OLED_SystemSettings_Deinterlace;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    oled_menuItem = OLED_SystemSettings_LockMethod;
                    break;
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
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_ClockGenerator
    else if (oled_menuItem == OLED_SystemSettings_ClockGenerator) {
        showMenuToggle("Menu->System", "Clock generator", !uopt->disableExternalClockGenerator);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_SYS_PAGE4);
        }
        OSD_handleCommand(OSD_CMD_SYS_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE4);
                    oled_menuItem = OLED_SystemSettings_FrameTimeLock;
                    break;
                case IR_KEY_OK:
                    userCommand = 'X';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_ADCCalibration
    else if (oled_menuItem == OLED_SystemSettings_ADCCalibration) {
        showMenuToggle("Menu->System", "ADC calibration", uopt->enableCalibrationADC);

        if (results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_SYS_PAGE4);
        }
        OSD_handleCommand(OSD_CMD_SYS_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    oled_menuItem = OLED_SystemSettings_LockMethod;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW3);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE4);
                    oled_menuItem = OLED_SystemSettings_FrameTimeLock;
                    break;
                case IR_KEY_OK:
                    userCommand = 'w';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_FrameTimeLock
    else if (oled_menuItem == OLED_SystemSettings_FrameTimeLock) {
        showMenuToggle("Menu->System", "Frame Time Lock", uopt->enableFrameTimeLock);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE4);
                    oled_menuItem = OLED_SystemSettings_ADCCalibration;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE4);
                    oled_menuItem = OLED_SystemSettings_ClockGenerator;
                    break;
                case IR_KEY_OK:
                    userCommand = '5';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_LockMethod
    else if (oled_menuItem == OLED_SystemSettings_LockMethod) {
        showMenuValue("Menu->System", "Lock Method",
                      uopt->frameTimeLockMethod ? "1Vtotal only" : "0Vtotal+VSST");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_SYS_PAGE2);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    oled_menuItem = OLED_SystemSettings_Force5060Hz;
                    break;
                case IR_KEY_DOWN:
                    oled_menuItem = OLED_SystemSettings_ADCCalibration;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE4);
                    break;
                case IR_KEY_OK:
                    userCommand = 'i';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_Deinterlace
    else if (oled_menuItem == OLED_SystemSettings_Deinterlace) {
        showMenuValue("Menu->System", "Deinterlace", uopt->deintMode ? "Bob" : "Adaptive");

        if (results.value == IR_KEY_UP || results.value == IR_KEY_DOWN) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_SYS_PAGE2);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    oled_menuItem = OLED_SystemSettings_MatchedPresets;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW3);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    oled_menuItem = OLED_SystemSettings_Force5060Hz;
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
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInputSettings
    else if (oled_menuItem == OLED_SystemSettings_SVAVInputSettings) {
        showMenu("Menu->System", "Sv-Av InputSet");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_SYS_PAGE1);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_OK:
                    if (inputType == InputTypeSV || inputType == InputTypeAV) {
                        selectedMenuLine = 1;
                        OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1);
                        oled_menuItem = OLED_SystemSettings_SVAVInput_DoubleLine;
                    }
                    break;
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_Compatibility;
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_DoubleLine
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_DoubleLine) {
        showMenuValue("M>Sys>SvAv Set", "DoubleLine", lineOption ? "2X" : "1X");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1);
        }
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Smooth;
                    break;
                case IR_KEY_OK:
                    lineOption = !lineOption;
                    if(!lineOption) {
                        smoothOption = false;
                    }
                    settingLineOptionChanged = 1;
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_Smooth
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Smooth) {
        showMenuToggle("M>Sys>SvAv Set", "Smooth", smoothOption);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
            OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1);
        }
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_DoubleLine;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Bright;
                    break;
                case IR_KEY_OK:
                    if (lineOption) {
                        smoothOption = !smoothOption;
                        settingSmoothOptionChanged = 1;
                    }
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_Bright
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Bright) {
        showMenu("M>Sys>SvAv Set", "Bright");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1);
        }
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Smooth;
                    break;
                case IR_KEY_DOWN:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE2);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Contrast;
                    break;
                case IR_KEY_RIGHT:
                    brightness = MIN(brightness + STEP, 254);
                    ADV_sendBCSH(0x0a, brightness - 128);
                    break;
                case IR_KEY_LEFT:
                    brightness = MAX(brightness - STEP, 0);
                    ADV_sendBCSH(0x0a, brightness - 128);
                    break;
                case IR_KEY_OK:
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    saveUserPrefs();
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_Contrast
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Contrast) {
        showMenu("M>Sys>SvAv Set", "Contrast");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE2);
        }
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE2_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW3);
                    OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Bright;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE2);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Saturation;
                    break;
                case IR_KEY_RIGHT:
                    contrast = MIN(contrast + STEP, 254);
                    ADV_sendBCSH(0x08, contrast);
                    break;
                case IR_KEY_LEFT:
                    contrast = MAX(contrast - STEP, 0);
                    ADV_sendBCSH(0x08, contrast);
                    break;
                case IR_KEY_OK:
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_Saturation
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Saturation) {
        showMenu("M>Sys>SvAv Set", "Saturation");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
            OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE2);
        }
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE2_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE2);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Contrast;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE2);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Default;
                    break;
                case IR_KEY_RIGHT:
                    saturation = MIN(saturation + STEP, 254);
                    ADV_sendBCSH(0xe3, saturation);
                    break;
                case IR_KEY_LEFT:
                    saturation = MAX(saturation - STEP, 0);
                    ADV_sendBCSH(0xe3, saturation);
                    break;
                case IR_KEY_OK:
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_Default
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Default) {
        showMenu("M>Sys>SvAv Set", "Default");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE2);
        }
        OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE2_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SVAVINPUT_PAGE2);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Saturation;
                    break;
                case IR_KEY_OK:
                    ADV_sendBCSH('D', 'E');
                    brightness = 128;
                    contrast = 128;
                    saturation = 128;
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_Compatibility
    else if (oled_menuItem == OLED_SystemSettings_Compatibility) {
        showMenuToggle("Menu->System", "Compatibility", rgbComponentMode == 1);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
            OSD_handleCommand(OSD_CMD_SYS_PAGE1);
        }
        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_MatchedPresets;
                    break;
                case IR_KEY_OK:
                    rgbComponentMode = !rgbComponentMode;
                    if (rgbComponentMode > 1)
                        rgbComponentMode = 0;
                    ADV_sendCompatibility(rgbComponentMode);
                    if (GBS::ADC_INPUT_SEL::read())
                        applyVideoModePreset();
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // NOTE: ComponentVGA, Developer handlers moved to menu-unused.cpp

    return false;
}
