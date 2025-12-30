// ====================================================================================
// menu-misc.cpp
// IR Menu Handler for Mute, Volume and Info Display
// ====================================================================================

#include "../menu-core.h"
#include "../../drivers/pt2257.h"
#include "../../osd/osd-registry.h"

// ====================================================================================
// External References
// ====================================================================================

extern void saveUserPrefs();

// ====================================================================================
// IR_handleMuteDisplay - Mute Status Display Handler
// ====================================================================================

bool IR_handleMuteDisplay()
{
    if (oled_menuItem != OLED_Mute_Display) {
        return false;
    }

    showMenuCentered(uopt->audioMuted ? "MUTE ON" : "MUTE OFF");

    // TV OSD display (delegated to osd-misc.cpp)
    OSD_renderMuteDisplay(uopt->audioMuted);

    if (irDecode()) {
        switch (results.value) {
            case IR_KEY_MUTE:
                // Toggle mute again
                if (uopt->audioMuted) {
                    PT2257_mute(false);
                    uopt->audioMuted = 0;
                } else {
                    PT2257_mute(true);
                    uopt->audioMuted = 1;
                }
                lastMenuItemTime = millis();  // Reset timeout
                break;
            case IR_KEY_MENU:
            case IR_KEY_EXIT:
                exitMenu();
                break;
        }
        irResume();
    }

    return true;
}

// ====================================================================================
// IR_handleMiscSettings - Volume Adjustment Handler
// ====================================================================================

// Volume key repeat state (non-static, accessed by menu-core.cpp)
uint32_t volumeLastKey = 0;
unsigned long volumeLastRepeatTime = 0;
#define VOLUME_REPEAT_INTERVAL 125  // Interval between repeats (ms)

// Set initial volume key and apply first volume change (called from menu-core.cpp)
void Volume_setInitialKey(uint32_t key)
{
    volumeLastKey = key;
    volumeLastRepeatTime = millis();

    // Apply volume change immediately (volume: 0=mute, 50=max)
    if (key == IR_KEY_VOL_UP) {
        uopt->volume = MIN(uopt->volume + 1, 50);
        PT2257_setVolume(uopt->volume);
    } else if (key == IR_KEY_VOL_DN) {
        uopt->volume = MAX(uopt->volume - 1, 0);
        PT2257_setVolume(uopt->volume);
    }
}

bool IR_handleMiscSettings()
{
    // OLED_Volume_Adjust
    if (oled_menuItem == OLED_Volume_Adjust) {
        showMenuCentered("Volume - / + dB");
        OSD_updateVolumeDisplay(uopt->volume);  // volume: 0=mute, 50=max

        if (irDecode()) {
            bool isRepeat = results.repeat;
            uint32_t key = results.value;

            // For repeat codes: use last key if value is 0xFFFFFFFF (NEC repeat marker)
            if (isRepeat || key == 0xFFFFFFFF) {
                if (volumeLastKey == 0) {
                    // No previous key stored, ignore this repeat
                    irResume();
                    return true;
                }
                // Throttle repeat rate
                unsigned long now = millis();
                if (now - volumeLastRepeatTime < VOLUME_REPEAT_INTERVAL) {
                    irResume();
                    return true;  // Too soon, skip this repeat
                }
                volumeLastRepeatTime = now;
                key = volumeLastKey;
            }

            switch (key) {
                case IR_KEY_VOL_UP:
                    uopt->volume = MIN(uopt->volume + 1, 50);
                    PT2257_setVolume(uopt->volume);
                    volumeLastKey = IR_KEY_VOL_UP;
                    volumeLastRepeatTime = millis();
                    break;
                case IR_KEY_VOL_DN:
                    uopt->volume = MAX(uopt->volume - 1, 0);
                    PT2257_setVolume(uopt->volume);
                    volumeLastKey = IR_KEY_VOL_DN;
                    volumeLastRepeatTime = millis();
                    break;
                case IR_KEY_MENU:
                case IR_KEY_EXIT:
                    volumeLastKey = 0;
                    exitMenu();
                    break;
                case IR_KEY_OK:
                    volumeLastKey = 0;
                    saveUserPrefs();
                    OSD_showSavingFeedback(ROW_1);
                    break;
                default:
                    volumeLastKey = 0;
                    break;
            }
            irResume();
        }
        return true;
    } else {
        volumeLastKey = 0;
    }

    return false;
}

// ====================================================================================
// IR_handleInfoDisplay - Info Display Handler
// ====================================================================================

bool IR_handleInfoDisplay()
{
    if (oled_menuItem != OLED_Info_Display) {
        return false;
    }

    showMenu("Menu-", "Info");

    // TV OSD rendering (delegated to osd-misc.cpp)
    OSD_renderInfoDisplay(isInfoDisplayActive);

    if (irDecode()) {
        switch (results.value) {
            case IR_KEY_MENU:
            case IR_KEY_EXIT:
                exitMenu();
                break;
        }
        irResume();
    }

    return true;
}
