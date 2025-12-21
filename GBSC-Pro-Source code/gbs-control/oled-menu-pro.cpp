// ====================================================================================
// oled-menu-pro.cpp
// OLED Menu Navigation and IR Remote Handling
//
// This file contains:
// - IR_handleMenuSelection(): Main menu state machine for IR remote navigation
// - IR_handleInput(): IR remote input handler and decoder
// ====================================================================================

#include "oled-menu-pro.h"
#include "gbs-control-pro.h"
#include "osd-render-pro.h"
#include "OLEDMenuImplementation-pro.h"
#include "tv5725.h"
#include "SSD1306Wire.h"
#include "options.h"
#include "ntsc_720x480.h"

#include <IRremoteESP8266.h>
#include <IRrecv.h>

#include "OSD_TV/OSD_stv9426.h"
#include "OSD_TV/remote.h"
#include "OSD_TV/PT2257.h"

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

// Highlight icon at position (1=top, 2=mid, 3=bottom)
static void highlightIcon(uint8_t pos) {
    OSD_writeCharRow1(icon4, P0, pos == 1 ? OSD_CURSOR_ACTIVE : OSD_CURSOR_INACTIVE);
    OSD_writeCharRow2(icon4, P0, pos == 2 ? OSD_CURSOR_ACTIVE : OSD_CURSOR_INACTIVE);
    OSD_writeCharRow3(icon4, P0, pos == 3 ? OSD_CURSOR_ACTIVE : OSD_CURSOR_INACTIVE);
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

// Show "limit" feedback on TV OSD row, then clear (blocking)
// row: ROW_1, ROW_2, or ROW_3
// iterations: number of loop iterations for delay (~400 = visible flash)
static void showLimitFeedback(uint8_t row, int iterations = 400) {
    for (int p = 0; p <= iterations; p++) {
        currentColor = OSD_TEXT_DISABLED;
        currentRow = row;
        OSD_writeString(20, "limit");
        OSD_writeCharAt(0x0d, _25);
    }
    currentColor = OSD_BACKGROUND;
    currentRow = row;
    OSD_writeString(20, "limit");
    OSD_writeCharAt(0x0d, _25);
}

// Show "OK" feedback on TV OSD row, then clear (blocking)
// row: ROW_1, ROW_2, or ROW_3
// iterations: number of loop iterations for delay (~800 = visible flash)
static void showOkFeedback(uint8_t row, int iterations = 800) {
    for (int p = 0; p <= iterations; p++) {
        currentColor = OSD_TEXT_DISABLED;
        currentRow = row;
        OSD_writeString(25, "OK");
    }
    currentColor = OSD_BACKGROUND;
    currentRow = row;
    OSD_writeString(25, "OK");
}

// Show "saving" feedback on TV OSD row, then clear (blocking)
// row: ROW_1, ROW_2, or ROW_3
// startPos: starting position for "saving" text (default 19)
// iterations: number of loop iterations for delay (~800 = visible flash)
static void showSavingFeedback(uint8_t row, uint8_t startPos = 19, int iterations = 800) {
    for (int p = 0; p <= iterations; p++) {
        currentColor = OSD_TEXT_DISABLED;
        currentRow = row;
        OSD_writeString(startPos, "saving");
    }
    currentColor = OSD_BACKGROUND;
    currentRow = row;
    OSD_writeString(startPos, "saving");
}

// Show 4-direction adjustment arrows on TV OSD row
// row: 1, 2, or 3
// dashStart: starting position for dashes (default 8)
// Displays dashes (dashStart-P13) and arrow icons (P14-P17)
static void showAdjustArrows(uint8_t row, uint8_t dashStart = 8) {
    OSD_drawDashRange(row, dashStart, 13);
    void (*rowFunc)(uint8_t, uint8_t, uint8_t);
    if (row == 1) rowFunc = OSD_writeCharRow1;
    else if (row == 2) rowFunc = OSD_writeCharRow2;
    else rowFunc = OSD_writeCharRow3;
    rowFunc(0x03, P14, OSD_CURSOR_ACTIVE);  // ↑
    rowFunc(0x08, P15, OSD_CURSOR_ACTIVE);  // ←
    rowFunc(0x18, P16, OSD_CURSOR_ACTIVE);  // →
    rowFunc(0x13, P17, OSD_CURSOR_ACTIVE);  // ↓
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
    if (results.value == IRKeyUp || results.value == IRKeyDown) {
        OSD_writeCharRow1(icon4, P0, isLoadRow ? OSD_CURSOR_ACTIVE : OSD_BACKGROUND);
        OSD_writeCharRow2(icon4, P0, isLoadRow ? OSD_BACKGROUND : OSD_CURSOR_ACTIVE);
        OSD_writeCharRow3(icon4, P0, OSD_BACKGROUND);
        if (isLoadRow) OSD_handleCommand(OSD_CMD_PROFILE_SAVELOAD);
    }

    OSD_handleCommand(OSD_CMD_PROFILE_SLOTDISPLAY);

    if (irDecode()) {
        switch (results.value) {
            case IRKeyMenu:
                exitMenu();
                break;
            case IRKeyUp:
                if (!isLoadRow) {
                    oled_menuItem = OLED_Profile_Load1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_PROFILE_SAVELOAD);
                }
                break;
            case IRKeyDown:
                if (isLoadRow) {
                    oled_menuItem = OLED_Profile_Save1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_PROFILE_SAVELOAD);
                }
                break;
            case IRKeyRight:
                oled_menuItem = getNextSlot(base, idx);
                break;
            case IRKeyLeft:
                oled_menuItem = getPrevSlot(base, idx);
                break;
            case IRKeyOk:
                uopt->presetSlot = getSlotChar(idx);
                if (isLoadRow) {
                    OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW1);
                } else {
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    showOkFeedback(ROW_2);
                }
                break;
            case IRKeyExit:
                if (!isLoadRow) selectedMenuLine = 1;
                OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                oled_menuItem = OLED_OutputResolution;
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

        if (results.value == IRKeyUp || results.value == IRKeyDown) {
            selectedMenuLine = 2;
            highlightIcon(2);
            OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
                    oled_menuItem = OLED_Input;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
                    oled_menuItem = OLED_ScreenSettings;
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_OutputResolution_1080;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_OUTPUT_1080_1024_960);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_OutputResolution_1080) {
        showMenu("Menu->Output", "1920x1080");

        if (results.value == IRKeyUp) {
            highlightIcon(1);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_OUTPUT_1080_1024_960);
                    oled_menuItem = OLED_OutputResolution_1024;
                    break;
                case IRKeyOk:
                    userCommand = 's';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_OUTPUT_1080_1024_960);
                    oled_menuItem = OLED_OutputResolution_1080;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_OUTPUT_1080_1024_960);
                    oled_menuItem = OLED_OutputResolution_960;
                    break;
                case IRKeyOk:
                    userCommand = 'p';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(3);
            OSD_handleCommand(OSD_CMD_OUTPUT_1080_1024_960);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_OUTPUT_1080_1024_960);
                    oled_menuItem = OLED_OutputResolution_1024;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_OutputResolution_720;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
                    break;
                case IRKeyOk:
                    userCommand = 'f';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyUp) {
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_OutputResolution_960;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_OUTPUT_1080_1024_960);
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
                    oled_menuItem = OLED_OutputResolution_480;
                    break;
                case IRKeyOk:
                    userCommand = 'g';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
                    oled_menuItem = OLED_OutputResolution_720;
                    break;
                case IRKeyOk:
                    userCommand = 'h';
                    break;
                case IRKeyExit:
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
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
            OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
        }
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
                    oled_menuItem = OLED_OutputResolution_720;
                    break;
                case IRKeyOk:
                    break;
                case IRKeyExit:
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
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyRight:
                    keepSettings = 0;
                    OSD_handleCommand(OSD_CMD_INPUT_INFO);
                    break;
                case IRKeyLeft:
                    keepSettings = 1;
                    OSD_handleCommand(OSD_CMD_INPUT_INFO);
                    break;
                case IRKeyOk:
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
                case IRKeyExit:
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
    //     if (results.value == IRKeyDown || results.value == IRKeyUp) {
    //         highlightIcon(3);
    //         OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
    //     }
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
    //                 oled_menuItem = OLED_OutputResolution;
    //                 break;
    //             case IRKeyUp:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
    //                 oled_menuItem = OLED_OutputResolution_480;
    //                 break;
    //             case IRKeyDown:
    //                 oled_menuItem = OLED_OutputResolution_PassThrough;
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_OUTPUT_PASSTHROUGH);
    //                 break;
    //             case IRKeyOk:
    //                 userCommand = 'L';
    //                 break;
    //             case IRKeyExit:
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

        if (results.value == IRKeyUp) {
            highlightIcon(1);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Scale;
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_ScreenSettings_MoveAdjust;
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    showAdjustArrows(1, 5);
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Move;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Borders;
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_ScreenSettings_ScaleAdjust;
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    showAdjustArrows(2, 6);
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(3);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Scale;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_ScreenSettings_FullHeight;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SCREEN_FULLHEIGHT);
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_ScreenSettings_BordersAdjust;
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    showAdjustArrows(3);
                    break;
                case IRKeyExit:
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
        if (results.value == IRKeyOk) {
            OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
            showAdjustArrows(1, 5);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyOk:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Move;
                    break;
                case IRKeyRight:
                    lastMenuItemTime = millis();
                    serialCommand = '6';
                    if (GBS::IF_HBIN_SP::read() < 10) {
                        showLimitFeedback(ROW_1);
                    }
                    break;
                case IRKeyLeft:
                    lastMenuItemTime = millis();
                    serialCommand = '7';
                    if (GBS::IF_HBIN_SP::read() >= 0x150) {
                        showLimitFeedback(ROW_1);
                    }
                    break;
                case IRKeyUp:
                    lastMenuItemTime = millis();
                    shiftVerticalUpIF();
                    break;
                case IRKeyDown:
                    lastMenuItemTime = millis();
                    shiftVerticalDownIF();
                    break;
                case IRKeyExit:
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
        if (results.value == IRKeyOk) {
            OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
            showAdjustArrows(2, 6);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyOk:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Scale;
                    break;
                case IRKeyRight:
                    lastMenuItemTime = millis();
                    serialCommand = 'h';
                    if (GBS::VDS_HSCALE::read() == 1023) {
                        showLimitFeedback(ROW_2);
                    }
                    break;
                case IRKeyLeft:
                    lastMenuItemTime = millis();
                    serialCommand = 'z';
                    if (GBS::VDS_HSCALE::read() <= 256) {
                        showLimitFeedback(ROW_2);
                    }
                    break;
                case IRKeyUp:
                    lastMenuItemTime = millis();
                    serialCommand = '5';
                    if (GBS::VDS_VSCALE::read() == 1023) {
                        showLimitFeedback(ROW_2);
                    }
                    break;
                case IRKeyDown:
                    lastMenuItemTime = millis();
                    serialCommand = '4';
                    if (GBS::VDS_VSCALE::read() <= 256) {
                        showLimitFeedback(ROW_2);
                    }
                    break;
                case IRKeyExit:
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
        if (results.value == IRKeyOk) {
            OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
            showAdjustArrows(3);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyOk:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    oled_menuItem = OLED_ScreenSettings_Borders;
                    break;
                case IRKeyRight:
                    userCommand = 'A';
                    if (!((GBS::VDS_DIS_HB_ST::read() > 4) && (GBS::VDS_DIS_HB_SP::read() < (GBS::VDS_HSYNC_RST::read() - 4)))) {
                        showLimitFeedback(ROW_3);
                    }
                    break;
                case IRKeyLeft:
                    userCommand = 'B';
                    if (!((GBS::VDS_DIS_HB_ST::read() < (GBS::VDS_HSYNC_RST::read() - 4)) && (GBS::VDS_DIS_HB_SP::read() > 4))) {
                        showLimitFeedback(ROW_3);
                    }
                    break;
                case IRKeyUp:
                    userCommand = 'C';
                    if (!((GBS::VDS_DIS_VB_ST::read() > 6) && (GBS::VDS_DIS_VB_SP::read() < (GBS::VDS_VSYNC_RST::read() - 4)))) {
                        showLimitFeedback(ROW_3);
                    }
                    break;
                case IRKeyDown:
                    userCommand = 'D';
                    if (!((GBS::VDS_DIS_VB_ST::read() < (GBS::VDS_VSYNC_RST::read() - 4)) && (GBS::VDS_DIS_VB_SP::read() > 6))) {
                        showLimitFeedback(ROW_3);
                    }
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_SCREEN_FULLHEIGHT);
        }
        OSD_handleCommand(OSD_CMD_SCREEN_FULLHEIGHT_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_ScreenSettings_Borders;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    break;
                case IRKeyOk: {
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
                case IRKeyExit:
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

        if (results.value == IRKeyUp) {
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    oled_menuItem = OLED_ColorSettings_RGB_B;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    oled_menuItem = OLED_ColorSettings_Scanlines;
                    break;
                case IRKeyRight:
                    userCommand = 'n';
                    break;
                case IRKeyLeft:
                    userCommand = 'o';
                    break;
                case IRKeyOk:
                    serialCommand = 'T';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    oled_menuItem = OLED_ColorSettings_ADCGain;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    oled_menuItem = OLED_ColorSettings_LineFilter;
                    break;
                case IRKeyRight:
                    if (scanlinesAllowed) {
                        userCommand = 'K';
                    }
                    break;
                case IRKeyLeft:
                    if (scanlinesAllowed) {
                        userCommand = 'K';
                    }
                    break;
                case IRKeyOk:
                    if (scanlinesAllowed) {
                        userCommand = '7';
                    }
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(3);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    oled_menuItem = OLED_ColorSettings_Scanlines;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_ColorSettings_Sharpness;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    break;
                case IRKeyOk:
                    userCommand = 'm';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyUp) {
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_ColorSettings_LineFilter;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    oled_menuItem = OLED_ColorSettings_Peaking;
                    break;
                case IRKeyOk:
                    userCommand = 'W';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    oled_menuItem = OLED_ColorSettings_Sharpness;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    oled_menuItem = OLED_ColorSettings_StepResponse;
                    break;
                case IRKeyOk:
                    if (!isPeakingLocked()) {
                        serialCommand = 'f';
                    }
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(3);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
        }

        OSD_handleCommand(OSD_CMD_COLOR_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    oled_menuItem = OLED_ColorSettings_Peaking;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_ColorSettings_DefaultColor;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
                    break;
                case IRKeyOk:
                    serialCommand = 'V';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyUp) {
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
        }
        OSD_handleCommand(OSD_CMD_COLOR_RGB_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    oled_menuItem = OLED_ColorSettings_RGB_G;
                    break;
                case IRKeyRight:
                    R_VAL = MIN(R_VAL + STEP, 255);
                    applyRGBtoYUVConversion();
                    break;
                case IRKeyLeft:
                    R_VAL = MAX(0, R_VAL - STEP);
                    applyRGBtoYUVConversion();
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
        }
        OSD_handleCommand(OSD_CMD_COLOR_RGB_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    oled_menuItem = OLED_ColorSettings_RGB_R;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    oled_menuItem = OLED_ColorSettings_RGB_B;
                    break;
                case IRKeyRight:
                    G_VAL = MIN(G_VAL + STEP, 255);
                    applyRGBtoYUVConversion();
                    break;
                case IRKeyLeft:
                    G_VAL = MAX(0, G_VAL - STEP);
                    applyRGBtoYUVConversion();
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(3);
            OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
        }
        OSD_handleCommand(OSD_CMD_COLOR_RGB_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    oled_menuItem = OLED_ColorSettings_RGB_G;
                    break;
                case IRKeyDown:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE1);
                    oled_menuItem = OLED_ColorSettings_ADCGain;
                    break;
                case IRKeyRight:
                    B_VAL = MIN(B_VAL + STEP, 255);
                    applyRGBtoYUVConversion();
                    break;
                case IRKeyLeft:
                    B_VAL = MAX(0, B_VAL - STEP);
                    applyRGBtoYUVConversion();
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyUp) {
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
        }

        OSD_handleCommand(OSD_CMD_SYS_SVINPUT_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_ColorSettings_RGB_B;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
                    oled_menuItem = OLED_ColorSettings_Color;
                    break;
                case IRKeyRight:
                    cur = MIN(cur + STEP, 255);
                    GBS::VDS_Y_GAIN::write(cur);
                    break;
                case IRKeyLeft:
                    cur = MAX(0, cur - STEP);
                    GBS::VDS_Y_GAIN::write(cur);
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
        }
        OSD_handleCommand(OSD_CMD_SYS_SVINPUT_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
                    oled_menuItem = OLED_ColorSettings_Y_Gain;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
                    oled_menuItem = OLED_ColorSettings_DefaultColor;
                    break;
                case IRKeyRight:
                    userCommand = 'V';
                    break;
                case IRKeyLeft:
                    userCommand = 'R';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_COLOR_PAGE3);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_ColorSettings_StepResponse;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_COLOR_PAGE2);
                    break;
                case IRKeyOk:
                    userCommand = 'U';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyUp || results.value == IRKeyDown) {
            highlightIcon(3);
            OSD_handleCommand(OSD_CMD_SYS_PAGE1);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_Compatibility;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_SystemSettings_Deinterlace;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    break;
                case IRKeyOk:
                    serialCommand = 'Z';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(3);
            OSD_handleCommand(OSD_CMD_SYS_PAGE1);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_Compatibility;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_SystemSettings_Deinterlace;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    oled_menuItem = OLED_SystemSettings_Deinterlace;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    oled_menuItem = OLED_SystemSettings_LockMethod;
                    break;
                case IRKeyOk:
                    if (uopt->PalForce60 == 0) {
                        uopt->PalForce60 = 1;
                    } else {
                        uopt->PalForce60 = 0;
                    }
                    saveUserPrefs();
                    applyVideoModePreset();
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(3);
            OSD_handleCommand(OSD_CMD_SYS_PAGE4);
        }
        OSD_handleCommand(OSD_CMD_SYS_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE4);
                    oled_menuItem = OLED_SystemSettings_FrameTimeLock;
                    break;
                case IRKeyOk:
                    userCommand = 'X';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyUp) {
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_SYS_PAGE4);
        }
        OSD_handleCommand(OSD_CMD_SYS_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_SystemSettings_LockMethod;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE4);
                    oled_menuItem = OLED_SystemSettings_FrameTimeLock;
                    break;
                case IRKeyOk:
                    userCommand = 'w';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE4_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE4);
                    oled_menuItem = OLED_SystemSettings_ADCCalibration;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE4);
                    oled_menuItem = OLED_SystemSettings_ClockGenerator;
                    break;
                case IRKeyOk:
                    userCommand = '5';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(3);
            OSD_handleCommand(OSD_CMD_SYS_PAGE2);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    oled_menuItem = OLED_SystemSettings_Force5060Hz;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_SystemSettings_ADCCalibration;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE4);
                    break;
                case IRKeyOk:
                    userCommand = 'i';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyUp || results.value == IRKeyDown) {
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_SYS_PAGE2);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_SystemSettings_MatchedPresets;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE2);
                    oled_menuItem = OLED_SystemSettings_Force5060Hz;
                    break;
                case IRKeyOk:
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
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_SYS_PAGE1);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IRKeyOk:
                    if (inputType == InputTypeSV || inputType == InputTypeAV) {
                        selectedMenuLine = 1;
                        OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                        OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
                        oled_menuItem = OLED_SystemSettings_SVAVInput_DoubleLine;
                    }
                    break;
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_Compatibility;
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
        }
        OSD_handleCommand(OSD_CMD_ADCCALIB_DISPLAY);
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Smooth;
                    break;
                case IRKeyOk:
                    lineOption = !lineOption;
                    settingLineOptionChanged = 1;
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
            OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
        }
        OSD_handleCommand(OSD_CMD_ADCCALIB_DISPLAY);
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_DoubleLine;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Bright;
                    break;
                case IRKeyOk:
                    if (lineOption) {
                        smoothOption = !smoothOption;
                        settingSmoothOptionChanged = 1;
                    }
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(3);
            OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
        }
        OSD_handleCommand(OSD_CMD_ADCCALIB_DISPLAY);
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Smooth;
                    break;
                case IRKeyDown:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Contrast;
                    break;
                case IRKeyRight:
                    brightness = MIN(brightness + STEP, 254);
                    ADV_sendBCSH(0x0a, brightness - 128);
                    break;
                case IRKeyLeft:
                    brightness = MAX(brightness - STEP, 0);
                    ADV_sendBCSH(0x0a, brightness - 128);
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
        }
        OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW3);
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_ADCCALIB_RUNNING);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Bright;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Saturation;
                    break;
                case IRKeyRight:
                    contrast = MIN(contrast + STEP, 254);
                    ADV_sendBCSH(0x08, contrast);
                    break;
                case IRKeyLeft:
                    contrast = MAX(contrast - STEP, 0);
                    ADV_sendBCSH(0x08, contrast);
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
            OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
        }
        OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW3);
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Contrast;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Default;
                    break;
                case IRKeyRight:
                    saturation = MIN(saturation + STEP, 254);
                    ADV_sendBCSH(0xe3, saturation);
                    break;
                case IRKeyLeft:
                    saturation = MAX(saturation - STEP, 0);
                    ADV_sendBCSH(0xe3, saturation);
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(3);
            OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
        }
        OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW3);
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW2);
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Saturation;
                    break;
                case IRKeyOk:
                    ADV_sendBCSH('D', 'E');
                    brightness = 128;
                    contrast = 128;
                    saturation = 128;
                    saveUserPrefs();
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
            OSD_handleCommand(OSD_CMD_SYS_PAGE1);
        }
        OSD_handleCommand(OSD_CMD_SYS_PAGE1_VALUES);
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    oled_menuItem = OLED_SystemSettings_MatchedPresets;
                    break;
                case IRKeyOk:
                    rgbComponentMode = !rgbComponentMode;
                    if (rgbComponentMode > 1)
                        rgbComponentMode = 0;
                    ADV_sendCompatibility(rgbComponentMode);
                    if (GBS::ADC_INPUT_SEL::read())
                        applyVideoModePreset();
                    break;
                case IRKeyExit:
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
    //     if (results.value == IRKeyUp) {
    //         highlightIcon(1);
    //         OSD_handleCommand(OSD_CMD_SYS_PAGE2);
    //     }
    //     OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_SystemSettings;
    //                 break;
    //             case IRKeyUp:
    //                 oled_menuItem = OLED_SystemSettings_UseUpscaling;
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
    //                 OSD_handleCommand(OSD_CMD_SYS_PAGE1);
    //                 break;
    //             case IRKeyDown:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_SYS_PAGE2);
    //                 oled_menuItem = OLED_SystemSettings_Force5060Hz;
    //                 break;
    //             case IRKeyRight:
    //                 serialCommand = 'L';
    //                 break;
    //             case IRKeyLeft:
    //                 serialCommand = 'L';
    //                 break;
    //             case IRKeyExit:
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
    //     if (results.value == IRKeyDown || results.value == IRKeyUp) {
    //         highlightIcon(2);
    //     }
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyUp:
    //                 selectedMenuLine = 1;
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_SystemSettings;
    //                 break;
    //             case IRKeyDown:
    //                 selectedMenuLine = 3;
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_ResetSettings;
    //                 break;
    //             case IRKeyOk:
    //                 oled_menuItem = OLED_Developer_MemoryAdjust;
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //                 break;
    //             case IRKeyExit:
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
    //     if (results.value == IRKeyUp) {
    //         highlightIcon(1);
    //         OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //     }
    //     OSD_handleCommand(OSD_CMD_DEV_MEMORY_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_Developer;
    //                 break;
    //             case IRKeyDown:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //                 oled_menuItem = OLED_Developer_HSyncAdjust;
    //                 break;
    //             case IRKeyRight:
    //                 serialCommand = '+';
    //                 break;
    //             case IRKeyLeft:
    //                 serialCommand = '-';
    //                 break;
    //             case IRKeyExit:
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
    //     if (results.value == IRKeyDown || results.value == IRKeyUp) {
    //         highlightIcon(2);
    //     }
    //     OSD_handleCommand(OSD_CMD_DEV_MEMORY_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_Developer;
    //                 break;
    //             case IRKeyUp:
    //                 selectedMenuLine = 1;
    //                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //                 oled_menuItem = OLED_Developer_MemoryAdjust;
    //                 break;
    //             case IRKeyDown:
    //                 selectedMenuLine = 3;
    //                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //                 oled_menuItem = OLED_Developer_HTotalAdjust;
    //                 break;
    //             case IRKeyRight:
    //                 serialCommand = '0';
    //                 break;
    //             case IRKeyLeft:
    //                 serialCommand = '1';
    //                 break;
    //             case IRKeyExit:
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
    //     if (results.value == IRKeyDown || results.value == IRKeyUp) {
    //         highlightIcon(3);
    //         OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //     }
    //     OSD_handleCommand(OSD_CMD_DEV_MEMORY_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_Developer;
    //                 break;
    //             case IRKeyUp:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //                 oled_menuItem = OLED_Developer_HSyncAdjust;
    //                 break;
    //             case IRKeyDown:
    //                 oled_menuItem = OLED_Developer_DebugView;
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_DEV_DEBUG);
    //                 break;
    //             case IRKeyRight:
    //                 serialCommand = 'a';
    //                 break;
    //             case IRKeyLeft:
    //                 serialCommand = 'A';
    //                 break;
    //             case IRKeyExit:
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
    //     if (results.value == IRKeyUp) {
    //         highlightIcon(1);
    //         OSD_handleCommand(OSD_CMD_DEV_DEBUG);
    //     }
    //     OSD_handleCommand(OSD_CMD_DEV_DEBUG_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_Developer;
    //                 break;
    //             case IRKeyUp:
    //                 oled_menuItem = OLED_Developer_HTotalAdjust;
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
    //                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //                 break;
    //             case IRKeyDown:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_DEV_DEBUG);
    //                 oled_menuItem = OLED_Developer_ADCFilter;
    //                 break;
    //             case IRKeyRight:
    //                 serialCommand = 'D';
    //                 break;
    //             case IRKeyLeft:
    //                 serialCommand = 'D';
    //                 break;
    //             case IRKeyExit:
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
    //     if (results.value == IRKeyDown || results.value == IRKeyUp) {
    //         highlightIcon(2);
    //     }
    //     OSD_handleCommand(OSD_CMD_DEV_DEBUG_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_Developer;
    //                 break;
    //             case IRKeyUp:
    //                 selectedMenuLine = 1;
    //                 OSD_handleCommand(OSD_CMD_DEV_DEBUG);
    //                 oled_menuItem = OLED_Developer_DebugView;
    //                 break;
    //             case IRKeyDown:
    //                 selectedMenuLine = 3;
    //                 OSD_handleCommand(OSD_CMD_DEV_DEBUG);
    //                 oled_menuItem = OLED_Developer_FreezeCapture;
    //                 break;
    //             case IRKeyRight:
    //                 serialCommand = 'F';
    //                 break;
    //             case IRKeyLeft:
    //                 serialCommand = 'F';
    //                 break;
    //             case IRKeyExit:
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
    //     if (results.value == IRKeyDown || results.value == IRKeyUp) {
    //         highlightIcon(3);
    //         OSD_handleCommand(OSD_CMD_DEV_DEBUG);
    //     }
    //     OSD_handleCommand(OSD_CMD_DEV_DEBUG_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_Developer;
    //                 break;
    //             case IRKeyUp:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_DEV_DEBUG);
    //                 oled_menuItem = OLED_Developer_ADCFilter;
    //                 break;
    //             case IRKeyDown:
    //                 oled_menuItem = OLED_Developer_MemoryAdjust;
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
    //                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
    //                 break;
    //             case IRKeyRight:
    //                 userCommand = 'F';
    //                 break;
    //             case IRKeyLeft:
    //                 userCommand = 'F';
    //                 break;
    //             case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            selectedMenuLine = 1;
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_Input_RGBs;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
                    selectedMenuLine = 1;
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
        }
        if (irDecode()) {
            switch (results.value) {
                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    rgbComponentMode = 1;
                    InputRGBs_mode(rgbComponentMode);
                    rto->isInLowPowerMode = false;
                    break;
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
                    oled_menuItem = OLED_Input_RGsB;
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
            OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
        }
        if (irDecode()) {
            switch (results.value) {
                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    rgbComponentMode = 1;
                    InputRGsB_mode(rgbComponentMode);
                    break;
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
                    oled_menuItem = OLED_Input_RGBs;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
                    oled_menuItem = OLED_Input_VGA;
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(3);
            OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
        }
        if (irDecode()) {
            switch (results.value) {
                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    rgbComponentMode = 0;
                    InputVGA_mode(rgbComponentMode);
                    break;
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
                    oled_menuItem = OLED_Input_RGsB;
                    break;
                case IRKeyDown:
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
                    oled_menuItem = OLED_Input_YPBPR;
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
        }
        OSD_handleCommand(OSD_CMD_INPUT_FORMAT);
        if (irDecode()) {
            switch (results.value) {
                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    InputYUV();
                    break;
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE1);
                    oled_menuItem = OLED_Input_VGA;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
                    oled_menuItem = OLED_Input_SV;
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
            OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
        }
        OSD_handleCommand(OSD_CMD_INPUT_FORMAT);
        if (irDecode()) {
            switch (results.value) {
                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    InputSV_mode(SVModeOption + 1);
                    break;
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
                    oled_menuItem = OLED_Input_YPBPR;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
                    oled_menuItem = OLED_Input_AV;
                    break;
                case IRKeyLeft:
                    if (SVModeOption <= MODEOPTION_MIN)
                        SVModeOption = MODEOPTION_MAX;
                    SVModeOption--;
                    SVModeOptionChanged = 1;
                    break;
                case IRKeyRight:
                    SVModeOption++;
                    if (SVModeOption >= MODEOPTION_MAX)
                        SVModeOption = MODEOPTION_MIN;
                    SVModeOptionChanged = 1;
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(3);
            OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
        }
        OSD_handleCommand(OSD_CMD_INPUT_FORMAT);
        if (irDecode()) {
            switch (results.value) {
                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    InputAV_mode(AVModeOption + 1);
                    break;
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_INPUT_PAGE2);
                    oled_menuItem = OLED_Input_SV;
                    break;
                case IRKeyLeft:
                    if (AVModeOption <= MODEOPTION_MIN)
                        AVModeOption = MODEOPTION_MAX;
                    AVModeOption--;
                    AVModeOptionChanged = 1;
                    break;
                case IRKeyRight:
                    AVModeOption++;
                    if (AVModeOption >= MODEOPTION_MAX)
                        AVModeOption = MODEOPTION_MIN;
                    AVModeOptionChanged = 1;
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(3);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_ScreenSettings_Move;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SCREEN_SETTINGS);
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            selectedMenuLine = 2;
            highlightIcon(2);
            OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    oled_menuItem = OLED_ResetSettings;
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_ColorSettings_RGB_R;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_COLOR_RGB_LABELS);
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyUp) {
            highlightIcon(1);
            OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_ScreenSettings;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE1_UPDATE);
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
                    OSD_handleCommand(OSD_CMD_SYS_PAGE1);
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyOk) {
            highlightIcon(3);
            OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyOk:
                    userCommand = '1';
                    break;
                case IRKeyExit:
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

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
        }

        OSD_handleCommand(OSD_CMD_SYS_PAGE5_VALUES);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    exitMenu();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE5);
                    oled_menuItem = OLED_EnableOTA;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CMD_SYS_PAGE5);
                    oled_menuItem = OLED_ResetDefaults;
                    break;
                case IRKeyOk:
                    userCommand = 'a';
                    break;
                case IRKeyExit:
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
    //     if (results.value == IRKeyUp) {
    //         highlightIcon(1);
    //         OSD_handleCommand(OSD_CMD_SYS_PAGE5);
    //     }
    //     OSD_handleCommand(OSD_CMD_SYS_PAGE5_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_ResetSettings;
    //                 break;
    //             case IRKeyDown:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_SYS_PAGE5);
    //                 oled_menuItem = OLED_Restart;
    //                 break;
    //             case IRKeyRight:
    //                 serialCommand = 'c';
    //                 break;
    //             case IRKeyLeft:
    //                 serialCommand = 'c';
    //                 break;
    //             case IRKeyExit:
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
    //     if (results.value == IRKeyDown || results.value == IRKeyUp) {
    //         highlightIcon(3);
    //     }
    //     OSD_handleCommand(OSD_CMD_SYS_PAGE5_VALUES);
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu:
    //                 OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
    //                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
    //                 oled_menuItem = OLED_ResetSettings;
    //                 break;
    //             case IRKeyUp:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand(OSD_CMD_SYS_PAGE5);
    //                 oled_menuItem = OLED_Restart;
    //                 break;
    //             case IRKeyOk:
    //                 userCommand = '1';
    //                 break;
    //             case IRKeyExit:
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
        currentColor = OSD_TEXT_SELECTED;
        currentRow = ROW_1;
        OSD_writeString(1, "Line input volume");
        // Display 2-digit volume value at positions 20-21 (0-50 range)
        currentColor = OSD_TEXT_NORMAL;
        currentRow = ROW_1;
        OSD_writeChar(digitChars[osdDisplayValue / 10], 20);  // tens
        OSD_writeChar(digitChars[osdDisplayValue % 10], 21);  // units

        if (irDecode()) {
            switch (results.value) {
                case kRecv2:
                    volume = MAX(volume - 1, 0);
                    osdDisplayValue = 50 - volume;
                    PT_2257(volume);
                    break;
                case kRecv3:
                    volume = MIN(volume + 1, 50);
                    osdDisplayValue = 50 - volume;
                    PT_2257(volume);
                    break;
                case IRKeyMenu:
                case IRKeyExit:
                    exitMenu();
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    showSavingFeedback(ROW_1);
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
    currentColor = OSD_CURSOR_ACTIVE;
    currentRow = ROW_1;
    OSD_writeString(0, "Info:");
    currentColor = OSD_TEXT_NORMAL;
    OSD_writeString(26, "Hz");

    // Row 1: Output resolution (positions 6-14) - clear then write
    OSD_clearRowContent(ROW_1, 15, 6);
    currentColor = OSD_TEXT_NORMAL;
    currentRow = ROW_1;
    OSD_writeString(6, getOutputResolutionName(rto->presetID));

    // Row 1: Input type (positions 17-22) - clear then write
    OSD_clearRowContent(ROW_1, 23, 17);
    currentColor = OSD_TEXT_NORMAL;
    currentRow = ROW_1;
    OSD_writeString(18, getInputTypeName(inputType));

    // Row 1: Frame rate
    // Display frame rate (2 digits, 0-99 Hz range)
    osdDisplayValue = ofr;
    currentColor = OSD_TEXT_NORMAL;
    currentRow = ROW_1;
    OSD_writeChar(digitChars[osdDisplayValue / 10], 24);  // tens
    OSD_writeChar(digitChars[osdDisplayValue % 10], 25);  // units

    OSD_clearRowContent(ROW_2, 28, 0);

    currentColor = OSD_CURSOR_ACTIVE;
    currentRow = ROW_2;

    OSD_writeString(0, "Current:");

    currentColor = OSD_TEXT_NORMAL;
    currentRow = ROW_2;

    OSD_writeString(0xFF, " ");
    if ((rto->sourceDisconnected || !rto->boardHasPower || isInfoDisplayActive == 1)) {
        OSD_writeString(0xFF, "No Input");
    } else if (((currentInput == 1) || (inputType == InputTypeRGBs || inputType == InputTypeRGsB || inputType == InputTypeVGA))) {
        OSD_writeCharRow2(B, P16, OSD_BACKGROUND);
        OSD_writeString(0xFF, "RGB ");
        vsyncActive = GBS::STATUS_SYNC_PROC_VSACT::read();
        if (vsyncActive) {
            hsyncActive = GBS::STATUS_SYNC_PROC_HSACT::read();
            if (hsyncActive) {
                OSD_writeString(0xFF, "HV   ");
            }
        } else if ((inputType == InputTypeVGA) && ((!vsyncActive || !hsyncActive))) {
            OSD_writeCharRow2(B, P11, OSD_BACKGROUND);
            OSD_writeString(0x09, "No Input");
        }
    } else if ((rto->continousStableCounter > 35 || currentInput != 1) || (inputType == InputTypeYUV || inputType == InputTypeSV || inputType == InputTypeAV)) {
        OSD_writeCharRow2(B, P16, OSD_BACKGROUND);
        if (inputType == InputTypeYUV)
            OSD_writeString(0xFF, "  YPBPR  ");
        else if (inputType == InputTypeSV)
            OSD_writeString(0xFF, "   SV    ");
        else if (inputType == InputTypeAV)
            OSD_writeString(0xFF, "   AV    ");
    } else {
        OSD_writeString(0xFF, "No Input");
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
                OSD_writeString(0xFF, "   576p");
            } else if (S0_Read_Resolution & 0x20) {
                if (abs(GBS::STATUS_SYNC_PROC_VTOTAL::read() - 312) <= 10)
                    OSD_writeString(0xFF, "   288p");
                else
                    OSD_writeString(0xFF, "   576i");
            } else if (S0_Read_Resolution & 0x10) {
                OSD_writeString(0xFF, "   480p");
            } else if (S0_Read_Resolution & 0x08) {
                if (abs(GBS::STATUS_SYNC_PROC_VTOTAL::read() - 262) <= 10)
                    OSD_writeString(0xFF, "   240p");
                else
                    OSD_writeString(0xFF, "   480i");
            } else {
                OSD_writeString(0xFF, "   Err");
            }
        } else {
            OSD_writeString(0xFF, "   Err");
        }
    }

    if (irDecode()) {
        switch (results.value) {
            case IRKeyMenu:
                exitMenu();
                break;
            case IRKeyExit:
                selectedMenuLine = 1;
                OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
                oled_menuItem = OLED_Input;
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
        case IRKeyMenu:
        case IRKeySave:
        case IRKeyInfo:
        case IRKeyRight:
        case IRKeyLeft:
        case IRKeyUp:
        case IRKeyDown:
        case IRKeyOk:
        case IRKeyExit:
        case IRKeyMute:
        case kRecv2:
        case kRecv3:
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
    currentRow = ROW_2;

    // Display countdown timer
    if (secondsRemaining >= 10) {
        OSD_writeCharRow2((secondsRemaining / 10) + '0', P11, OSD_TEXT_NORMAL);
        OSD_writeCharRow2((secondsRemaining % 10) + '0', P12, OSD_TEXT_NORMAL);
        OSD_writeString(14, " s ");
    } else {
        OSD_writeCharRow2('0', P12, OSD_BACKGROUND);
        OSD_writeCharRow2(secondsRemaining + '0', P11, OSD_TEXT_NORMAL);
        OSD_writeString(13, " s ");
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
    OSD_fillRowBackground(ROW_1, _9, OSD_BACKGROUND);

    // Display on OSD
    for (int i = 0; i <= 800; i++) {
        currentColor = OSD_TEXT_SELECTED;
        currentRow = ROW_1;
        OSD_writeCharAt(M, _1), OSD_writeCharAt(U, _2), OSD_writeCharAt(T, _3), OSD_writeCharAt(E, _4);
        currentColor = OSD_TEXT_NORMAL;
        if (muted) {
            OSD_writeCharAt(O, _6), OSD_writeCharAt(N, _7);
        } else {
            OSD_writeCharAt(O, _6), OSD_writeCharAt(F, _7), OSD_writeCharAt(F, _8);
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
    OSD_fillRowBackground(ROW_1, _9, OSD_BACKGROUND);
    OSD_clearRow1Colors();
    OSD_init();
}

// Handle mute toggle
static void IR_handleMuteToggle(void)
{
    lastMenuItemTime = millis();

    if (audioMuted) {
        PT_MUTE(0x78);  // Unmute
        IR_displayMuteStatus(false);
        audioMuted = 0;
    } else {
        PT_MUTE(0x79);  // Mute
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
        OSD_fillRowBackground(ROW_1, _27, OSD_BACKGROUND);
        OSD_fillRowBackground(ROW_2, _27, OSD_BACKGROUND);
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
    OSD_fillRowBackground(ROW_1, _27, OSD_BACKGROUND);
    OSD_fillRowBackground(ROW_2, _27, OSD_BACKGROUND);
    oled_menuItem = OLED_Info_Display;
}

// Handle Volume keys - opens volume adjust menu
static void IR_handleVolumeKeyPress(void)
{
    lastMenuItemTime = millis();
    NEW_OLED_MENU = false;
    OSD_fillRowBackground(ROW_1, _25, OSD_BACKGROUND);
    oled_menuItem = OLED_Volume_Adjust;
}

void IR_handleInput()
{
    if (!irDecode()) {
        return;
    }

    switch (results.value) {
        case IRKeyMenu:
            IR_handleMenuKeyPress();
            break;
        case IRKeySave:
            IR_handleSaveKeyPress();
            break;
        case IRKeyInfo:
            IR_handleInfoKeyPress();
            break;
        case IRKeyMute:
            IR_handleMuteToggle();
            break;
        case kRecv2:
        case kRecv3:
            IR_handleVolumeKeyPress();
            break;
    }

    irResume();
    delay(5);
}
