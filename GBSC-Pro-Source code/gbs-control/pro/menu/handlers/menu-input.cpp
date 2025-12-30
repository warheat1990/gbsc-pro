// ====================================================================================
// menu-input.cpp
// IR Menu Handler for Input Selection
// ====================================================================================

#include "../menu-core.h"

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

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_OK:
                    isInfoDisplayActive = 0;
                    uopt->advCompatibility = 1;
                    InputRGBs_mode(uopt->advCompatibility);
                    rto->isInLowPowerMode = false;
                    break;
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Input_AV);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Input_RGsB);
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Input);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Input_RGsB
    else if (oled_menuItem == OLED_Input_RGsB) {
        showMenu("Menu->Input", "RGsB");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_OK:
                    isInfoDisplayActive = 0;
                    uopt->advCompatibility = 1;
                    InputRGsB_mode(uopt->advCompatibility);
                    break;
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Input_RGBs);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Input_VGA);
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Input);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Input_VGA
    else if (oled_menuItem == OLED_Input_VGA) {
        showMenu("Menu->Input", "VGA");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_OK:
                    isInfoDisplayActive = 0;
                    uopt->advCompatibility = 0;
                    InputVGA_mode(uopt->advCompatibility);
                    break;
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Input_RGsB);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Input_YPBPR);
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Input);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Input_YPBPR
    else if (oled_menuItem == OLED_Input_YPBPR) {
        showMenu("Menu->Input", "YPbPr");

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
                    Menu_navigateTo(OLED_Input_VGA);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Input_SV);
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Input);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Input_SV
    else if (oled_menuItem == OLED_Input_SV) {
        showMenuValue("Menu->Input", "SV", getVideoFormatName(uopt->svVideoFormat));

        OSD_handleCommand(OSD_CMD_INPUT_PAGE2_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_OK:
                    isInfoDisplayActive = 0;
                    InputSV_mode(uopt->svVideoFormat + 1);
                    break;
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Input_YPBPR);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Input_AV);
                    break;
                case IR_KEY_LEFT:
                    if (uopt->svVideoFormat <= MODEOPTION_MIN)
                        uopt->svVideoFormat = MODEOPTION_MAX;
                    uopt->svVideoFormat--;
                    svVideoFormatChanged = 1;
                    break;
                case IR_KEY_RIGHT:
                    uopt->svVideoFormat++;
                    if (uopt->svVideoFormat >= MODEOPTION_MAX)
                        uopt->svVideoFormat = MODEOPTION_MIN;
                    svVideoFormatChanged = 1;
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Input);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Input_AV
    else if (oled_menuItem == OLED_Input_AV) {
        showMenuValue("Menu->Input", "AV", getVideoFormatName(uopt->avVideoFormat));

        OSD_handleCommand(OSD_CMD_INPUT_PAGE2_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_OK:
                    isInfoDisplayActive = 0;
                    InputAV_mode(uopt->avVideoFormat + 1);
                    break;
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Input_SV);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Input_RGBs);
                    break;
                case IR_KEY_LEFT:
                    if (uopt->avVideoFormat <= MODEOPTION_MIN)
                        uopt->avVideoFormat = MODEOPTION_MAX;
                    uopt->avVideoFormat--;
                    avVideoFormatChanged = 1;
                    break;
                case IR_KEY_RIGHT:
                    uopt->avVideoFormat++;
                    if (uopt->avVideoFormat >= MODEOPTION_MAX)
                        uopt->avVideoFormat = MODEOPTION_MIN;
                    avVideoFormatChanged = 1;
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Input);
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}
