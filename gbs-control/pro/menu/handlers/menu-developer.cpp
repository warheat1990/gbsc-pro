// ====================================================================================
// menu-developer.cpp
// IR Menu Handlers for Developer Menu (17 items across 6 pages)
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
    // OLED_Developer - Main menu entry (row 2, page 3)
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

    // ==================== PAGE 1 ====================

    // OLED_Developer_MemoryAdjust - MEM left/right
    else if (oled_menuItem == OLED_Developer_MemoryAdjust) {
        showMenu("Developer", "MEM left/right");
        OSD_handleCommand(OSD_CMD_DEV_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_ResetChip);
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
        OSD_handleCommand(OSD_CMD_DEV_PAGE1_VALUES);

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
        OSD_handleCommand(OSD_CMD_DEV_PAGE1_VALUES);

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

    // ==================== PAGE 2 ====================

    // OLED_Developer_DebugView - Debug view
    else if (oled_menuItem == OLED_Developer_DebugView) {
        showMenu("Developer", "Debug view");
        OSD_handleCommand(OSD_CMD_DEV_PAGE2_VALUES);

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
        OSD_handleCommand(OSD_CMD_DEV_PAGE2_VALUES);

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
        OSD_handleCommand(OSD_CMD_DEV_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_ADCFilter);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_ResyncHTotal);
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

    // ==================== PAGE 3 ====================

    // OLED_Developer_ResyncHTotal - Resync HTotal (command '.')
    else if (oled_menuItem == OLED_Developer_ResyncHTotal) {
        showMenu("Developer", "Resync HTotal");
        OSD_handleCommand(OSD_CMD_DEV_PAGE3_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_FreezeCapture);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_CycleSDRAM);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    serialCommand = '.';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Developer_CycleSDRAM - Cycle SDRAM (command 'l')
    else if (oled_menuItem == OLED_Developer_CycleSDRAM) {
        showMenu("Developer", "Cycle SDRAM");
        OSD_handleCommand(OSD_CMD_DEV_PAGE3_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_ResyncHTotal);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_PLLDivider);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    userCommand = 'l';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Developer_PLLDivider - PLL divider (command 'n' to increase, 'M' to decrease)
    else if (oled_menuItem == OLED_Developer_PLLDivider) {
        showMenu("Developer", "PLL div");
        OSD_handleCommand(OSD_CMD_DEV_PAGE3_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_CycleSDRAM);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_InvertSync);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_OK:
                    serialCommand = 'n';
                    break;
                case IR_KEY_LEFT:
                    serialCommand = 'M';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    // ==================== PAGE 4 ====================

    // OLED_Developer_InvertSync - Invert Sync (command '8')
    else if (oled_menuItem == OLED_Developer_InvertSync) {
        showMenu("Developer", "Invert Sync");
        OSD_handleCommand(OSD_CMD_DEV_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_PLLDivider);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_SyncWatcher);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    serialCommand = '8';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Developer_SyncWatcher - SyncWatcher (command 'm')
    else if (oled_menuItem == OLED_Developer_SyncWatcher) {
        showMenu("Developer", "SyncWatcher");
        OSD_handleCommand(OSD_CMD_DEV_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_InvertSync);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_SyncProcessor);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    serialCommand = 'm';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Developer_SyncProcessor - SyncProcessor (command 'l' action)
    else if (oled_menuItem == OLED_Developer_SyncProcessor) {
        showMenu("Developer", "SyncProcessor");
        OSD_handleCommand(OSD_CMD_DEV_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_SyncWatcher);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_Oversampling);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    serialCommand = 'L';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    // ==================== PAGE 5 ====================

    // OLED_Developer_Oversampling - Oversampling (command 'o')
    else if (oled_menuItem == OLED_Developer_Oversampling) {
        showMenu("Developer", "Oversampling");
        OSD_handleCommand(OSD_CMD_DEV_PAGE5_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_SyncProcessor);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_SnapFrameRate);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    serialCommand = 'o';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Developer_SnapFrameRate - 60/50Hz HDMI (command 'S')
    else if (oled_menuItem == OLED_Developer_SnapFrameRate) {
        showMenu("Developer", "60/50Hz HDMI");
        OSD_handleCommand(OSD_CMD_DEV_PAGE5_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_Oversampling);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_IFAutoOffset);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    serialCommand = 'S';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Developer_IFAutoOffset - IF Auto Offset (command 'E')
    else if (oled_menuItem == OLED_Developer_IFAutoOffset) {
        showMenu("Developer", "IF Auto Offset");
        OSD_handleCommand(OSD_CMD_DEV_PAGE5_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_SnapFrameRate);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_SOGLevel);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    userCommand = 'E';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    // ==================== PAGE 6 ====================

    // OLED_Developer_SOGLevel - SOG Level-- (command 'z')
    else if (oled_menuItem == OLED_Developer_SOGLevel) {
        showMenu("Developer", "SOG Level--");
        OSD_handleCommand(OSD_CMD_DEV_PAGE6_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_IFAutoOffset);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_ResetChip);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    userCommand = 'z';
                    break;
                case IR_KEY_EXIT:
                    Menu_navigateTo(OLED_Developer);
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Developer_ResetChip - Reset Chip (command 'q')
    else if (oled_menuItem == OLED_Developer_ResetChip) {
        showMenu("Developer", "Reset Chip");
        OSD_handleCommand(OSD_CMD_DEV_PAGE6_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    Menu_navigateTo(OLED_Developer);
                    break;
                case IR_KEY_UP:
                    Menu_navigateTo(OLED_Developer_SOGLevel);
                    break;
                case IR_KEY_DOWN:
                    Menu_navigateTo(OLED_Developer_MemoryAdjust);
                    break;
                case IR_KEY_RIGHT:
                case IR_KEY_LEFT:
                case IR_KEY_OK:
                    serialCommand = 'q';
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
