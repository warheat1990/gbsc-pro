// ====================================================================================
// menu-main.cpp
// IR Menu Handler for Main Menu Navigation
// ====================================================================================

#include "menu-common.h"

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

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            selectedMenuLine = 1;
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IR_KEY_OK:
                    oled_menuItem = OLED_Input_RGBs;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
                    selectedMenuLine = 1;
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

        if (results.value == IR_KEY_UP || results.value == IR_KEY_DOWN) {
            selectedMenuLine = 2;
            OSD_highlightIcon(2);
            OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_Input;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_ScreenSettings;
                    break;
                case IR_KEY_OK:
                    oled_menuItem = OLED_OutputResolution_1080;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_OUTPUT_1080_1024_960);
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
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IR_KEY_OK:
                    oled_menuItem = OLED_ScreenSettings_Move;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
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

        if (results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    oled_menuItem = OLED_ScreenSettings;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IR_KEY_OK:
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
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

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            selectedMenuLine = 2;
            OSD_highlightIcon(2);
            OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    oled_menuItem = OLED_ResetSettings;
                    break;
                case IR_KEY_OK:
                    oled_menuItem = OLED_ColorSettings_RGB_R;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
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

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_OK) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
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

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
        }

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
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
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
