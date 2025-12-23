// ====================================================================================
// menu-main.cpp
// IR Menu Handler for Main Menu Navigation
// ====================================================================================

#include "../menu-core.h"

// ====================================================================================
// External References
// ====================================================================================

extern char userCommand;

// ====================================================================================
// IR_handleMainMenu - Main Menu Navigation
// ====================================================================================

bool IR_handleMainMenu()
{
    // ==================== Main Menu Page 1 ====================

    // OLED_Input - Main menu entry (row 1, page 1)
    if (oled_menuItem == OLED_Input) {
        showMenu("Menu->>>", "Input");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_OutputResolution);
                    break;
                case IR_KEY_OK:
                    Menu_navigateTo(OLED_Input_RGBs);
                    break;
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_OutputResolution - Main menu entry (row 2, page 1)
    else if (oled_menuItem == OLED_OutputResolution) {
        showMenu("Menu->>>", "Output Resolution");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Input);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_ScreenSettings);
                    break;
                case IR_KEY_OK:
                    Menu_navigateTo(OLED_OutputResolution_1080);
                    break;
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ScreenSettings - Main menu entry (row 3, page 1)
    else if (oled_menuItem == OLED_ScreenSettings) {
        showMenu("Menu->>>", "Screen Settings");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_OutputResolution);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings);
                    break;
                case IR_KEY_OK:
                    Menu_navigateTo(OLED_ScreenSettings_Move);
                    break;
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    // ==================== Main Menu Page 2 ====================

    // OLED_SystemSettings - Main menu entry (row 1, page 2)
    else if (oled_menuItem == OLED_SystemSettings) {
        showMenu("Menu->>>", "System Settings");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_ScreenSettings);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_ColorSettings);
                    break;
                case IR_KEY_OK:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInputSettings);
                    break;
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings - Main menu entry (row 2, page 2)
    else if (oled_menuItem == OLED_ColorSettings) {
        showMenu("Menu->>>", "Picture Settings");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_ResetSettings);
                    break;
                case IR_KEY_OK:
                    Menu_navigateTo(OLED_ColorSettings_RGB_R);
                    break;
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ResetSettings - Main menu entry (row 3, page 2)
    else if (oled_menuItem == OLED_ResetSettings) {
        showMenu("Menu->>>", "Reset Settings");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_ColorSettings);
                    break;
                case IR_KEY_OK:
                    userCommand = '1';
                    break;
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Restart
    else if (oled_menuItem == OLED_Restart) {
        showMenu("Menu-", "Restart");
        OSD_handleCommand(OSD_CMD_SYS_PAGE5_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE5);
                    oled_menuItem = OLED_EnableOTA;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE5);
                    oled_menuItem = OLED_ResetDefaults;
                    break;
                case IR_KEY_OK:
                    userCommand = 'a';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW3);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ResetSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}
