// ====================================================================================
// menu-developer.cpp
// IR Menu Handlers for Developer Menu
// ====================================================================================

#include "../menu-core.h"

// ====================================================================================
// External References
// ====================================================================================

extern char serialCommand;
extern char userCommand;

// ====================================================================================
// Developer Menu Handler
// ====================================================================================

bool IR_handleDeveloperMenu()
{
    // OLED_Developer - Main menu entry (row 1, page 3)
    if (oled_menuItem == OLED_Developer) {
        showMenu("Menu->>>", "Developer");

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Preferences);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_FirmwareVersion);
                    break;
                case IR_KEY_OK:
                    Menu_navigateTo(OLED_Developer_MemoryAdjust);
                    break;
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Developer_MemoryAdjust - MEM left/right
    else if (oled_menuItem == OLED_Developer_MemoryAdjust) {
        showMenu("Developer", "MEM left/right");
        OSD_handleCommand(OSD_CMD_DEV_MEMORY_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_HSyncAdjust);
                    break;
                case IR_KEY_RIGHT:
                    serialCommand = '+';
                    break;
                case IR_KEY_LEFT:
                    serialCommand = '-';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Developer_HSyncAdjust - HS left/right
    else if (oled_menuItem == OLED_Developer_HSyncAdjust) {
        showMenu("Developer", "HS left/right");
        OSD_handleCommand(OSD_CMD_DEV_MEMORY_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_MemoryAdjust);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_HTotalAdjust);
                    break;
                case IR_KEY_RIGHT:
                    serialCommand = '0';
                    break;
                case IR_KEY_LEFT:
                    serialCommand = '1';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Developer_HTotalAdjust - HTotal -/+
    else if (oled_menuItem == OLED_Developer_HTotalAdjust) {
        showMenu("Developer", "HTotal -/+");
        OSD_handleCommand(OSD_CMD_DEV_MEMORY_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_HSyncAdjust);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_DebugView);
                    break;
                case IR_KEY_RIGHT:
                    serialCommand = 'a';
                    break;
                case IR_KEY_LEFT:
                    serialCommand = 'A';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Developer_DebugView - Debug view
    else if (oled_menuItem == OLED_Developer_DebugView) {
        showMenu("Developer", "Debug view");
        OSD_handleCommand(OSD_CMD_DEV_DEBUG_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_HTotalAdjust);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_ADCFilter);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    serialCommand = 'D';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Developer_ADCFilter - ADC filter
    else if (oled_menuItem == OLED_Developer_ADCFilter) {
        showMenu("Developer", "ADC filter");
        OSD_handleCommand(OSD_CMD_DEV_DEBUG_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_DebugView);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_FreezeCapture);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    serialCommand = 'F';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Developer_FreezeCapture - Freeze capture
    else if (oled_menuItem == OLED_Developer_FreezeCapture) {
        showMenu("Developer", "Freeze capture");
        OSD_handleCommand(OSD_CMD_DEV_DEBUG_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_ADCFilter);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_MemoryAdjust);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    userCommand = 'F';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}
