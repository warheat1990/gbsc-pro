// ====================================================================================
// menu-screen.cpp
// IR Menu Handler for Screen Settings
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
        OSD_handleCommand(OSD_CMD_SCREEN_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_ScreenSettings_Scale);
                    break;
                case IR_KEY_OK:
                    oled_menuItem = OLED_ScreenSettings_MoveAdjust;
                    OSD_handleCommand(OSD_CMD_SCREEN_PAGE1);
                    OSD_showAdjustArrows(1);
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_ScreenSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_ScreenSettings_Scale) {
        showMenu("Menu->Screen", "Scale");
        OSD_handleCommand(OSD_CMD_SCREEN_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_ScreenSettings_Move);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_ScreenSettings_Borders);
                    break;
                case IR_KEY_OK:
                    oled_menuItem = OLED_ScreenSettings_ScaleAdjust;
                    OSD_handleCommand(OSD_CMD_SCREEN_PAGE1);
                    OSD_showAdjustArrows(2);
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_ScreenSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_ScreenSettings_Borders) {
        showMenu("Menu->Screen", "Borders");
        OSD_handleCommand(OSD_CMD_SCREEN_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_ScreenSettings_Scale);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_ScreenSettings_FullHeight);
                    break;
                case IR_KEY_OK:
                    oled_menuItem = OLED_ScreenSettings_BordersAdjust;
                    OSD_handleCommand(OSD_CMD_SCREEN_PAGE1);
                    OSD_showAdjustArrows(3);
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_ScreenSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_ScreenSettings_MoveAdjust) {
        OSD_handleCommand(OSD_CMD_SCREEN_PAGE1_VALUES);
        if (irDecode()) {
            uint32_t key = IR_getKeyRepeat();
            if (key) {
                switch (key) {
                    case IR_KEY_MENU:
                        IR_clearRepeatKey();
                        exitMenu();
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ScreenSettings_Move);
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
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ScreenSettings_Move);
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

    if (oled_menuItem == OLED_ScreenSettings_ScaleAdjust) {
        OSD_handleCommand(OSD_CMD_SCREEN_PAGE1_VALUES);
        if (irDecode()) {
            uint32_t key = IR_getKeyRepeat();
            if (key) {
                switch (key) {
                    case IR_KEY_MENU:
                        IR_clearRepeatKey();
                        exitMenu();
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ScreenSettings_Scale);
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
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ScreenSettings_Scale);
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

    if (oled_menuItem == OLED_ScreenSettings_BordersAdjust) {
        OSD_handleCommand(OSD_CMD_SCREEN_PAGE1_VALUES);
        if (irDecode()) {
            uint32_t key = IR_getKeyRepeat();
            if (key) {
                switch (key) {
                    case IR_KEY_MENU:
                        IR_clearRepeatKey();
                        exitMenu();
                        break;
                    case IR_KEY_OK:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ScreenSettings_Borders);
                        break;
                    case IR_KEY_RIGHT:
                        lastMenuItemTime = millis();
                        userCommand = 'A';
                        if (!((GBS::VDS_DIS_HB_ST::read() > 4) && (GBS::VDS_DIS_HB_SP::read() < (GBS::VDS_HSYNC_RST::read() - 4)))) {
                            OSD_showLimitFeedback(ROW_3);
                        }
                        break;
                    case IR_KEY_LEFT:
                        lastMenuItemTime = millis();
                        userCommand = 'B';
                        if (!((GBS::VDS_DIS_HB_ST::read() < (GBS::VDS_HSYNC_RST::read() - 4)) && (GBS::VDS_DIS_HB_SP::read() > 4))) {
                            OSD_showLimitFeedback(ROW_3);
                        }
                        break;
                    case IR_KEY_UP:
                        lastMenuItemTime = millis();
                        userCommand = 'C';
                        if (!((GBS::VDS_DIS_VB_ST::read() > 6) && (GBS::VDS_DIS_VB_SP::read() < (GBS::VDS_VSYNC_RST::read() - 4)))) {
                            OSD_showLimitFeedback(ROW_3);
                        }
                        break;
                    case IR_KEY_DOWN:
                        lastMenuItemTime = millis();
                        userCommand = 'D';
                        if (!((GBS::VDS_DIS_VB_ST::read() < (GBS::VDS_VSYNC_RST::read() - 4)) && (GBS::VDS_DIS_VB_SP::read() > 6))) {
                            OSD_showLimitFeedback(ROW_3);
                        }
                        break;
                    case IR_KEY_EXIT:
                        IR_clearRepeatKey();
                        Menu_navigateTo(OLED_ScreenSettings_Borders);
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

    if (oled_menuItem == OLED_ScreenSettings_FullHeight) {
        showMenuToggle("Menu->Screen", "Full height", uopt->wantFullHeight);
        OSD_handleCommand(OSD_CMD_SCREEN_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_ScreenSettings_Borders);
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
                    Menu_navigateTo(OLED_ScreenSettings);
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}
