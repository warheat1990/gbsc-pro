// ====================================================================================
// menu-screen.cpp
// IR Menu Handler for Screen Settings
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
extern void shiftVerticalUpIF();
extern void shiftVerticalDownIF();
extern void saveUserPrefs();
extern uint8_t getVideoMode();
extern void applyPresets(uint8_t videoMode);

// ====================================================================================
// IR_handleScreenSettings - Screen Settings Menu
// ====================================================================================

bool IR_handleScreenSettings(void)
{
    if (oled_menuItem == OLED_ScreenSettings_Move) {
        showMenu("Menu->Screen", "Move");

        if (results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Scale;
                    break;
                case IR_KEY_OK:
                    oled_menuItem = OLED_ScreenSettings_MoveAdjust;
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    OSD_showAdjustArrows(1, 5);
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
                    oled_menuItem = OLED_ScreenSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_ScreenSettings_Scale) {
        showMenu("Menu->Screen", "Scale");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Move;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Borders;
                    break;
                case IR_KEY_OK:
                    oled_menuItem = OLED_ScreenSettings_ScaleAdjust;
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    OSD_showAdjustArrows(2, 6);
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
                    oled_menuItem = OLED_ScreenSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_ScreenSettings_Borders) {
        showMenu("Menu->Screen", "Borders");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Scale;
                    break;
                case IR_KEY_DOWN:
                    oled_menuItem = OLED_ScreenSettings_FullHeight;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SCREEN_FULLHEIGHT);
                    break;
                case IR_KEY_OK:
                    oled_menuItem = OLED_ScreenSettings_BordersAdjust;
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    OSD_showAdjustArrows(3);
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
                    oled_menuItem = OLED_ScreenSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_ScreenSettings_MoveAdjust) {
        if (results.value == IR_KEY_OK) {
            OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
            OSD_showAdjustArrows(1, 5);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_OK:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Move;
                    break;
                case IR_KEY_RIGHT:
                    lastMenuItemTime = millis();
                    serialCommand = '6';
                    if (GBS::IF_HBIN_SP::read() < 10) {
                        OSD_showLimitFeedback(ROW_1);
                    }
                    break;
                case IR_KEY_LEFT:
                    lastMenuItemTime = millis();
                    serialCommand = '7';
                    if (GBS::IF_HBIN_SP::read() >= 0x150) {
                        OSD_showLimitFeedback(ROW_1);
                    }
                    break;
                case IR_KEY_UP:
                    lastMenuItemTime = millis();
                    shiftVerticalUpIF();
                    break;
                case IR_KEY_DOWN:
                    lastMenuItemTime = millis();
                    shiftVerticalDownIF();
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Move;
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_ScreenSettings_ScaleAdjust) {
        if (results.value == IR_KEY_OK) {
            OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
            OSD_showAdjustArrows(2, 6);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_OK:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Scale;
                    break;
                case IR_KEY_RIGHT:
                    lastMenuItemTime = millis();
                    serialCommand = 'h';
                    if (GBS::VDS_HSCALE::read() == 1023) {
                        OSD_showLimitFeedback(ROW_2);
                    }
                    break;
                case IR_KEY_LEFT:
                    lastMenuItemTime = millis();
                    serialCommand = 'z';
                    if (GBS::VDS_HSCALE::read() <= 256) {
                        OSD_showLimitFeedback(ROW_2);
                    }
                    break;
                case IR_KEY_UP:
                    lastMenuItemTime = millis();
                    serialCommand = '5';
                    if (GBS::VDS_VSCALE::read() == 1023) {
                        OSD_showLimitFeedback(ROW_2);
                    }
                    break;
                case IR_KEY_DOWN:
                    lastMenuItemTime = millis();
                    serialCommand = '4';
                    if (GBS::VDS_VSCALE::read() <= 256) {
                        OSD_showLimitFeedback(ROW_2);
                    }
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Scale;
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_ScreenSettings_BordersAdjust) {
        if (results.value == IR_KEY_OK) {
            OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
            OSD_showAdjustArrows(3);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_OK:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Borders;
                    break;
                case IR_KEY_RIGHT:
                    userCommand = 'A';
                    if (!((GBS::VDS_DIS_HB_ST::read() > 4) && (GBS::VDS_DIS_HB_SP::read() < (GBS::VDS_HSYNC_RST::read() - 4)))) {
                        OSD_showLimitFeedback(ROW_3);
                    }
                    break;
                case IR_KEY_LEFT:
                    userCommand = 'B';
                    if (!((GBS::VDS_DIS_HB_ST::read() < (GBS::VDS_HSYNC_RST::read() - 4)) && (GBS::VDS_DIS_HB_SP::read() > 4))) {
                        OSD_showLimitFeedback(ROW_3);
                    }
                    break;
                case IR_KEY_UP:
                    userCommand = 'C';
                    if (!((GBS::VDS_DIS_VB_ST::read() > 6) && (GBS::VDS_DIS_VB_SP::read() < (GBS::VDS_VSYNC_RST::read() - 4)))) {
                        OSD_showLimitFeedback(ROW_3);
                    }
                    break;
                case IR_KEY_DOWN:
                    userCommand = 'D';
                    if (!((GBS::VDS_DIS_VB_ST::read() < (GBS::VDS_VSYNC_RST::read() - 4)) && (GBS::VDS_DIS_VB_SP::read() > 6))) {
                        OSD_showLimitFeedback(ROW_3);
                    }
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Borders;
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_ScreenSettings_FullHeight) {
        showMenuToggle("Menu->Screen", "Full height", uopt->wantFullHeight);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_SCREEN_FULLHEIGHT);
        }
        OSD_handleCommand(OSD_CMD_SCREEN_FULLHEIGHT_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    oled_menuItem = OLED_ScreenSettings_Borders;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    break;
                case IR_KEY_OK: {
                    uopt->wantFullHeight = !uopt->wantFullHeight;
                    saveUserPrefs();
                    uint8_t vidMode = getVideoMode();
                    if (uopt->presetPreference == 5) {
                        if (GBS::GBS_PRESET_ID::read() == 0x05 || GBS::GBS_PRESET_ID::read() == 0x15) {
                            applyPresets(vidMode);
                        }
                    }
                    applyVideoModePreset();
                } break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
                    oled_menuItem = OLED_ScreenSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}
