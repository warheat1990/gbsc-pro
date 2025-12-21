// ====================================================================================
// oled-menu-pro.cpp
// OLED Menu Navigation and IR Remote Handling
//
// This file contains:
// - IR_handleMenuSelection(): Main menu state machine for IR remote navigation
// - IR_handleInput(): IR remote input handler and decoder
// - IR_handle*(): Sub-handlers for each menu section (Output, Screen, Color, etc.)
// - OLED_init*Menu(): OLED menu initialization functions
// - OLED_handle*Selection(): OLED menu selection handlers
// ====================================================================================

#include "oled-menu-pro.h"
#include "gbs-control-pro.h"
#include "osd-render-pro.h"
#include "options-pro.h"
#include "../OLEDMenuManager.h"
#include "../OLEDMenuImplementation.h"
#include "../tv5725.h"
#include "../options.h"
#include "../ntsc_720x480.h"

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <SSD1306Wire.h>

#include "drivers/stv9426.h"
#include "drivers/ir_remote.h"
#include "drivers/pt2257.h"

// ====================================================================================
// External References - gbs-control.ino
// ====================================================================================

typedef TV5725<GBS_ADDR> GBS;
extern SSD1306Wire display;
extern struct runTimeOptions *rto;
extern struct userOptions *uopt;
extern char serialCommand;
extern char userCommand;

extern uint8_t getVideoMode();
extern void applyPresets(uint8_t videoMode);
extern void shiftVerticalUpIF();
extern void shiftVerticalDownIF();
extern void saveUserPrefs();
extern void disableMotionAdaptDeinterlace();
extern void disableScanlines();
extern boolean areScanLinesAllowed();
extern float getOutputFrameRate();
extern void writeProgramArrayNew(const uint8_t *programArray, boolean skipMDSection);
extern void doPostPresetLoadSteps();
extern void freezeVideo();

// ====================================================================================
// External References - OLEDMenuImplementation.cpp
// ====================================================================================

extern OLEDMenuManager oledMenu;
extern unsigned long oledMenuFreezeStartTime;
extern unsigned long oledMenuFreezeTimeoutInMS;

// ====================================================================================
// Helper Functions
// ====================================================================================

// Prepare OLED display for drawing (clear if needed, set defaults)
static void oledPrepare() {
    if (oledClearFlag) display.clear();
    oledClearFlag = ~0;
    display.setColor(OLEDDISPLAY_COLOR::WHITE);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
}

// Display menu item on OLED (title at top, label centered)
static void showMenu(const char* title, const char* label) {
    oledPrepare();
    display.drawString(1, 0, title);
    display.drawString(1, 28, label);
    display.display();
}

// Display single centered text on OLED
static void showMenuCentered(const char* text, uint8_t x = 8, uint8_t y = 15) {
    oledPrepare();
    display.drawString(x, y, text);
    display.display();
}

// Display menu with ON/OFF toggle
static void showMenuToggle(const char* title, const char* label, bool isOn) {
    oledPrepare();
    display.drawString(1, 0, title);
    display.drawString(1, 22, label);
    display.drawString(1, 44, isOn ? "ON" : "OFF");
    display.display();
}

// Display menu with custom value text
static void showMenuValue(const char* title, const char* label, const char* value) {
    oledPrepare();
    display.drawString(1, 0, title);
    display.drawString(1, 22, label);
    display.drawString(1, 44, value);
    display.display();
}

// Exit menu completely
static void exitMenu() {
    oled_menuItem = OLED_None;
    OSD_displayOff();   // Turn off OSD before clearing (avoids glitch)
    OSD_clearAll();
    OSD_init();
}

// Decode IR input and set flag
static bool irDecode() {
    if (irrecv.decode(&results)) {
        irDecodedFlag = 1;
        return true;
    }
    return false;
}

// Resume IR receiver after handling
static void irResume() {
    irrecv.resume();
}

// Show selection feedback on OLED with "Loaded" indicator, then freeze menu
// Used by OLED menu handlers for Input/Settings/TV Mode selection
static void showSelectionFeedback(OLEDMenuManager *manager, const char* label) {
    oledMenuFreezeTimeoutInMS = 1000;
    oledMenuFreezeStartTime = millis();
    OLEDDisplay *oledDisplay = manager->getDisplay();
    oledDisplay->clear();
    oledDisplay->setColor(OLEDDISPLAY_COLOR::WHITE);
    oledDisplay->setFont(ArialMT_Plain_16);
    oledDisplay->setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_CENTER);
    oledDisplay->drawString(OLED_MENU_WIDTH / 2, 16, label);
    oledDisplay->drawXbm((OLED_MENU_WIDTH - TEXT_LOADED_WIDTH) / 2, OLED_MENU_HEIGHT / 2, IMAGE_ITEM(TEXT_LOADED));
    oledDisplay->display();
    manager->freeze();
}

// Check if freeze timeout expired and unfreeze if so
// Returns true if still frozen, false if unfrozen
static bool checkFreezeTimeout(OLEDMenuManager *manager) {
    if (millis() - oledMenuFreezeStartTime >= oledMenuFreezeTimeoutInMS) {
        manager->unfreeze();
        return false;
    }
    return true;
}

// ====================================================================================
// Profile Navigation Helpers
// ====================================================================================

// Check if state is in Load range (Load1-Load20), return index 0-19 or -1
static int getLoadSlotIndex(int state) {
    int idx = state - OLED_Profile_Load1;
    return (idx >= 0 && idx < 20) ? idx : -1;
}

// Check if state is in Save range (Save1-Save20), return index 0-19 or -1
static int getSaveSlotIndex(int state) {
    int idx = state - OLED_Profile_Save1;
    return (idx >= 0 && idx < 20) ? idx : -1;
}

// Circular navigation: next slot (wraps 19 -> 0)
static OLED_MenuState getNextSlot(OLED_MenuState base, int idx) {
    return (OLED_MenuState)(base + ((idx + 1) % 20));
}

// Circular navigation: prev slot (wraps 0 -> 19)
static OLED_MenuState getPrevSlot(OLED_MenuState base, int idx) {
    return (OLED_MenuState)(base + ((idx + 19) % 20));
}

// Get slot character 'A'-'T' from index 0-19
static char getSlotChar(int idx) {
    return 'A' + idx;
}

// Handle profile row navigation (unified for Load and Save rows)
// isLoadRow: true = Row 1 (Load), false = Row 2 (Save)
static bool handleProfileRow(bool isLoadRow) {
    int idx = isLoadRow ? getLoadSlotIndex(oled_menuItem) : getSaveSlotIndex(oled_menuItem);
    if (idx < 0) return false;

    OLED_MenuState base = isLoadRow ? OLED_Profile_Load1 : OLED_Profile_Save1;

    // Special case: first Load slot shows menu header
    if (isLoadRow && idx == 0) {
        showMenu("Menu-", "Profile");
    }

    // Visual feedback for up/down navigation
    if (results.value == IR_KEY_UP || results.value == IR_KEY_DOWN) {
        OSD_writeCharAtRow(1, icon4, 0, isLoadRow ? OSD_CURSOR_ACTIVE : OSD_BACKGROUND);
        OSD_writeCharAtRow(2, icon4, 0, isLoadRow ? OSD_BACKGROUND : OSD_CURSOR_ACTIVE);
        OSD_writeCharAtRow(3, icon4, 0, OSD_BACKGROUND);
        if (isLoadRow) OSD_handleCommand(OSD_CMD_PROFILE_SAVELOAD);
    }

    OSD_handleCommand(OSD_CMD_PROFILE_SLOTDISPLAY);

    if (irDecode()) {
        switch (results.value) {
            case IR_KEY_MENU:
                exitMenu();
                break;
            case IR_KEY_UP:
                if (!isLoadRow) {
                    oled_menuItem = OLED_Profile_Load1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_PROFILE_SAVELOAD);
                }
                break;
            case IR_KEY_DOWN:
                if (isLoadRow) {
                    oled_menuItem = OLED_Profile_Save1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_PROFILE_SAVELOAD);
                }
                break;
            case IR_KEY_RIGHT:
                oled_menuItem = getNextSlot(base, idx);
                break;
            case IR_KEY_LEFT:
                oled_menuItem = getPrevSlot(base, idx);
                break;
            case IR_KEY_OK:
                uopt->presetSlot = getSlotChar(idx);
                if (isLoadRow) {
                    OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW1);
                } else {
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    OSD_showOkFeedback(ROW_2);
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

// ====================================================================================
// IR Menu Handlers - Output Resolution
// ====================================================================================

static bool IR_handleOutputResolution(void)
{
    if (oled_menuItem == OLED_OutputResolution) {
        showMenu("Menu->>>", "Output Resolution");

        if (results.value == IR_KEY_UP || results.value == IR_KEY_DOWN) {
            selectedMenuLine = 2;
            OSD_highlightIcon(2);
            OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
                    oled_menuItem = OLED_Input;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
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
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
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
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
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
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
                    break;
                case IR_KEY_OK:
                    userCommand = 'f';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
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
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
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
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
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
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
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
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
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

                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
                    oled_menuItem = OLED_OutputResolution_PassThrough;
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
                    oled_menuItem = OLED_OutputResolution_PassThrough;
                    break;
            }
            irResume();
        }
        return true;
    }

    // TODO: Re-enable when Downscale feature is implemented
    // if (oled_menuItem == OLED_OutputResolution_Downscale) {
    //     showMenu("Menu->Output", "Downscale");
    //
    //     if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
    //         OSD_highlightIcon(3);
    //         OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
    //     }
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IR_KEY_MENU:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
    //                 oled_menuItem = OLED_OutputResolution;
    //                 break;
    //             case IR_KEY_UP:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
    //                 oled_menuItem = OLED_OutputResolution_480;
    //                 break;
    //             case IR_KEY_DOWN:
    //                 oled_menuItem = OLED_OutputResolution_PassThrough;
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_OUTPUT_PASSTHROUGH);
    //                 break;
    //             case IR_KEY_OK:
    //                 userCommand = 'L';
    //                 break;
    //             case IR_KEY_EXIT:
    //                 exitMenu();
    //                 break;
    //         }
    //         irResume();
    //     }
    //     return true;
    // }

    return false;
}

// ====================================================================================
// IR Menu Handlers - Screen Settings
// ====================================================================================

static bool IR_handleScreenSettings(void)
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

// ====================================================================================
// IR_handleColorSettings - Color Settings Menu Handlers
// ====================================================================================

static bool IR_handleColorSettings()
{
    // OLED_ColorSettings_ADCGain
    if (oled_menuItem == OLED_ColorSettings_ADCGain) {
        showMenuToggle("Menu->Color", "ADC gain", uopt->enableAutoGain);

        if (results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    oled_menuItem = OLED_ColorSettings_RGB_B;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    oled_menuItem = OLED_ColorSettings_Scanlines;
                    break;
                case IR_KEY_RIGHT:
                    userCommand = 'n';
                    break;
                case IR_KEY_LEFT:
                    userCommand = 'o';
                    break;
                case IR_KEY_OK:
                    serialCommand = 'T';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_Scanlines
    else if (oled_menuItem == OLED_ColorSettings_Scanlines) {
        boolean scanlinesAllowed = areScanLinesAllowed();
        showMenuToggle("Menu->Color", "Scanlines", uopt->wantScanlines);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    oled_menuItem = OLED_ColorSettings_ADCGain;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    oled_menuItem = OLED_ColorSettings_LineFilter;
                    break;
                case IR_KEY_RIGHT:
                    if (scanlinesAllowed) {
                        userCommand = 'K';
                    }
                    break;
                case IR_KEY_LEFT:
                    if (scanlinesAllowed) {
                        userCommand = 'K';
                    }
                    break;
                case IR_KEY_OK:
                    if (scanlinesAllowed) {
                        userCommand = '7';
                    }
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_LineFilter
    else if (oled_menuItem == OLED_ColorSettings_LineFilter) {
        showMenuToggle("Menu->Color", "Line filter", uopt->wantVdsLineFilter);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    oled_menuItem = OLED_ColorSettings_Scanlines;
                    break;
                case IR_KEY_DOWN:
                    oled_menuItem = OLED_ColorSettings_Sharpness;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    break;
                case IR_KEY_OK:
                    userCommand = 'm';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_Sharpness
    else if (oled_menuItem == OLED_ColorSettings_Sharpness) {
        showMenuToggle("Menu->Color", "Sharpness", GBS::VDS_PK_LB_GAIN::read() != 0x16);

        if (results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    oled_menuItem = OLED_ColorSettings_LineFilter;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    oled_menuItem = OLED_ColorSettings_Peaking;
                    break;
                case IR_KEY_OK:
                    userCommand = 'W';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_Peaking
    else if (oled_menuItem == OLED_ColorSettings_Peaking) {
        if (isPeakingLocked()) {
            showMenuValue("Menu->Color", "Peaking", "LOCKED");
        } else {
            showMenuToggle("Menu->Color", "Peaking", uopt->wantPeaking);
        }

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    oled_menuItem = OLED_ColorSettings_Sharpness;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    oled_menuItem = OLED_ColorSettings_StepResponse;
                    break;
                case IR_KEY_OK:
                    if (!isPeakingLocked()) {
                        serialCommand = 'f';
                    }
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_StepResponse
    else if (oled_menuItem == OLED_ColorSettings_StepResponse) {
        showMenuToggle("Menu->Color", "Step response", uopt->wantStepResponse);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    oled_menuItem = OLED_ColorSettings_Peaking;
                    break;
                case IR_KEY_DOWN:
                    oled_menuItem = OLED_ColorSettings_DefaultColor;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
                    break;
                case IR_KEY_OK:
                    serialCommand = 'V';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_RGB_R
    else if (oled_menuItem == OLED_ColorSettings_RGB_R) {
        showMenu("Menu->Color", "R ");

        if (results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
        }
        OSD_handleCommand(OSD_CMD_COLOR_RGB_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    oled_menuItem = OLED_ColorSettings_RGB_G;
                    break;
                case IR_KEY_RIGHT:
                    R_VAL = MIN(R_VAL + STEP, 255);
                    applyRGBtoYUVConversion();
                    break;
                case IR_KEY_LEFT:
                    R_VAL = MAX(0, R_VAL - STEP);
                    applyRGBtoYUVConversion();
                    break;
                case IR_KEY_OK:
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_RGB_G
    else if (oled_menuItem == OLED_ColorSettings_RGB_G) {
        if (uopt->enableAutoGain == 1) {
            uopt->enableAutoGain = 0;
            saveUserPrefs();
        } else {
            uopt->enableAutoGain = 0;
        }
        showMenu("Menu->Color", "G ");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
        }
        OSD_handleCommand(OSD_CMD_COLOR_RGB_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    oled_menuItem = OLED_ColorSettings_RGB_R;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    oled_menuItem = OLED_ColorSettings_RGB_B;
                    break;
                case IR_KEY_RIGHT:
                    G_VAL = MIN(G_VAL + STEP, 255);
                    applyRGBtoYUVConversion();
                    break;
                case IR_KEY_LEFT:
                    G_VAL = MAX(0, G_VAL - STEP);
                    applyRGBtoYUVConversion();
                    break;
                case IR_KEY_OK:
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_RGB_B
    else if (oled_menuItem == OLED_ColorSettings_RGB_B) {
        showMenu("Menu->Color", "B");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
        }
        OSD_handleCommand(OSD_CMD_COLOR_RGB_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    oled_menuItem = OLED_ColorSettings_RGB_G;
                    break;
                case IR_KEY_DOWN:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    oled_menuItem = OLED_ColorSettings_ADCGain;
                    break;
                case IR_KEY_RIGHT:
                    B_VAL = MIN(B_VAL + STEP, 255);
                    applyRGBtoYUVConversion();
                    break;
                case IR_KEY_LEFT:
                    B_VAL = MAX(0, B_VAL - STEP);
                    applyRGBtoYUVConversion();
                    break;
                case IR_KEY_OK:
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_Y_Gain
    else if (oled_menuItem == OLED_ColorSettings_Y_Gain) {
        uint8_t cur = GBS::VDS_Y_GAIN::read();
        showMenu("Menu->Color", "Y gain");

        if (results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
        }

        OSD_handleCommand(OSD_CMD_SYS_SVINPUT_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    oled_menuItem = OLED_ColorSettings_RGB_B;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
                    oled_menuItem = OLED_ColorSettings_Color;
                    break;
                case IR_KEY_RIGHT:
                    cur = MIN(cur + STEP, 255);
                    GBS::VDS_Y_GAIN::write(cur);
                    break;
                case IR_KEY_LEFT:
                    cur = MAX(0, cur - STEP);
                    GBS::VDS_Y_GAIN::write(cur);
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_Color
    else if (oled_menuItem == OLED_ColorSettings_Color) {
        showMenu("Menu->Color", "Color");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
        }
        OSD_handleCommand(OSD_CMD_SYS_SVINPUT_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
                    oled_menuItem = OLED_ColorSettings_Y_Gain;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
                    oled_menuItem = OLED_ColorSettings_DefaultColor;
                    break;
                case IR_KEY_RIGHT:
                    userCommand = 'V';
                    break;
                case IR_KEY_LEFT:
                    userCommand = 'R';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_ColorSettings_DefaultColor
    else if (oled_menuItem == OLED_ColorSettings_DefaultColor) {
        showMenu("Menu->Color", "Default Color");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    oled_menuItem = OLED_ColorSettings_StepResponse;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    break;
                case IR_KEY_OK:
                    userCommand = 'U';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}

// ====================================================================================
// IR_handleSystemSettings - System Settings Menu Handlers
// ====================================================================================

static bool IR_handleSystemSettings()
{
    // OLED_SystemSettings_MatchedPresets
    if (oled_menuItem == OLED_SystemSettings_MatchedPresets) {
        showMenuToggle("Menu->System", "Matched presets", uopt->matchPresetSource);

        if (results.value == IR_KEY_UP || results.value == IR_KEY_DOWN) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_SYS_PAGE1);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_Compatibility;
                    break;
                case IR_KEY_DOWN:
                    oled_menuItem = OLED_SystemSettings_Deinterlace;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    break;
                case IR_KEY_OK:
                    serialCommand = 'Z';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_UseUpscaling
    else if (oled_menuItem == OLED_SystemSettings_UseUpscaling) {
        showMenuToggle("Menu->System", "Use upscaling", uopt->preferScalingRgbhv);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_SYS_PAGE1);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_Compatibility;
                    break;
                case IR_KEY_DOWN:
                    oled_menuItem = OLED_SystemSettings_Deinterlace;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_Force5060Hz
    else if (oled_menuItem == OLED_SystemSettings_Force5060Hz) {
        showMenuToggle("Menu->System", "Force 50 / 60Hz", uopt->PalForce60);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    oled_menuItem = OLED_SystemSettings_Deinterlace;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    oled_menuItem = OLED_SystemSettings_LockMethod;
                    break;
                case IR_KEY_OK:
                    if (uopt->PalForce60 == 0) {
                        uopt->PalForce60 = 1;
                    } else {
                        uopt->PalForce60 = 0;
                    }
                    saveUserPrefs();
                    applyVideoModePreset();
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_ClockGenerator
    else if (oled_menuItem == OLED_SystemSettings_ClockGenerator) {
        showMenuToggle("Menu->System", "Clock generator", !uopt->disableExternalClockGenerator);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_SYS_PAGE4);
        }
        OSD_handleCommand(OSD_CMD_SYS_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE4);
                    oled_menuItem = OLED_SystemSettings_FrameTimeLock;
                    break;
                case IR_KEY_OK:
                    userCommand = 'X';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_ADCCalibration
    else if (oled_menuItem == OLED_SystemSettings_ADCCalibration) {
        showMenuToggle("Menu->System", "ADC calibration", uopt->enableCalibrationADC);

        if (results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_SYS_PAGE4);
        }
        OSD_handleCommand(OSD_CMD_SYS_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    oled_menuItem = OLED_SystemSettings_LockMethod;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE4);
                    oled_menuItem = OLED_SystemSettings_FrameTimeLock;
                    break;
                case IR_KEY_OK:
                    userCommand = 'w';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_FrameTimeLock
    else if (oled_menuItem == OLED_SystemSettings_FrameTimeLock) {
        showMenuToggle("Menu->System", "Frame Time Lock", uopt->enableFrameTimeLock);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE4);
                    oled_menuItem = OLED_SystemSettings_ADCCalibration;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE4);
                    oled_menuItem = OLED_SystemSettings_ClockGenerator;
                    break;
                case IR_KEY_OK:
                    userCommand = '5';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_LockMethod
    else if (oled_menuItem == OLED_SystemSettings_LockMethod) {
        showMenuValue("Menu->System", "Lock Method",
                      uopt->frameTimeLockMethod ? "1Vtotal only" : "0Vtotal+VSST");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_SYS_PAGE2);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    oled_menuItem = OLED_SystemSettings_Force5060Hz;
                    break;
                case IR_KEY_DOWN:
                    oled_menuItem = OLED_SystemSettings_ADCCalibration;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE4);
                    break;
                case IR_KEY_OK:
                    userCommand = 'i';
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_Deinterlace
    else if (oled_menuItem == OLED_SystemSettings_Deinterlace) {
        showMenuValue("Menu->System", "Deinterlace", uopt->deintMode ? "Bob" : "Adaptive");

        if (results.value == IR_KEY_UP || results.value == IR_KEY_DOWN) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_SYS_PAGE2);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    oled_menuItem = OLED_SystemSettings_MatchedPresets;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    oled_menuItem = OLED_SystemSettings_Force5060Hz;
                    break;
                case IR_KEY_OK:
                    if (uopt->deintMode != 1) {
                        uopt->deintMode = 1;
                        disableMotionAdaptDeinterlace();
                        if (GBS::GBS_OPTION_SCANLINES_ENABLED::read()) {
                            disableScanlines();
                        }
                        saveUserPrefs();
                    } else if (uopt->deintMode != 0) {
                        uopt->deintMode = 0;
                        saveUserPrefs();
                    }
                    applyVideoModePreset();
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInputSettings
    else if (oled_menuItem == OLED_SystemSettings_SVAVInputSettings) {
        showMenu("Menu->System", "Sv-Av InputSet");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_SYS_PAGE1);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_OK:
                    if (inputType == InputTypeSV || inputType == InputTypeAV) {
                        selectedMenuLine = 1;
                        OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                        OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
                        oled_menuItem = OLED_SystemSettings_SVAVInput_DoubleLine;
                    }
                    break;
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_Compatibility;
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_DoubleLine
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_DoubleLine) {
        showMenuValue("M>Sys>SvAv Set", "DoubleLine", lineOption ? "2X" : "1X");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
        }
        OSD_handleCommand(OSD_CMD_ADCCALIB_DISPLAY);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Smooth;
                    break;
                case IR_KEY_OK:
                    lineOption = !lineOption;
                    settingLineOptionChanged = 1;
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_Smooth
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Smooth) {
        showMenuToggle("M>Sys>SvAv Set", "Smooth", smoothOption);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
            OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
        }
        OSD_handleCommand(OSD_CMD_ADCCALIB_DISPLAY);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_DoubleLine;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Bright;
                    break;
                case IR_KEY_OK:
                    if (lineOption) {
                        smoothOption = !smoothOption;
                        settingSmoothOptionChanged = 1;
                    }
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_Bright
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Bright) {
        showMenu("M>Sys>SvAv Set", "Bright");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
        }
        OSD_handleCommand(OSD_CMD_ADCCALIB_DISPLAY);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Smooth;
                    break;
                case IR_KEY_DOWN:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Contrast;
                    break;
                case IR_KEY_RIGHT:
                    brightness = MIN(brightness + STEP, 254);
                    ADV_sendBCSH(0x0a, brightness - 128);
                    break;
                case IR_KEY_LEFT:
                    brightness = MAX(brightness - STEP, 0);
                    ADV_sendBCSH(0x0a, brightness - 128);
                    break;
                case IR_KEY_OK:
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    saveUserPrefs();
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_Contrast
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Contrast) {
        showMenu("M>Sys>SvAv Set", "Contrast");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
        }
        OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW3);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Bright;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Saturation;
                    break;
                case IR_KEY_RIGHT:
                    contrast = MIN(contrast + STEP, 254);
                    ADV_sendBCSH(0x08, contrast);
                    break;
                case IR_KEY_LEFT:
                    contrast = MAX(contrast - STEP, 0);
                    ADV_sendBCSH(0x08, contrast);
                    break;
                case IR_KEY_OK:
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_Saturation
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Saturation) {
        showMenu("M>Sys>SvAv Set", "Saturation");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
            OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
        }
        OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW3);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Contrast;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Default;
                    break;
                case IR_KEY_RIGHT:
                    saturation = MIN(saturation + STEP, 254);
                    ADV_sendBCSH(0xe3, saturation);
                    break;
                case IR_KEY_LEFT:
                    saturation = MAX(saturation - STEP, 0);
                    ADV_sendBCSH(0xe3, saturation);
                    break;
                case IR_KEY_OK:
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_SVAVInput_Default
    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Default) {
        showMenu("M>Sys>SvAv Set", "Default");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(3);
            OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
        }
        OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW3);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Saturation;
                    break;
                case IR_KEY_OK:
                    ADV_sendBCSH('D', 'E');
                    brightness = 128;
                    contrast = 128;
                    saturation = 128;
                    saveUserPrefs();
                    break;
                case IR_KEY_EXIT:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_Compatibility
    else if (oled_menuItem == OLED_SystemSettings_Compatibility) {
        showMenuToggle("Menu->System", "Compatibility", rgbComponentMode == 1);

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            OSD_highlightIcon(2);
            OSD_handleCommand(OSD_CMD_SYS_PAGE1);
        }
        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_UP:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_MatchedPresets;
                    break;
                case IR_KEY_OK:
                    rgbComponentMode = !rgbComponentMode;
                    if (rgbComponentMode > 1)
                        rgbComponentMode = 0;
                    ADV_sendCompatibility(rgbComponentMode);
                    if (GBS::ADC_INPUT_SEL::read())
                        applyVideoModePreset();
                    break;
                case IR_KEY_EXIT:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings_ComponentVGA (disabled)
    // else if (oled_menuItem == OLED_SystemSettings_ComponentVGA) {
    //     showMenu("Menu->System", "Component/VGA");
    //
    //     if (results.value == IR_KEY_UP) {
    //         OSD_highlightIcon(1);
    //         OSD_handleCommand(OSD_CMD_SYS_PAGE2);
    //     }
    //     OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IR_KEY_MENU:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_SystemSettings;
    //                 break;
    //             case IR_KEY_UP:
    //                 oled_menuItem = OLED_SystemSettings_UseUpscaling;
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
    //                 OSD_handleCommand(OSD_CMD_SYS_PAGE1);
    //                 break;
    //             case IR_KEY_DOWN:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_SYS_PAGE2);
    //                 oled_menuItem = OLED_SystemSettings_Force5060Hz;
    //                 break;
    //             case IR_KEY_RIGHT:
    //                 serialCommand = 'L';
    //                 break;
    //             case IR_KEY_LEFT:
    //                 serialCommand = 'L';
    //                 break;
    //             case IR_KEY_EXIT:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
    //                 oled_menuItem = OLED_Input;
    //                 break;
    //         }
    //         irResume();
    //     }
    //     return true;
    // }

    // OLED_Developer (disabled)
    // if (oled_menuItem == OLED_Developer) {
    //     showMenu("Menu->>>", "Developer");
    //
    //     if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
    //         OSD_highlightIcon(2);
    //     }
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IR_KEY_UP:
    //                 selectedMenuLine = 1;
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_SystemSettings;
    //                 break;
    //             case IR_KEY_DOWN:
    //                 selectedMenuLine = 3;
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_ResetSettings;
    //                 break;
    //             case IR_KEY_OK:
    //                 oled_menuItem = OLED_Developer_MemoryAdjust;
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //                 break;
    //             case IR_KEY_EXIT:
    //                 exitMenu();
    //                 break;
    //         }
    //         irResume();
    //     }
    //     return true;
    // }

    // OLED_Developer_MemoryAdjust (disabled)
    // else if (oled_menuItem == OLED_Developer_MemoryAdjust) {
    //     showMenu("Menu->Dev", "MEM left/right");
    //
    //     if (results.value == IR_KEY_UP) {
    //         OSD_highlightIcon(1);
    //         OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //     }
    //     OSD_handleCommand(OSD_CMD_DEV_MEMORY_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IR_KEY_MENU:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_Developer;
    //                 break;
    //             case IR_KEY_DOWN:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //                 oled_menuItem = OLED_Developer_HSyncAdjust;
    //                 break;
    //             case IR_KEY_RIGHT:
    //                 serialCommand = '+';
    //                 break;
    //             case IR_KEY_LEFT:
    //                 serialCommand = '-';
    //                 break;
    //             case IR_KEY_EXIT:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
    //                 oled_menuItem = OLED_Input;
    //                 break;
    //         }
    //         irResume();
    //     }
    //     return true;
    // }

    // OLED_Developer_HSyncAdjust (disabled)
    // else if (oled_menuItem == OLED_Developer_HSyncAdjust) {
    //     showMenu("Menu->Dev", "HS left/right");
    //
    //     if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
    //         OSD_highlightIcon(2);
    //     }
    //     OSD_handleCommand(OSD_CMD_DEV_MEMORY_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IR_KEY_MENU:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_Developer;
    //                 break;
    //             case IR_KEY_UP:
    //                 selectedMenuLine = 1;
    //                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //                 oled_menuItem = OLED_Developer_MemoryAdjust;
    //                 break;
    //             case IR_KEY_DOWN:
    //                 selectedMenuLine = 3;
    //                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //                 oled_menuItem = OLED_Developer_HTotalAdjust;
    //                 break;
    //             case IR_KEY_RIGHT:
    //                 serialCommand = '0';
    //                 break;
    //             case IR_KEY_LEFT:
    //                 serialCommand = '1';
    //                 break;
    //             case IR_KEY_EXIT:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
    //                 oled_menuItem = OLED_Input;
    //                 break;
    //         }
    //         irResume();
    //     }
    //     return true;
    // }

    // OLED_Developer_HTotalAdjust (disabled)
    // else if (oled_menuItem == OLED_Developer_HTotalAdjust) {
    //     showMenu("Menu->Dev", "HTotal -/+");
    //
    //     if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
    //         OSD_highlightIcon(3);
    //         OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //     }
    //     OSD_handleCommand(OSD_CMD_DEV_MEMORY_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IR_KEY_MENU:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_Developer;
    //                 break;
    //             case IR_KEY_UP:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //                 oled_menuItem = OLED_Developer_HSyncAdjust;
    //                 break;
    //             case IR_KEY_DOWN:
    //                 oled_menuItem = OLED_Developer_DebugView;
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_DEV_DEBUG);
    //                 break;
    //             case IR_KEY_RIGHT:
    //                 serialCommand = 'a';
    //                 break;
    //             case IR_KEY_LEFT:
    //                 serialCommand = 'A';
    //                 break;
    //             case IR_KEY_EXIT:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
    //                 oled_menuItem = OLED_Input;
    //                 break;
    //         }
    //         irResume();
    //     }
    //     return true;
    // }

    // OLED_Developer_DebugView (disabled)
    // else if (oled_menuItem == OLED_Developer_DebugView) {
    //     showMenu("Menu->Dev", "Debug view");
    //
    //     if (results.value == IR_KEY_UP) {
    //         OSD_highlightIcon(1);
    //         OSD_handleCommand(OSD_CMD_DEV_DEBUG);
    //     }
    //     OSD_handleCommand(OSD_CMD_DEV_DEBUG_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IR_KEY_MENU:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_Developer;
    //                 break;
    //             case IR_KEY_UP:
    //                 oled_menuItem = OLED_Developer_HTotalAdjust;
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
    //                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //                 break;
    //             case IR_KEY_DOWN:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_DEV_DEBUG);
    //                 oled_menuItem = OLED_Developer_ADCFilter;
    //                 break;
    //             case IR_KEY_RIGHT:
    //                 serialCommand = 'D';
    //                 break;
    //             case IR_KEY_LEFT:
    //                 serialCommand = 'D';
    //                 break;
    //             case IR_KEY_EXIT:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
    //                 oled_menuItem = OLED_Input;
    //                 break;
    //         }
    //         irResume();
    //     }
    //     return true;
    // }

    // OLED_Developer_ADCFilter (disabled)
    // else if (oled_menuItem == OLED_Developer_ADCFilter) {
    //     showMenu("Menu->Dev", "ADC filter");
    //
    //     if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
    //         OSD_highlightIcon(2);
    //     }
    //     OSD_handleCommand(OSD_CMD_DEV_DEBUG_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IR_KEY_MENU:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_Developer;
    //                 break;
    //             case IR_KEY_UP:
    //                 selectedMenuLine = 1;
    //                 OSD_handleCommand(OSD_CMD_DEV_DEBUG);
    //                 oled_menuItem = OLED_Developer_DebugView;
    //                 break;
    //             case IR_KEY_DOWN:
    //                 selectedMenuLine = 3;
    //                 OSD_handleCommand(OSD_CMD_DEV_DEBUG);
    //                 oled_menuItem = OLED_Developer_FreezeCapture;
    //                 break;
    //             case IR_KEY_RIGHT:
    //                 serialCommand = 'F';
    //                 break;
    //             case IR_KEY_LEFT:
    //                 serialCommand = 'F';
    //                 break;
    //             case IR_KEY_EXIT:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
    //                 oled_menuItem = OLED_Input;
    //                 break;
    //         }
    //         irResume();
    //     }
    //     return true;
    // }

    // OLED_Developer_FreezeCapture (disabled)
    // else if (oled_menuItem == OLED_Developer_FreezeCapture) {
    //     showMenu("Menu->Dev", "Freeze capture");
    //
    //     if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
    //         OSD_highlightIcon(3);
    //         OSD_handleCommand(OSD_CMD_DEV_DEBUG);
    //     }
    //     OSD_handleCommand(OSD_CMD_DEV_DEBUG_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IR_KEY_MENU:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_Developer;
    //                 break;
    //             case IR_KEY_UP:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_DEV_DEBUG);
    //                 oled_menuItem = OLED_Developer_ADCFilter;
    //                 break;
    //             case IR_KEY_DOWN:
    //                 oled_menuItem = OLED_Developer_MemoryAdjust;
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //                 break;
    //             case IR_KEY_RIGHT:
    //                 userCommand = 'F';
    //                 break;
    //             case IR_KEY_LEFT:
    //                 userCommand = 'F';
    //                 break;
    //             case IR_KEY_EXIT:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
    //                 oled_menuItem = OLED_Input;
    //                 break;
    //         }
    //         irResume();
    //     }
    //     return true;
    // }

    return false;
}

// ====================================================================================
// IR_handleInputSelection - Input Selection Menu Handlers
// ====================================================================================

static bool IR_handleInputSelection()
{
    // OLED_Input
    if (oled_menuItem == OLED_Input) {
        showMenu("Menu->>>", "Input");

        if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
            selectedMenuLine = 1;
            OSD_highlightIcon(1);
            OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
        }

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_MENU:
                    exitMenu();
                    break;
                case IR_KEY_DOWN:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
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

    // OLED_Input_RGBs
    else if (oled_menuItem == OLED_Input_RGBs) {
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
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
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
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
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
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
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
        OSD_handleCommand(OSD_CMD_INPUT_FORMAT);
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
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
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
        OSD_handleCommand(OSD_CMD_INPUT_FORMAT);
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
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
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
        OSD_handleCommand(OSD_CMD_INPUT_FORMAT);
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
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
                    oled_menuItem = OLED_Input;
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}
// ====================================================================================
// IR_handleProfileManagement - Profile Management Menu Handlers
// ====================================================================================

static bool IR_handleProfileManagement()
{
    // Row 1: Load profile - loads preset from selected slot
    if (handleProfileRow(true)) return true;

    // Row 2: Save profile - saves current settings to selected slot
    if (handleProfileRow(false)) return true;

    return false;
}

// ====================================================================================
// IR_handleMainMenu - Main Menu Navigation Handlers
// ====================================================================================

static bool IR_handleMainMenu()
{
    // OLED_ScreenSettings - Main menu entry
    if (oled_menuItem == OLED_ScreenSettings) {
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
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
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

    // OLED_ColorSettings - Main menu entry
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
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    break;
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_SystemSettings - Main menu entry
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
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
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

    // OLED_ResetSettings - Main menu entry
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

    // OLED_EnableOTA (disabled)
    // else if (oled_menuItem == OLED_EnableOTA) {
    //     showMenu("Menu->Reset", "Enable OTA");
    //
    //     if (results.value == IR_KEY_UP) {
    //         OSD_highlightIcon(1);
    //         OSD_handleCommand(OSD_CMD_SYS_PAGE5);
    //     }
    //     OSD_handleCommand(OSD_CMD_SYS_PAGE5_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IR_KEY_MENU:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_ResetSettings;
    //                 break;
    //             case IR_KEY_DOWN:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_SYS_PAGE5);
    //                 oled_menuItem = OLED_Restart;
    //                 break;
    //             case IR_KEY_RIGHT:
    //                 serialCommand = 'c';
    //                 break;
    //             case IR_KEY_LEFT:
    //                 serialCommand = 'c';
    //                 break;
    //             case IR_KEY_EXIT:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
    //                 oled_menuItem = OLED_Input;
    //                 break;
    //         }
    //         irResume();
    //     }
    //     return true;
    // }

    // OLED_ResetDefaults (disabled)
    // else if (oled_menuItem == OLED_ResetDefaults) {
    //     showMenu("Menu->Reset", "Reset defaults");
    //
    //     if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
    //         OSD_highlightIcon(3);
    //     }
    //     OSD_handleCommand(OSD_CMD_SYS_PAGE5_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IR_KEY_MENU:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_ResetSettings;
    //                 break;
    //             case IR_KEY_UP:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_SYS_PAGE5);
    //                 oled_menuItem = OLED_Restart;
    //                 break;
    //             case IR_KEY_OK:
    //                 userCommand = '1';
    //                 break;
    //             case IR_KEY_EXIT:
    //                 exitMenu();
    //                 break;
    //         }
    //         irResume();
    //     }
    //     return true;
    // }

    return false;
}

// ====================================================================================
// IR_handleMiscSettings - Volume, Info Display Handlers
// ====================================================================================

static bool IR_handleMiscSettings()
{
    // OLED_Volume_Adjust
    if (oled_menuItem == OLED_Volume_Adjust) {
        showMenuCentered("Volume - / + dB");

        // TV OSD display
        osdDisplayValue = 50 - volume;
        OSD_writeStringAtRow(1, 1, "Line input volume", OSD_TEXT_SELECTED);
        // Display 2-digit volume value at positions 20-21 (0-50 range)
        OSD_writeCharAtRow(1, digitChars[osdDisplayValue / 10], 20, OSD_TEXT_NORMAL);  // tens
        OSD_writeCharAtRow(1, digitChars[osdDisplayValue % 10], 21, OSD_TEXT_NORMAL);  // units

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_VOL_UP:
                    volume = MAX(volume - 1, 0);
                    osdDisplayValue = 50 - volume;
                    PT2257_setAttenuation(volume);
                    break;
                case IR_KEY_VOL_DN:
                    volume = MIN(volume + 1, 50);
                    osdDisplayValue = 50 - volume;
                    PT2257_setAttenuation(volume);
                    break;
                case IR_KEY_MENU:
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
                case IR_KEY_OK:
                    saveUserPrefs();
                    OSD_showSavingFeedback(ROW_1);
                    break;
            }
            irResume();
        }
        return true;
    }

    return false;
}

// ====================================================================================
// IR_handleInfoDisplay - Info Display Handler
// ====================================================================================

// Helper: Get output resolution string by presetID
static const char* getOutputResolutionName(uint8_t presetID) {
    switch (presetID) {
        case 0x01: case 0x11: return "1280x960";
        case 0x02: case 0x12: return "1280x1024";
        case 0x03: case 0x13: return "1280x720";
        case 0x05: case 0x15: return "1920x1080";
        case 0x06: case 0x16: return "Downscale";
        case 0x04:            return "720x480";
        case 0x14:            return "768x576";
        default:              return "Bypass";
    }
}

// Helper: Get input type string
static const char* getInputTypeName(uint8_t type) {
    switch (type) {
        case InputTypeRGBs: return "RGBs";
        case InputTypeRGsB: return "RGsB";
        case InputTypeVGA:  return "VGA";
        case InputTypeYUV:  return "YPBPR";
        case InputTypeSV:   return "SV";
        case InputTypeAV:   return "AV";
        default:            return "";
    }
}

static bool IR_handleInfoDisplay()
{
    if (oled_menuItem != OLED_Info_Display) {
        return false;
    }

    showMenu("Menu-", "Info");

    boolean vsyncActive = 0;
    boolean hsyncActive = 0;
    float ofr = getOutputFrameRate();
    uint8_t currentInput = GBS::ADC_INPUT_SEL::read();
    rto->presetID = GBS::GBS_PRESET_ID::read();

    // Row 1: Info header
    OSD_writeStringAtRow(1, 0, "Info:", OSD_CURSOR_ACTIVE);
    OSD_writeStringAtRow(1, 26, "Hz", OSD_TEXT_NORMAL);

    // Row 1: Output resolution (positions 6-14) - clear then write
    OSD_clearRowContent(ROW_1, 15, 6);
    OSD_writeStringAtRow(1, 6, getOutputResolutionName(rto->presetID), OSD_TEXT_NORMAL);

    // Row 1: Input type (positions 17-22) - clear then write
    OSD_clearRowContent(ROW_1, 23, 17);
    OSD_writeStringAtRow(1, 18, getInputTypeName(inputType), OSD_TEXT_NORMAL);

    // Row 1: Frame rate
    // Display frame rate (2 digits, 0-99 Hz range)
    osdDisplayValue = ofr;
    OSD_writeCharAtRow(1, digitChars[osdDisplayValue / 10], 24, OSD_TEXT_NORMAL);  // tens
    OSD_writeCharAtRow(1, digitChars[osdDisplayValue % 10], 25, OSD_TEXT_NORMAL);  // units

    OSD_writeStringAtRow(2, 0, "Current:", OSD_CURSOR_ACTIVE);
    OSD_writeStringAtRow(2, 0xFF, " ", OSD_TEXT_NORMAL);

    if ((rto->sourceDisconnected || !rto->boardHasPower || isInfoDisplayActive == 1)) {
        OSD_writeStringAtRow(2, 0xFF, "No Input", OSD_TEXT_NORMAL);
    } else if (((currentInput == 1) || (inputType == InputTypeRGBs || inputType == InputTypeRGsB || inputType == InputTypeVGA))) {
        OSD_writeCharAtRow(2, 'B', 16, OSD_BACKGROUND);
        OSD_writeStringAtRow(2, 0xFF, "RGB ", OSD_TEXT_NORMAL);
        vsyncActive = GBS::STATUS_SYNC_PROC_VSACT::read();
        if (vsyncActive) {
            hsyncActive = GBS::STATUS_SYNC_PROC_HSACT::read();
            if (hsyncActive) {
                OSD_writeStringAtRow(2, 0xFF, "HV   ", OSD_TEXT_NORMAL);
            }
        } else if ((inputType == InputTypeVGA) && ((!vsyncActive || !hsyncActive))) {
            OSD_writeCharAtRow(2, 'B', 11, OSD_BACKGROUND);
            OSD_writeStringAtRow(2, 0x09, "No Input", OSD_TEXT_NORMAL);
        }
    } else if ((rto->continousStableCounter > 35 || currentInput != 1) || (inputType == InputTypeYUV || inputType == InputTypeSV || inputType == InputTypeAV)) {
        OSD_writeCharAtRow(2, 'B', 16, OSD_BACKGROUND);
        if (inputType == InputTypeYUV)
            OSD_writeStringAtRow(2, 0xFF, "  YPBPR  ", OSD_TEXT_NORMAL);
        else if (inputType == InputTypeSV)
            OSD_writeStringAtRow(2, 0xFF, "   SV    ", OSD_TEXT_NORMAL);
        else if (inputType == InputTypeAV)
            OSD_writeStringAtRow(2, 0xFF, "   AV    ", OSD_TEXT_NORMAL);
    } else {
        OSD_writeStringAtRow(2, 0xFF, "No Input", OSD_TEXT_NORMAL);
    }

    // Show resolution only if input is connected
    if (!rto->sourceDisconnected && rto->boardHasPower) {
        static uint8_t S0_Read_Resolution;
        static unsigned long Tim_info = 0;
        if ((millis() - Tim_info) >= 1000) {
            S0_Read_Resolution = GBS::STATUS_00::read();
            Tim_info = millis();
        }

        if (S0_Read_Resolution & 0x80) {
            if (S0_Read_Resolution & 0x40) {
                OSD_writeStringAtRow(2, 0xFF, "   576p", OSD_TEXT_NORMAL);
            } else if (S0_Read_Resolution & 0x20) {
                if (abs(GBS::STATUS_SYNC_PROC_VTOTAL::read() - 312) <= 10)
                    OSD_writeStringAtRow(2, 0xFF, "   288p", OSD_TEXT_NORMAL);
                else
                    OSD_writeStringAtRow(2, 0xFF, "   576i", OSD_TEXT_NORMAL);
            } else if (S0_Read_Resolution & 0x10) {
                OSD_writeStringAtRow(2, 0xFF, "   480p", OSD_TEXT_NORMAL);
            } else if (S0_Read_Resolution & 0x08) {
                if (abs(GBS::STATUS_SYNC_PROC_VTOTAL::read() - 262) <= 10)
                    OSD_writeStringAtRow(2, 0xFF, "   240p", OSD_TEXT_NORMAL);
                else
                    OSD_writeStringAtRow(2, 0xFF, "   480i", OSD_TEXT_NORMAL);
            } else {
                OSD_writeStringAtRow(2, 0xFF, "   Err", OSD_TEXT_NORMAL);
            }
        } else {
            OSD_writeStringAtRow(2, 0xFF, "   Err", OSD_TEXT_NORMAL);
        }
    }

    if (irDecode()) {
        switch (results.value) {
            case IR_KEY_MENU:
                exitMenu();
                break;
            case IR_KEY_EXIT:
                exitMenu();
                break;
        }
        irResume();
    }

    return true;
}

// ====================================================================================
// IR_handleMenuSelection - Menu State Machine
// ====================================================================================

// Check if the IR key is a valid menu navigation key
static bool IR_isValidMenuKey(uint32_t key)
{
    switch (key) {
        case IR_KEY_MENU:
        case IR_KEY_SAVE:
        case IR_KEY_INFO:
        case IR_KEY_RIGHT:
        case IR_KEY_LEFT:
        case IR_KEY_UP:
        case IR_KEY_DOWN:
        case IR_KEY_OK:
        case IR_KEY_EXIT:
        case IR_KEY_MUTE:
        case IR_KEY_VOL_UP:
        case IR_KEY_VOL_DN:
            return true;
        default:
            return false;
    }
}

// Get the user command for a given resolution
static char IR_getResolutionCommand(uint8_t resolution)
{
    switch (resolution) {
        case Output960P:  return 'f';  // 1280x960
        case Output720P:  return 'g';  // 1280x720
        case Output480P:  return 'h';  // 480p/576p
        case Output1024P: return 'p';  // 1280x1024
        case Output1080P: return 's';  // 1920x1080
        default:          return 'g';  // Default to 720p
    }
}

// Handle resolution confirmation countdown display
static void IR_updateResolutionCountdown(void)
{
    if ((millis() - lastResolutionTime) < OSD_RESOLUTION_UP_TIME ||
        oled_menuItem != OLED_RetainedSettings) {
        return;
    }

    lastMenuItemTime = millis();
    lastResolutionTime = millis();

    uint8_t secondsRemaining = OSD_RESOLUTION_CLOSE_TIME / 1000 -
                               ((lastResolutionTime - resolutionStartTime) / 1000);

    // Display countdown timer
    if (secondsRemaining >= 10) {
        OSD_writeCharAtRow(2, (secondsRemaining / 10) + '0', 11, OSD_TEXT_NORMAL);
        OSD_writeCharAtRow(2, (secondsRemaining % 10) + '0', 12, OSD_TEXT_NORMAL);
        OSD_writeStringAtRow(2, 14, " s ", OSD_TEXT_NORMAL);
    } else {
        OSD_writeCharAtRow(2, '0', 12, OSD_BACKGROUND);
        OSD_writeCharAtRow(2, secondsRemaining + '0', 11, OSD_TEXT_NORMAL);
        OSD_writeStringAtRow(2, 13, " s ", OSD_TEXT_NORMAL);
    }

    // Countdown expired - apply resolution
    if ((lastResolutionTime - resolutionStartTime) >= OSD_RESOLUTION_CLOSE_TIME) {
        userCommand = IR_getResolutionCommand(tentativeResolution);
        OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
        OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
        oled_menuItem = OLED_OutputResolution_PassThrough;
    }
}

// Handle menu timeout and cleanup
static void IR_handleMenuTimeout(void)
{
    // Track menu item changes
    if (lastOledMenuItem != oled_menuItem && oled_menuItem != OLED_None) {
        lastMenuItemTime = millis();
        oledClearFlag = 1;
    }

    // Check for menu timeout
    if ((millis() - lastMenuItemTime) >= OSD_CLOSE_TIME && oled_menuItem != OLED_None) {
        // Restore display settings if Info Display was active
        if (isInfoDisplayActive) {
            GBS::VDS_DIS_HB_ST::write(horizontalBlankStart);
            GBS::VDS_DIS_HB_SP::write(horizontalBlankStop);
            isInfoDisplayActive = 0;
        }

        // Close menu
        oled_menuItem = OLED_None;
        lastOledMenuItem = OLED_None;
        OSD_clearAll();
        OSD_init();
    }

    lastOledMenuItem = oled_menuItem;
}

void IR_handleMenuSelection(void)
{
    NEW_OLED_MENU = (oled_menuItem == OLED_None);

    // Dispatch to appropriate handler
    IR_handleOutputResolution() ||
    IR_handleScreenSettings() ||
    IR_handleColorSettings() ||
    IR_handleSystemSettings() ||
    IR_handleInputSelection() ||
    IR_handleProfileManagement() ||
    IR_handleMainMenu() ||
    IR_handleMiscSettings() ||
    IR_handleInfoDisplay();

    // Reset activity timer on valid IR input
    if (IR_isValidMenuKey(results.value) && irDecodedFlag && oled_menuItem != OLED_None) {
        lastMenuItemTime = millis();
        irDecodedFlag = 0;
        resetOLEDScreenSaverTimer();
    }

    // Handle resolution confirmation countdown
    IR_updateResolutionCountdown();

    // Handle menu timeout
    IR_handleMenuTimeout();
}

// ====================================================================================
// IR_handleInput - IR Remote Input Handler
// ====================================================================================

// Display mute status on OSD and OLED
static void IR_displayMuteStatus(bool muted)
{
    const char* statusText = muted ? "MUTE ON" : "MUTE OFF";

    NEW_OLED_MENU = false;
    OSD_fillRowBackground(ROW_1, 9, OSD_BACKGROUND);

    // Display on OSD
    for (int i = 0; i <= 800; i++) {
        OSD_writeStringAtRow(1, 1, "MUTE", OSD_TEXT_SELECTED);
        if (muted) {
            OSD_writeStringAtRow(1, 6, "ON ", OSD_TEXT_NORMAL);  // Extra space to clear "OFF"
        } else {
            OSD_writeStringAtRow(1, 6, "OFF", OSD_TEXT_NORMAL);
        }

        // Display on OLED
        display.clear();
        if (muted) {
            display.flipScreenVertically();
        }
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(8, 15, statusText);
        display.display();
    }

    oled_menuItem = OLED_None;
    OSD_fillRowBackground(ROW_1, 9, OSD_BACKGROUND);
    OSD_clearRowColors(ROW_1);
    OSD_init();
}

// Handle mute toggle
static void IR_handleMuteToggle(void)
{
    lastMenuItemTime = millis();

    if (audioMuted) {
        PT2257_mute(false);  // Unmute
        IR_displayMuteStatus(false);
        audioMuted = 0;
    } else {
        PT2257_mute(true);  // Mute
        IR_displayMuteStatus(true);
        audioMuted = 1;
    }
}

// Handle Menu key press - opens main menu or info display
static void IR_handleMenuKeyPress(void)
{
    lastMenuItemTime = millis();
    NEW_OLED_MENU = false;

    // Check if source is disconnected or board has no power
    bool noSignal = rto->sourceDisconnected ||
                    !rto->boardHasPower ||
                    GBS::PAD_CKIN_ENZ::read();

    if (noSignal) {
        // Show info display when no signal
        OSD_fillRowBackground(ROW_1, 28, OSD_BACKGROUND);
        OSD_fillRowBackground(ROW_2, 28, OSD_BACKGROUND);
        oled_menuItem = OLED_Info_Display;

        // Save horizontal blank values before modifying
        isInfoDisplayActive = 1;
        horizontalBlankStart = GBS::VDS_DIS_HB_ST::read();
        horizontalBlankStop = GBS::VDS_DIS_HB_SP::read();

        // Initialize display for info
        writeProgramArrayNew(ntsc_720x480, false);
        doPostPresetLoadSteps();
        GBS::VDS_DIS_HB_ST::write(0x00);
        GBS::VDS_DIS_HB_SP::write(0xffff);
        freezeVideo();
        GBS::SP_CLAMP_MANUAL::write(1);
    } else {
        // Open main input menu
        selectedMenuLine = 1;
        OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
        oled_menuItem = OLED_Input;
        display.clear();
    }
}

// Handle Save key press - opens profile menu
static void IR_handleSaveKeyPress(void)
{
    lastMenuItemTime = millis();
    NEW_OLED_MENU = false;
    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    OSD_handleCommand(OSD_CMD_PROFILE_SAVELOAD);
    oled_menuItem = OLED_Profile_Load1;
}

// Handle Info key press - opens info display
static void IR_handleInfoKeyPress(void)
{
    lastMenuItemTime = millis();
    NEW_OLED_MENU = false;
    OSD_fillRowBackground(ROW_1, 28, OSD_BACKGROUND);
    OSD_fillRowBackground(ROW_2, 28, OSD_BACKGROUND);
    oled_menuItem = OLED_Info_Display;
}

// Handle Volume keys - opens volume adjust menu
static void IR_handleVolumeKeyPress(void)
{
    lastMenuItemTime = millis();
    NEW_OLED_MENU = false;
    OSD_fillRowBackground(ROW_1, 25, OSD_BACKGROUND);
    oled_menuItem = OLED_Volume_Adjust;
}

void IR_handleInput()
{
    if (!irDecode()) {
        return;
    }

    switch (results.value) {
        case IR_KEY_MENU:
            IR_handleMenuKeyPress();
            break;
        case IR_KEY_SAVE:
            IR_handleSaveKeyPress();
            break;
        case IR_KEY_INFO:
            IR_handleInfoKeyPress();
            break;
        case IR_KEY_MUTE:
            IR_handleMuteToggle();
            break;
        case IR_KEY_VOL_UP:
        case IR_KEY_VOL_DN:
            IR_handleVolumeKeyPress();
            break;
    }

    irResume();
    delay(5);
}

// ====================================================================================
// OLED Menu Initialization - Input Menu
// ====================================================================================

void OLED_initInputMenu(OLEDMenuItem *root) {
    OLEDMenuItem *advMenu = oledMenu.registerItem(root, MT_NULL, IMAGE_ITEM(OM_ADVINPUT));

    const char *inputLabels[6] = {
        "RGBs", "RGsB", "VGA", "YPBPR", "SV", "AV"
    };

    uint8_t inputTags[6] = {
        MT_RGBs, MT_RGsB, MT_VGA, MT_YPBPR, MT_SV, MT_AV
    };

    for (size_t i = 0; i < 6; ++i) {
        oledMenu.registerItem(advMenu, inputTags[i], inputLabels[i], OLED_handleInputSelection);
    }
}

// ====================================================================================
// OLED Menu Initialization - Settings Menu
// ====================================================================================

void OLED_initSettingsMenu(OLEDMenuItem *root) {
    OLEDMenuItem *settingMenu = oledMenu.registerItem(root, MT_NULL, IMAGE_ITEM(OM_SETTING));

#ifdef ACE
    const char *settingLabels[6] = {
        "Smooth_Off", "Smooth_On ",
        "Compatibility_Off", "Compatibility_On ",
        "ACE_Off", "ACE_On "
    };
    uint8_t settingTags[6] = {
        MT_SMOOTH_OFF, MT_SMOOTH_ON,
        MT_COMPATIBILITY_OFF, MT_COMPATIBILITY_ON,
        MT_ACE_OFF, MT_ACE_ON
    };
    const size_t settingCount = 6;
#else
    const char *settingLabels[4] = {
        "Smooth_Off", "Smooth_On ",
        "Compatibility_Off", "Compatibility_On "
    };
    uint8_t settingTags[4] = {
        MT_SMOOTH_OFF, MT_SMOOTH_ON,
        MT_COMPATIBILITY_OFF, MT_COMPATIBILITY_ON
    };
    const size_t settingCount = 4;
#endif

    for (size_t i = 0; i < settingCount; ++i) {
        oledMenu.registerItem(settingMenu, settingTags[i], settingLabels[i], OLED_handleSettingSelection);
    }

    // TV Mode submenu
    OLEDMenuItem *tvModeMenu = oledMenu.registerItem(settingMenu, MT_NULL, IMAGE_ITEM(OM_TVMODE));

    const char *tvModeLabels[12] = {
        "AUTO", "PAL", "NTSC-M", "PAL-60", "NTSC443", "NTSC-J",
        "PAL-N w/ p", "PAL-M w/o p", "PAL-M", "PAL Cmb -N", "PAL Cmb -N w/ p", "SECAM"
    };

    uint8_t tvModeTags[12] = {
        MT_MODE_AUTO, MT_MODE_PAL, MT_MODE_NTSCM, MT_MODE_PAL60,
        MT_MODE_NTSC443, MT_MODE_NTSCJ, MT_MODE_PALNwp, MT_MODE_PALMwop,
        MT_MODE_PALM, MT_MODE_PALCmbN, MT_MODE_PALCmbNwp, MT_MODE_SECAM
    };

    for (size_t i = 0; i < 12; ++i) {
        oledMenu.registerItem(tvModeMenu, tvModeTags[i], tvModeLabels[i], OLED_handleTvModeSelection);
    }
}

// ====================================================================================
// OLED Menu Handlers - Input Selection
// ====================================================================================

bool OLED_handleInputSelection(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav, bool isFirstTime) {
    if (!isFirstTime) {
        checkFreezeTimeout(manager);
        return false;
    }

    showSelectionFeedback(manager, item->str);

    // Apply input selection
    INPUT_PresetPreference preset = INPUT_PresetPreference::MT_RGBs;

    switch (item->tag) {
        case MT_RGBs:  preset = INPUT_PresetPreference::MT_RGBs;  break;
        case MT_RGsB:  preset = INPUT_PresetPreference::MT_RGsB;  break;
        case MT_VGA:   preset = INPUT_PresetPreference::MT_VGA;   break;
        case MT_YPBPR: preset = INPUT_PresetPreference::MT_YPBPR; break;
        case MT_SV:    preset = INPUT_PresetPreference::MT_SV;    break;
        case MT_AV:    preset = INPUT_PresetPreference::MT_AV;    break;
        default: break;
    }

    uopt->INPUT_presetPreference = preset;

    switch (preset) {
        case INPUT_PresetPreference::MT_RGBs:  InputRGBs(); break;
        case INPUT_PresetPreference::MT_RGsB:  InputRGsB(); break;
        case INPUT_PresetPreference::MT_VGA:   InputVGA();  break;
        case INPUT_PresetPreference::MT_YPBPR: InputYUV();  break;
        case INPUT_PresetPreference::MT_SV:    InputSV();   break;
        case INPUT_PresetPreference::MT_AV:    InputAV();   break;
        default: break;
    }

    return false;
}

// ====================================================================================
// OLED Menu Handlers - Settings Selection
// ====================================================================================

bool OLED_handleSettingSelection(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav, bool isFirstTime) {
    if (!isFirstTime) {
        checkFreezeTimeout(manager);
        return false;
    }

    showSelectionFeedback(manager, item->str);

    // Apply setting
    SETTING_PresetPreference preset = SETTING_PresetPreference::MT_7391_1X;

    switch (item->tag) {
        case MT_7391_1X:          preset = SETTING_PresetPreference::MT_7391_1X;          break;
        case MT_7391_2X:          preset = SETTING_PresetPreference::MT_7391_2X;          break;
        case MT_SMOOTH_OFF:       preset = SETTING_PresetPreference::MT_SMOOTH_OFF;       break;
        case MT_SMOOTH_ON:        preset = SETTING_PresetPreference::MT_SMOOTH_ON;        break;
        case MT_COMPATIBILITY_OFF: preset = SETTING_PresetPreference::MT_COMPATIBILITY_OFF; break;
        case MT_COMPATIBILITY_ON:  preset = SETTING_PresetPreference::MT_COMPATIBILITY_ON;  break;
#ifdef ACE
        case MT_ACE_OFF:          preset = SETTING_PresetPreference::MT_ACE_OFF;          break;
        case MT_ACE_ON:           preset = SETTING_PresetPreference::MT_ACE_ON;           break;
#endif
        default: break;
    }

    uopt->SETTING_presetPreference = preset;

    switch (preset) {
        case SETTING_PresetPreference::MT_7391_1X:
            ADV_sendLine1X();
            break;
        case SETTING_PresetPreference::MT_7391_2X:
            ADV_sendLine2X();
            break;
        case SETTING_PresetPreference::MT_SMOOTH_OFF:
            ADV_sendSmoothOff();
            break;
        case SETTING_PresetPreference::MT_SMOOTH_ON:
            ADV_sendSmoothOn();
            break;
        case SETTING_PresetPreference::MT_COMPATIBILITY_OFF:
            rgbComponentMode = COMPATIBILITY_OFF;
            ADV_sendCompatibility(rgbComponentMode);
            break;
        case SETTING_PresetPreference::MT_COMPATIBILITY_ON:
            rgbComponentMode = COMPATIBILITY_ON;
            ADV_sendCompatibility(rgbComponentMode);
            break;
#ifdef ACE
        case SETTING_PresetPreference::MT_ACE_OFF:
            {
                unsigned char Adv_ACE_OFF[7] = {0x41, 0x44, 'S', 0x81};
                Serial.write(Adv_ACE_OFF, 7);
            }
            break;
        case SETTING_PresetPreference::MT_ACE_ON:
            {
                unsigned char Adv_ACE_ON[7] = {0x41, 0x44, 'S', 0x80};
                Serial.write(Adv_ACE_ON, 7);
            }
            break;
#endif
        default:
            break;
    }

    return false;
}

// ====================================================================================
// OLED Menu Handlers - TV Mode Selection
// ====================================================================================

bool OLED_handleTvModeSelection(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav, bool isFirstTime) {
    if (!isFirstTime) {
        checkFreezeTimeout(manager);
        return false;
    }

    showSelectionFeedback(manager, item->str);

    // Apply TV mode selection
    TVMODE_PresetPreference preset = TVMODE_PresetPreference::MT_MODE_AUTO;

    switch (item->tag) {
        case MT_MODE_AUTO:      preset = TVMODE_PresetPreference::MT_MODE_AUTO;      break;
        case MT_MODE_PAL:       preset = TVMODE_PresetPreference::MT_MODE_PAL;       break;
        case MT_MODE_NTSCM:     preset = TVMODE_PresetPreference::MT_MODE_NTSCM;     break;
        case MT_MODE_PAL60:     preset = TVMODE_PresetPreference::MT_MODE_PAL60;     break;
        case MT_MODE_NTSC443:   preset = TVMODE_PresetPreference::MT_MODE_NTSC443;   break;
        case MT_MODE_NTSCJ:     preset = TVMODE_PresetPreference::MT_MODE_NTSCJ;     break;
        case MT_MODE_PALNwp:    preset = TVMODE_PresetPreference::MT_MODE_PALNwp;    break;
        case MT_MODE_PALMwop:   preset = TVMODE_PresetPreference::MT_MODE_PALMwop;   break;
        case MT_MODE_PALM:      preset = TVMODE_PresetPreference::MT_MODE_PALM;      break;
        case MT_MODE_PALCmbN:   preset = TVMODE_PresetPreference::MT_MODE_PALCmbN;   break;
        case MT_MODE_PALCmbNwp: preset = TVMODE_PresetPreference::MT_MODE_PALCmbNwp; break;
        case MT_MODE_SECAM:     preset = TVMODE_PresetPreference::MT_MODE_SECAM;     break;
        default: break;
    }

    uopt->TVMODE_presetPreference = preset;

    // Update AV/SV mode options if applicable
    if (inputType == InputTypeAV) {
        AVModeOption = 0;
        saveUserPrefs();
    } else if (inputType == InputTypeSV) {
        SVModeOption = 0;
        saveUserPrefs();
    }

    // Send TV mode command if in SV or AV mode
    if (inputType == InputTypeSV || inputType == InputTypeAV) {
        ADV_sendVideoFormat(ADV_VideoFormats[preset]);
    }

    return false;
}
