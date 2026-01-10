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
extern void resetSyncProcessor();

// ====================================================================================
// IR_handleSystemSettings - System Settings Menu
// ====================================================================================

bool IR_handleSystemSettings()
{
    // OLED_SystemSettings_SyncStripper (first item - wraps from HdmiLimitedRange)
    // Note: Sync Stripper only applies to RGB inputs, not AV/SV
    if (oled_menuItem == OLED_SystemSettings_SyncStripper) {
        bool syncStripperAvailable = (uopt->activeInputType != InputTypeSV) &&
                                      (uopt->activeInputType != InputTypeAV);
        showMenuToggle("Menu->System", "Sync Stripper", uopt->advSyncStripper == 1);
        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_HdmiLimitedRange);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_MatchedPresets);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    if (syncStripperAvailable) {
                        uopt->advSyncStripper = !uopt->advSyncStripper;
                        if (uopt->advSyncStripper > 1)
                            uopt->advSyncStripper = 1;
                        ADV_sendSyncStripper(uopt->advSyncStripper);
                        saveUserPrefs();
                        if (GBS::ADC_INPUT_SEL::read()) {
                            resetSyncProcessor();
                            delay(50);
                            applyVideoModePreset();
                        }
                    }
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
                    Menu_navigateTo(OLED_SystemSettings_SyncStripper);
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
        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);

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
        OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);

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
                    Menu_navigateTo(OLED_SystemSettings_HdmiLimitedRange);
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

    // OLED_SystemSettings_HdmiLimitedRange (last item - wraps to Sync Stripper)
    else if (oled_menuItem == OLED_SystemSettings_HdmiLimitedRange) {
        static const char* hdmiLimitedLabels[] = {"Off", "HD", "SD", "All"};
        showMenuValue("Menu->System", "HDMI Limited Range", hdmiLimitedLabels[uopt->hdmiLimitedRange]);
        OSD_handleCommand(OSD_CMD_SYS_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_ClockGenerator);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_SyncStripper);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_OK:
                    uopt->hdmiLimitedRange = (uopt->hdmiLimitedRange + 1) % 4;
                    saveUserPrefs();
                    applyPresets(rto->videoStandardInput);
                    break;
                case IR_KEY_LEFT:
                    uopt->hdmiLimitedRange = (uopt->hdmiLimitedRange + 3) % 4;
                    saveUserPrefs();
                    applyPresets(rto->videoStandardInput);
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_SystemSettings);
                    break;
            }
            irResume();
        }
        return true;
    }
    return false;
}
