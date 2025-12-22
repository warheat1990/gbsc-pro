// ====================================================================================
// menu-input.cpp
// IR Menu Handler for Input Selection
// ====================================================================================

#include "menu-common.h"

// ====================================================================================
// External References
// ====================================================================================

extern struct runTimeOptions *rto;

// ====================================================================================
// IR_handleInputSelection - Input Selection Menu
// ====================================================================================

bool IR_handleInputSelection()
{
    // OLED_Input_RGBs
    if (oled_menuItem == OLED_Input_RGBs) {
        showMenu("Menu->Input", "RGBs");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
        }
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_OK:
                    isInfoDisplayActive = 0;
                    rgbComponentMode = 1;
                    InputRGBs_mode(rgbComponentMode);
                    rto->isInLowPowerMode = false;
                    break;
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
                    oled_menuItem = OLED_Input_RGsB;
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_Input;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Input_RGsB
    else if (oled_menuItem == OLED_Input_RGsB) {
        showMenu("Menu->Input", "RGsB");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
            OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
        }
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_OK:
                    isInfoDisplayActive = 0;
                    rgbComponentMode = 1;
                    InputRGsB_mode(rgbComponentMode);
                    break;
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
                    oled_menuItem = OLED_Input_RGBs;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
                    oled_menuItem = OLED_Input_VGA;
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_Input;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Input_VGA
    else if (oled_menuItem == OLED_Input_VGA) {
        showMenu("Menu->Input", "VGA");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
        }
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_OK:
                    isInfoDisplayActive = 0;
                    rgbComponentMode = 0;
                    InputVGA_mode(rgbComponentMode);
                    break;
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
                    oled_menuItem = OLED_Input_RGsB;
                    break;
                case IR_KEY_DOWN:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
                    oled_menuItem = OLED_Input_YPBPR;
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_Input;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Input_YPBPR
    else if (oled_menuItem == OLED_Input_YPBPR) {
        showMenu("Menu->Input", "YPBPR");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
        }
        OSD_handleCommand(OSD_CMD_INPUT_PAGE2_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_OK:
                    isInfoDisplayActive = 0;
                    InputYUV();
                    break;
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
                    oled_menuItem = OLED_Input_VGA;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
                    oled_menuItem = OLED_Input_SV;
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_Input;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Input_SV
    else if (oled_menuItem == OLED_Input_SV) {
        showMenuValue("Menu->Input", "SV", getVideoFormatName(SVModeOption));

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
            OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
        }
        OSD_handleCommand(OSD_CMD_INPUT_PAGE2_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_OK:
                    isInfoDisplayActive = 0;
                    InputSV_mode(SVModeOption + 1);
                    break;
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
                    oled_menuItem = OLED_Input_YPBPR;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
                    oled_menuItem = OLED_Input_AV;
                    break;
                case IR_KEY_LEFT:
                    if (SVModeOption <= MODEOPTION_MIN)
                        SVModeOption = MODEOPTION_MAX;
                    SVModeOption--;
                    SVModeOptionChanged = 1;
                    break;
                case IR_KEY_RIGHT:
                    SVModeOption++;
                    if (SVModeOption >= MODEOPTION_MAX)
                        SVModeOption = MODEOPTION_MIN;
                    SVModeOptionChanged = 1;
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_Input;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Input_AV
    else if (oled_menuItem == OLED_Input_AV) {
        showMenuValue("Menu->Input", "AV", getVideoFormatName(AVModeOption));

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
        }
        OSD_handleCommand(OSD_CMD_INPUT_PAGE2_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_OK:
                    isInfoDisplayActive = 0;
                    InputAV_mode(AVModeOption + 1);
                    break;
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
                    oled_menuItem = OLED_Input_SV;
                    break;
                case IR_KEY_LEFT:
                    if (AVModeOption <= MODEOPTION_MIN)
                        AVModeOption = MODEOPTION_MAX;
                    AVModeOption--;
                    AVModeOptionChanged = 1;
                    break;
                case IR_KEY_RIGHT:
                    AVModeOption++;
                    if (AVModeOption >= MODEOPTION_MAX)
                        AVModeOption = MODEOPTION_MIN;
                    AVModeOptionChanged = 1;
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                    oled_menuItem = OLED_Input;
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}
