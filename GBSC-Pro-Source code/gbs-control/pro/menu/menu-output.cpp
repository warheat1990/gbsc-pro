// ====================================================================================
// menu-output.cpp
// IR Menu Handler for Output Resolution Selection
// ====================================================================================

#include "menu-common.h"

// ====================================================================================
// External References
// ====================================================================================

extern char userCommand;
extern void saveUserPrefs();

// ====================================================================================
// IR_handleOutputResolution - Output Resolution Menu
// ====================================================================================

bool IR_handleOutputResolution(void)
{
    if (oled_menuItem == OLED_OutputResolution_1080) {
        showMenu("Menu->Output", "1920x1080");

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
                    OSD_handleCommand(OSD_CMD_OUTPUT_1080_1024_960);
                    oled_menuItem = OLED_OutputResolution_1024;
                    break;
                case IR_KEY_OK:
                    userCommand = 's';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_OutputResolution;
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_OutputResolution_1024) {
        showMenu("Menu->Output", "1280x1024");

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
                    OSD_handleCommand(OSD_CMD_OUTPUT_1080_1024_960);
                    oled_menuItem = OLED_OutputResolution_1080;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_OUTPUT_1080_1024_960);
                    oled_menuItem = OLED_OutputResolution_960;
                    break;
                case IR_KEY_OK:
                    userCommand = 'p';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_OutputResolution;
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_OutputResolution_960) {
        showMenu("Menu->Output", "1280x960");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_OUTPUT_1080_1024_960);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_OUTPUT_1080_1024_960);
                    oled_menuItem = OLED_OutputResolution_1024;
                    break;
                case IR_KEY_DOWN:
                    oled_menuItem = OLED_OutputResolution_720;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
                    break;
                case IR_KEY_OK:
                    userCommand = 'f';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_OutputResolution;
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_OutputResolution_720) {
        showMenu("Menu->Output", "1280x720");

        if (results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    oled_menuItem = OLED_OutputResolution_960;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW3);
                    OSD_handleCommand(OSD_CMD_OUTPUT_1080_1024_960);
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
                    oled_menuItem = OLED_OutputResolution_480;
                    break;
                case IR_KEY_OK:
                    userCommand = 'g';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_OutputResolution;
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_OutputResolution_480) {
        showMenu("Menu->Output", "480p/576p");

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
                    OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
                    oled_menuItem = OLED_OutputResolution_720;
                    break;
                case IR_KEY_OK:
                    userCommand = 'h';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_OutputResolution;
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_OutputResolution_PassThrough) {
        showMenu("Menu->Output", "Pass Through");
        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
            OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
        }
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
                    oled_menuItem = OLED_OutputResolution_720;
                    break;
                case IR_KEY_OK:
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_OutputResolution;
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_RetainedSettings) {
        showMenu("Retained settings?", "");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_RIGHT:
                    keepSettings = 0;
                    OSD_handleCommand(OSD_CMD_INPUT_INFO);
                    break;
                case IR_KEY_LEFT:
                    keepSettings = 1;
                    OSD_handleCommand(OSD_CMD_INPUT_INFO);
                    break;
                case IR_KEY_OK:
                    if (keepSettings) {
                        saveUserPrefs();
                    } else {
                        if (tentativeResolution == Output960P)
                            userCommand = 'f';
                        else if (tentativeResolution == Output720P)
                            userCommand = 'g';
                        else if (tentativeResolution == Output480P)
                            userCommand = 'h';
                        else if (tentativeResolution == Output1024P)
                            userCommand = 'p';
                        else if (tentativeResolution == Output1080P)
                            userCommand = 's';
                        else
                            userCommand = 'g';
                    }

                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW2);
                    OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
                    oled_menuItem = OLED_OutputResolution_PassThrough;
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW2);
                    OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
                    oled_menuItem = OLED_OutputResolution_PassThrough;
                    break;
            }
            irResume();
        }
        return true;
    }

    // NOTE: Downscale handler moved to menu-unused.cpp

    return false;
}
