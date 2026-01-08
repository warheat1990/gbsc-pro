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
        showMenu("Menu->>>", "Input source");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Restart);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInputSettings);
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

    // OLED_SystemSettings_SVAVInputSettings - Main menu entry (row 2, page 1)
    else if (oled_menuItem == OLED_SystemSettings_SVAVInputSettings) {
        showMenu("Menu->>>", "AV/SV Inputs");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Input);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_OutputResolution);
                    break;
                case IR_KEY_OK:
                    // Only enter submenu if SV/AV input is active
                    if (uopt->activeInputType == InputTypeSV || uopt->activeInputType == InputTypeAV) {
                        Menu_navigateTo(OLED_SystemSettings_SVAVInput_I2PSettings);
                    }
                    break;
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_OutputResolution - Main menu entry (row 3, page 1)
    else if (oled_menuItem == OLED_OutputResolution) {
        showMenu("Menu->>>", "Output Resolution");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings_SVAVInputSettings);
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

    // OLED_ScreenSettings - Main menu entry (row 1, page 2)
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
                    Menu_navigateTo(OLED_ColorSettings);
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

    // OLED_ColorSettings - Main menu entry (row 2, page 2)
    else if (oled_menuItem == OLED_ColorSettings) {
        showMenu("Menu->>>", "Picture Settings");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_ScreenSettings);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_SystemSettings);
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

    // OLED_SystemSettings - Main menu entry (row 3, page 2)
    else if (oled_menuItem == OLED_SystemSettings) {
        showMenu("Menu->>>", "System Settings");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_ColorSettings);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Preferences);
                    break;
                case IR_KEY_OK:
                    Menu_navigateTo(OLED_SystemSettings_Compatibility);
                    break;
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    // ==================== Main Menu Page 3 ====================

    // OLED_Preferences - Main menu entry (row 1, page 3)
    else if (oled_menuItem == OLED_Preferences) {
        showMenu("Menu->>>", "Preferences");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_SystemSettings);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_OK:
                    Menu_navigateTo(OLED_Preferences_Theme);
                    break;
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_FirmwareVersion - Main menu entry (row 3, page 3)
    else if (oled_menuItem == OLED_FirmwareVersion) {
        showMenu("Menu->>>", "Firmware Version");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_FactoryReset);
                    break;
                case IR_KEY_OK:
                    OSD_fillBackground();
                    OSD_handleCommand(OSD_CMD_FIRMWARE_VERSION);
                    oled_menuItem = OLED_FirmwareVersion_Info;
                    break;
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_FirmwareVersion_Info - Firmware version info screen (read-only)
    else if (oled_menuItem == OLED_FirmwareVersion_Info) {
        showMenu("Firmware", "Version");
        OSD_handleCommand(OSD_CMD_FIRMWARE_VERSION);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_EXIT:
                    // Return to main menu page 3
                    Menu_navigateTo(OLED_FirmwareVersion);
                    break;
            }
            irResume();
        }
        return true;
    }

    // ==================== Main Menu Page 4 ====================

    // OLED_FactoryReset - Main menu entry (row 1, page 4)
    else if (oled_menuItem == OLED_FactoryReset) {
        showMenu("Menu->>>", "Factory Reset");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_FirmwareVersion);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Restart);
                    break;
                case IR_KEY_OK:
                    factoryResetSelection = 0;
                    OSD_fillBackground();
                    OSD_handleCommand(OSD_CMD_FACTORY_RESET_CONFIRM);
                    oled_menuItem = OLED_FactoryReset_Confirm;
                    break;
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_FactoryReset_Confirm - Factory Reset confirmation screen
    else if (oled_menuItem == OLED_FactoryReset_Confirm) {
        showMenu("Factory", "Reset?");
        OSD_handleCommand(OSD_CMD_FACTORY_RESET_CONFIRM);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_LEFT:
                    // Switch to Yes
                    factoryResetSelection = 1;
                    break;
                case IR_KEY_RIGHT:
                    // Switch to No
                    factoryResetSelection = 0;
                    break;
                case IR_KEY_OK:
                    if (factoryResetSelection == 1) {
                        // User confirmed Yes - execute factory reset
                        userCommand = '1';
                    } else {
                        // User selected No - return to menu
                        Menu_navigateTo(OLED_FactoryReset);
                    }
                    break;
                case IR_KEY_EXIT:
                    // Return to Factory Reset menu item
                    Menu_navigateTo(OLED_FactoryReset);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Restart - Main menu entry (row 2, page 4)
    else if (oled_menuItem == OLED_Restart) {
        showMenu("Menu->>>", "Restart");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_FactoryReset);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Input);
                    break;
                case IR_KEY_OK:
                    userCommand = 'a';
                    break;
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}
