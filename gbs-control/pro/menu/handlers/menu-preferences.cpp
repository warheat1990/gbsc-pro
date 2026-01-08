// ====================================================================================
// menu-preferences.cpp
// IR Menu Handler for Preferences Menu (Theme, Volume, Mute)
// ====================================================================================

#include "../menu-core.h"
#include "../../drivers/pt2257.h"

// ====================================================================================
// External References
// ====================================================================================

extern void saveUserPrefs();

// ====================================================================================
// IR_handlePreferencesMenu - Preferences Menu Navigation
// ====================================================================================

bool IR_handlePreferencesMenu()
{
    // OLED_Preferences_Theme - Theme selection with left/right arrows
    if (oled_menuItem == OLED_Preferences_Theme) {
        showMenu("Preferences", "Theme");
        OSD_handleCommand(OSD_CMD_PREFERENCES_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Preferences_Mute);  // Wrap to last
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Preferences_Volume);
                    break;
                case IR_KEY_LEFT:
                    // Previous theme (wrap around)
                    {
                        uint8_t currentTheme = OSD_getTheme();
                        uint8_t newTheme = (currentTheme == 0) ? (OSD_THEME_COUNT - 1) : (currentTheme - 1);
                        OSD_setTheme(newTheme);
                        uopt->osdTheme = newTheme;
                        // Redraw with new theme colors
                        OSD_fillBackground();
                        OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                        OSD_handleCommand(OSD_CMD_PREFERENCES_PAGE1);
                        OSD_handleCommand(OSD_CMD_PREFERENCES_PAGE1_VALUES);
                        saveUserPrefs();
                    }
                    break;
                case IR_KEY_RIGHT:
                    // Next theme (wrap around)
                    {
                        uint8_t currentTheme = OSD_getTheme();
                        uint8_t newTheme = (currentTheme + 1) % OSD_THEME_COUNT;
                        OSD_setTheme(newTheme);
                        uopt->osdTheme = newTheme;
                        // Redraw with new theme colors
                        OSD_fillBackground();
                        OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                        OSD_handleCommand(OSD_CMD_PREFERENCES_PAGE1);
                        OSD_handleCommand(OSD_CMD_PREFERENCES_PAGE1_VALUES);
                        saveUserPrefs();
                    }
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Preferences);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Preferences_Volume - Volume adjustment with left/right arrows (with key repeat)
    else if (oled_menuItem == OLED_Preferences_Volume) {
        showMenu("Preferences", "Volume");
        OSD_handleCommand(OSD_CMD_PREFERENCES_PAGE1_VALUES);

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
                        Menu_navigateTo(OLED_Preferences_Theme);
                        break;
                    case IR_KEY_DOWN:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_Preferences_Mute);
                        break;
                    case IR_KEY_RIGHT:
                        // Increase volume (volume: 0=mute, 50=max)
                        lastMenuItemTime = millis();
                        uopt->volume = MIN(uopt->volume + 1, 50);
                        PT2257_setVolume(uopt->volume);
                        break;
                    case IR_KEY_LEFT:
                        // Decrease volume (volume: 0=mute, 50=max)
                        lastMenuItemTime = millis();
                        uopt->volume = MAX(uopt->volume - 1, 0);
                        PT2257_setVolume(uopt->volume);
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        saveUserPrefs();
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_Preferences);
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

    // OLED_Preferences_Mute - Mute toggle with OK button
    else if (oled_menuItem == OLED_Preferences_Mute) {
        showMenuToggle("Preferences", "Mute", uopt->audioMuted);
        OSD_handleCommand(OSD_CMD_PREFERENCES_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Preferences_Volume);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Preferences_Theme);  // Wrap to first
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    uopt->audioMuted = !uopt->audioMuted;
                    PT2257_mute(uopt->audioMuted);
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Preferences);
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}
