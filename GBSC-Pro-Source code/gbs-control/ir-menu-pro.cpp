// ====================================================================================
// ir-menu-pro.cpp
// IR Remote and Menu Navigation
//
// This file contains:
// - IR_handleMenuSelection(): Main menu state machine for IR remote navigation
// - IR_handleInput(): IR remote input handler and decoder
// ====================================================================================

#include "ir-menu-pro.h"
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

// Display menu item on OLED (replaces 10 lines with 1)
static void showMenu(const char* title, const char* label) {
    if (oledClearFlag) display.clear();
    oledClearFlag = ~0;
    display.setColor(OLEDDISPLAY_COLOR::WHITE);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(1, 0, title);
    display.drawString(1, 28, label);
    display.display();
}

// Highlight icon at position (1=top, 2=mid, 3=bottom)
static void highlightIcon(uint8_t pos) {
    OSD_writeCharRow1(icon4, P0, pos == 1 ? yellow : blue_fill);
    OSD_writeCharRow2(icon4, P0, pos == 2 ? yellow : blue_fill);
    OSD_writeCharRow3(icon4, P0, pos == 3 ? yellow : blue_fill);
}

// Display menu with ON/OFF toggle
static void showMenuToggle(const char* title, const char* label, bool isOn) {
    if (oledClearFlag) display.clear();
    oledClearFlag = ~0;
    display.setColor(OLEDDISPLAY_COLOR::WHITE);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(1, 0, title);
    display.drawString(1, 22, label);
    display.drawString(1, 44, isOn ? "ON" : "OFF");
    display.display();
}

// Display menu with custom value text
static void showMenuValue(const char* title, const char* label, const char* value) {
    if (oledClearFlag) display.clear();
    oledClearFlag = ~0;
    display.setColor(OLEDDISPLAY_COLOR::WHITE);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(1, 0, title);
    display.drawString(1, 22, label);
    display.drawString(1, 44, value);
    display.display();
}

// Exit to Input menu (replaces 3 lines with 1)
static void exitToInput() {
    OSD_handleCommand(OSD_CROSS_TOP);
    OSD_handleCommand('1');
    oled_menuItem = OLED_Input;
}

// Exit menu completely (replaces 3 lines with 1)
static void exitMenu() {
    oled_menuItem = OLED_None;
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
            OSD_handleCommand('1');
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_ScreenSettings;
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_OutputResolution_1080;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('3');
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
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('3');
                    oled_menuItem = OLED_OutputResolution_1024;
                    break;
                case IRKeyOk:
                    userCommand = 's';
                    break;
                case IRKeyExit:
                    exitToInput();
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
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('3');
                    oled_menuItem = OLED_OutputResolution_1080;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('3');
                    oled_menuItem = OLED_OutputResolution_960;
                    break;
                case IRKeyOk:
                    userCommand = 'p';
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('3');
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('3');
                    oled_menuItem = OLED_OutputResolution_1024;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_OutputResolution_720;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('4');
                    break;
                case IRKeyOk:
                    userCommand = 'f';
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('4');
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_OutputResolution_960;
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('3');
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('4');
                    oled_menuItem = OLED_OutputResolution_480;
                    break;
                case IRKeyOk:
                    userCommand = 'g';
                    break;
                case IRKeyExit:
                    exitToInput();
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
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('4');
                    oled_menuItem = OLED_OutputResolution_720;
                    break;
                case IRKeyOk:
                    userCommand = 'h';
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('4');
        }
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('4');
                    oled_menuItem = OLED_OutputResolution_720;
                    break;
                case IRKeyOk:
                    break;
                case IRKeyExit:
                    exitToInput();
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
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('4');
                    oled_menuItem = OLED_OutputResolution_PassThrough;
                    break;
                case IRKeyRight:
                    keepSettings = 0;
                    OSD_handleCommand('!');
                    break;
                case IRKeyLeft:
                    keepSettings = 1;
                    OSD_handleCommand('!');
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

                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('4');
                    oled_menuItem = OLED_OutputResolution_PassThrough;
                    break;
                case IRKeyExit:
                    exitToInput();
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
    //         OSD_handleCommand('4');
    //     }
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu:
    //                 OSD_handleCommand(OSD_CROSS_MID);
    //                 OSD_handleCommand('1');
    //                 oled_menuItem = OLED_OutputResolution;
    //                 break;
    //             case IRKeyUp:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand('4');
    //                 oled_menuItem = OLED_OutputResolution_480;
    //                 break;
    //             case IRKeyDown:
    //                 oled_menuItem = OLED_OutputResolution_PassThrough;
    //                 OSD_handleCommand(OSD_CROSS_TOP);
    //                 OSD_handleCommand('5');
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
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_ScreenSettings;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('6');
                    oled_menuItem = OLED_ScreenSettings_Scale;
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_ScreenSettings_MoveAdjust;
                    OSD_handleCommand('6');
                    OSD_writeCharRow1(0x3E, P5, main0);
                    OSD_writeCharRow1(0x3E, P6, main0);
                    OSD_writeCharRow1(0x3E, P7, main0);
                    OSD_writeCharRow1(0x3E, P8, main0);
                    OSD_writeCharRow1(0x3E, P9, main0);
                    OSD_writeCharRow1(0x3E, P10, main0);
                    OSD_writeCharRow1(0x3E, P11, main0);
                    OSD_writeCharRow1(0x3E, P12, main0);
                    OSD_writeCharRow1(0x3E, P13, main0);
                    OSD_writeCharRow1(0x08, P15, yellow);
                    OSD_writeCharRow1(0x18, P16, yellow);
                    OSD_writeCharRow1(0x03, P14, yellow);
                    OSD_writeCharRow1(0x13, P17, yellow);
                    break;
                case IRKeyExit:
                    exitToInput();
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
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_ScreenSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('6');
                    oled_menuItem = OLED_ScreenSettings_Move;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('6');
                    oled_menuItem = OLED_ScreenSettings_Borders;
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_ScreenSettings_ScaleAdjust;
                    OSD_handleCommand('6');
                    OSD_writeCharRow2(0x3E, P6, main0);
                    OSD_writeCharRow2(0x3E, P7, main0);
                    OSD_writeCharRow2(0x3E, P8, main0);
                    OSD_writeCharRow2(0x3E, P9, main0);
                    OSD_writeCharRow2(0x3E, P10, main0);
                    OSD_writeCharRow2(0x3E, P11, main0);
                    OSD_writeCharRow2(0x3E, P12, main0);
                    OSD_writeCharRow2(0x3E, P13, main0);
                    OSD_writeCharRow2(0x08, P15, yellow);
                    OSD_writeCharRow2(0x18, P16, yellow);
                    OSD_writeCharRow2(0x03, P14, yellow);
                    OSD_writeCharRow2(0x13, P17, yellow);
                    break;
                case IRKeyExit:
                    exitToInput();
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
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_ScreenSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('6');
                    oled_menuItem = OLED_ScreenSettings_Scale;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_ScreenSettings_FullHeight;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('o');
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_ScreenSettings_BordersAdjust;
                    OSD_handleCommand('6');
                    OSD_writeCharRow3(0x3E, P8, main0);
                    OSD_writeCharRow3(0x3E, P9, main0);
                    OSD_writeCharRow3(0x3E, P10, main0);
                    OSD_writeCharRow3(0x3E, P11, main0);
                    OSD_writeCharRow3(0x3E, P12, main0);
                    OSD_writeCharRow3(0x3E, P13, main0);
                    OSD_writeCharRow3(0x08, P15, yellow);
                    OSD_writeCharRow3(0x18, P16, yellow);
                    OSD_writeCharRow3(0x03, P14, yellow);
                    OSD_writeCharRow3(0x13, P17, yellow);
                    break;
                case IRKeyExit:
                    exitToInput();
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_ScreenSettings_MoveAdjust) {
        if (results.value == IRKeyOk) {
            OSD_handleCommand('6');
            OSD_writeCharRow1(0x3E, P5, main0);
            OSD_writeCharRow1(0x3E, P6, main0);
            OSD_writeCharRow1(0x3E, P7, main0);
            OSD_writeCharRow1(0x3E, P8, main0);
            OSD_writeCharRow1(0x3E, P9, main0);
            OSD_writeCharRow1(0x3E, P10, main0);
            OSD_writeCharRow1(0x3E, P11, main0);
            OSD_writeCharRow1(0x3E, P12, main0);
            OSD_writeCharRow1(0x3E, P13, main0);
            OSD_writeCharRow1(0x08, P15, yellow);
            OSD_writeCharRow1(0x18, P16, yellow);
            OSD_writeCharRow1(0x03, P14, yellow);
            OSD_writeCharRow1(0x13, P17, yellow);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('6');
                    oled_menuItem = OLED_ScreenSettings_Move;
                    break;
                case IRKeyOk:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('6');
                    oled_menuItem = OLED_ScreenSettings_Move;
                    break;
                case IRKeyRight:
                    lastMenuItemTime = millis();
                    serialCommand = '6';
                    if (GBS::IF_HBIN_SP::read() >= 10) {
                    } else {
                        for (int p = 0; p <= 400; p++) {
                            currentColor = red;
                            currentRow = ROW_1;
                            writeChar(l, _20);
                            writeChar(i, _21);
                            writeChar(m, _22);
                            writeChar(i, _23);
                            writeChar(t, _24);
                            writeChar(0x0d, _25);
                        }
                    }
                    currentColor = blue_fill;
                    currentRow = ROW_1;
                    writeChar(l, _20);
                    writeChar(i, _21);
                    writeChar(m, _22);
                    writeChar(i, _23);
                    writeChar(t, _24);
                    writeChar(0x0d, _25);
                    break;
                case IRKeyLeft:
                    lastMenuItemTime = millis();
                    serialCommand = '7';
                    if (GBS::IF_HBIN_SP::read() < 0x150) {
                    } else {
                        for (int p = 0; p <= 400; p++) {
                            currentColor = red;
                            currentRow = ROW_1;
                            writeChar(l, _20);
                            writeChar(i, _21);
                            writeChar(m, _22);
                            writeChar(i, _23);
                            writeChar(t, _24);
                            writeChar(0x0d, _25);
                        }
                    }
                    currentColor = blue_fill;
                    currentRow = ROW_1;
                    writeChar(l, _20);
                    writeChar(i, _21);
                    writeChar(m, _22);
                    writeChar(i, _23);
                    writeChar(t, _24);
                    writeChar(0x0d, _25);
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
                    exitToInput();
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_ScreenSettings_ScaleAdjust) {
        if (results.value == IRKeyOk) {
            OSD_handleCommand('6');
            OSD_writeCharRow2(0x3E, P6, main0);
            OSD_writeCharRow2(0x3E, P7, main0);
            OSD_writeCharRow2(0x3E, P8, main0);
            OSD_writeCharRow2(0x3E, P9, main0);
            OSD_writeCharRow2(0x3E, P10, main0);
            OSD_writeCharRow2(0x3E, P11, main0);
            OSD_writeCharRow2(0x3E, P12, main0);
            OSD_writeCharRow2(0x3E, P13, main0);
            OSD_writeCharRow2(0x08, P15, yellow);
            OSD_writeCharRow2(0x18, P16, yellow);
            OSD_writeCharRow2(0x03, P14, yellow);
            OSD_writeCharRow2(0x13, P17, yellow);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('6');
                    oled_menuItem = OLED_ScreenSettings_Scale;
                    break;
                case IRKeyOk:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('6');
                    oled_menuItem = OLED_ScreenSettings_Scale;
                    break;
                case IRKeyRight:
                    lastMenuItemTime = millis();
                    serialCommand = 'h';
                    if (GBS::VDS_HSCALE::read() == 1023) {
                        for (int p = 0; p <= 400; p++) {
                            currentColor = red;
                            currentRow = ROW_2;
                            OSD_writeString(20, "limit");
                            writeChar(0x0d, _25);
                        }
                    }
                    currentColor = blue_fill;
                    currentRow = ROW_2;
                    OSD_writeString(20, "limit");
                    writeChar(0x0d, _25);
                    break;
                case IRKeyLeft:
                    lastMenuItemTime = millis();
                    serialCommand = 'z';
                    if (GBS::VDS_HSCALE::read() <= 256) {
                        for (int p = 0; p <= 400; p++) {
                            currentColor = red;
                            currentRow = ROW_2;
                            OSD_writeString(20, "limit");
                            writeChar(0x0d, _25);
                        }
                    }
                    currentColor = blue_fill;
                    currentRow = ROW_2;
                    OSD_writeString(20, "limit");
                    writeChar(0x0d, _25);
                    break;
                case IRKeyUp:
                    lastMenuItemTime = millis();
                    serialCommand = '5';
                    if (GBS::VDS_VSCALE::read() == 1023) {
                        for (int p = 0; p <= 400; p++) {
                            currentColor = red;
                            currentRow = ROW_2;
                            OSD_writeString(20, "limit");
                            writeChar(0x0d, _25);
                        }
                    }
                    currentColor = blue_fill;
                    currentRow = ROW_2;
                    OSD_writeString(20, "limit");
                    writeChar(0x0d, _25);
                    break;
                case IRKeyDown:
                    lastMenuItemTime = millis();
                    serialCommand = '4';
                    if (GBS::VDS_VSCALE::read() <= 256) {
                        for (int p = 0; p <= 400; p++) {
                            currentColor = red;
                            currentRow = ROW_2;
                            OSD_writeString(20, "limit");
                            writeChar(0x0d, _25);
                        }
                    }
                    currentColor = blue_fill;
                    currentRow = ROW_2;
                    OSD_writeString(20, "limit");
                    writeChar(0x0d, _25);
                    break;
                case IRKeyExit:
                    exitToInput();
                    break;
            }
            irResume();
        }
        return true;
    }

    if (oled_menuItem == OLED_ScreenSettings_BordersAdjust) {
        if (results.value == IRKeyOk) {
            OSD_handleCommand('6');
            OSD_writeCharRow3(0x3E, P8, main0);
            OSD_writeCharRow3(0x3E, P9, main0);
            OSD_writeCharRow3(0x3E, P10, main0);
            OSD_writeCharRow3(0x3E, P11, main0);
            OSD_writeCharRow3(0x3E, P12, main0);
            OSD_writeCharRow3(0x3E, P13, main0);
            OSD_writeCharRow3(0x08, P15, yellow);
            OSD_writeCharRow3(0x18, P16, yellow);
            OSD_writeCharRow3(0x03, P14, yellow);
            OSD_writeCharRow3(0x13, P17, yellow);
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('6');
                    oled_menuItem = OLED_ScreenSettings_Borders;
                    break;
                case IRKeyOk:
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('6');
                    oled_menuItem = OLED_ScreenSettings_Borders;
                    break;
                case IRKeyRight:
                    userCommand = 'A';
                    if ((GBS::VDS_DIS_HB_ST::read() > 4) && (GBS::VDS_DIS_HB_SP::read() < (GBS::VDS_HSYNC_RST::read() - 4))) {
                    } else {
                        for (int p = 0; p <= 400; p++) {
                            currentColor = red;
                            currentRow = ROW_3;
                            OSD_writeString(20, "limit");
                            writeChar(0x0d, _25);
                        }
                    }
                    currentColor = blue_fill;
                    currentRow = ROW_3;
                    OSD_writeString(20, "limit");
                    writeChar(0x0d, _25);
                    break;
                case IRKeyLeft:
                    userCommand = 'B';
                    if ((GBS::VDS_DIS_HB_ST::read() < (GBS::VDS_HSYNC_RST::read() - 4)) && (GBS::VDS_DIS_HB_SP::read() > 4)) {
                    } else {
                        for (int p = 0; p <= 400; p++) {
                            currentColor = red;
                            currentRow = ROW_3;
                            OSD_writeString(20, "limit");
                            writeChar(0x0d, _25);
                        }
                    }
                    currentColor = blue_fill;
                    currentRow = ROW_3;
                    OSD_writeString(20, "limit");
                    writeChar(0x0d, _25);
                    break;
                case IRKeyUp:
                    userCommand = 'C';
                    if ((GBS::VDS_DIS_VB_ST::read() > 6) && (GBS::VDS_DIS_VB_SP::read() < (GBS::VDS_VSYNC_RST::read() - 4))) {
                    } else {
                        for (int p = 0; p <= 400; p++) {
                            currentColor = red;
                            currentRow = ROW_3;
                            OSD_writeString(20, "limit");
                            writeChar(0x0d, _25);
                        }
                    }
                    currentColor = blue_fill;
                    currentRow = ROW_3;
                    OSD_writeString(20, "limit");
                    writeChar(0x0d, _25);
                    break;
                case IRKeyDown:
                    userCommand = 'D';
                    if ((GBS::VDS_DIS_VB_ST::read() < (GBS::VDS_VSYNC_RST::read() - 4)) && (GBS::VDS_DIS_VB_SP::read() > 6)) {
                    } else {
                        for (int p = 0; p <= 400; p++) {
                            currentColor = red;
                            currentRow = ROW_3;
                            OSD_writeString(20, "limit");
                            writeChar(0x0d, _25);
                        }
                    }
                    currentColor = blue_fill;
                    currentRow = ROW_3;
                    OSD_writeString(20, "limit");
                    writeChar(0x0d, _25);
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('o');
        }
        OSD_handleCommand('p');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_ScreenSettings;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_ScreenSettings_Borders;
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('6');
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
                    exitToInput();
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
            OSD_handleCommand('a');
        }

        OSD_handleCommand('e');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('d');
                    oled_menuItem = OLED_ColorSettings_RGB_B;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('a');
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
                    exitToInput();
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

        OSD_handleCommand('e');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('a');
                    oled_menuItem = OLED_ColorSettings_ADCGain;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('a');
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
                    exitToInput();
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
            OSD_handleCommand('a');
        }

        OSD_handleCommand('e');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('a');
                    oled_menuItem = OLED_ColorSettings_Scanlines;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_ColorSettings_Sharpness;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('b');
                    break;
                case IRKeyOk:
                    userCommand = 'm';
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('b');
        }

        OSD_handleCommand('f');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_ColorSettings_LineFilter;
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('a');
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('b');
                    oled_menuItem = OLED_ColorSettings_Peaking;
                    break;
                case IRKeyOk:
                    userCommand = 'W';
                    break;
                case IRKeyExit:
                    exitToInput();
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

        OSD_handleCommand('f');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('b');
                    oled_menuItem = OLED_ColorSettings_Sharpness;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('b');
                    oled_menuItem = OLED_ColorSettings_StepResponse;
                    break;
                case IRKeyOk:
                    if (!isPeakingLocked()) {
                        serialCommand = 'f';
                    }
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('b');
        }

        OSD_handleCommand('f');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('b');
                    oled_menuItem = OLED_ColorSettings_Peaking;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_ColorSettings_DefaultColor;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('c');
                    break;
                case IRKeyOk:
                    serialCommand = 'V';
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('d');
        }
        OSD_handleCommand('g');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('d');
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
                    exitToInput();
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
        OSD_handleCommand('g');
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('d');
                    oled_menuItem = OLED_ColorSettings_RGB_R;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('d');
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
                    exitToInput();
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
            OSD_handleCommand('d');
        }
        OSD_handleCommand('g');
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('d');
                    oled_menuItem = OLED_ColorSettings_RGB_G;
                    break;
                case IRKeyDown:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('a');
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
                    exitToInput();
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
            OSD_handleCommand('c');
        }

        OSD_handleCommand('h');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_ColorSettings_RGB_B;
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('d');
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('c');
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
                    exitToInput();
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
        OSD_handleCommand('h');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('c');
                    oled_menuItem = OLED_ColorSettings_Y_Gain;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('c');
                    oled_menuItem = OLED_ColorSettings_DefaultColor;
                    break;
                case IRKeyRight:
                    userCommand = 'V';
                    break;
                case IRKeyLeft:
                    userCommand = 'R';
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('c');
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_ColorSettings_StepResponse;
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('b');
                    break;
                case IRKeyOk:
                    userCommand = 'U';
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('i');
        }

        OSD_handleCommand(j);

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('i');
                    oled_menuItem = OLED_SystemSettings_Compatibility;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_SystemSettings_Deinterlace;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('k');
                    break;
                case IRKeyOk:
                    serialCommand = 'Z';
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('i');
        }

        OSD_handleCommand('j');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('i');
                    oled_menuItem = OLED_SystemSettings_Compatibility;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_SystemSettings_Deinterlace;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('k');
                    break;
                case IRKeyExit:
                    exitToInput();
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

        OSD_handleCommand('l');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('k');
                    oled_menuItem = OLED_SystemSettings_Deinterlace;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('k');
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
                    exitToInput();
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
            OSD_handleCommand('m');
        }
        OSD_handleCommand('n');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('m');
                    oled_menuItem = OLED_SystemSettings_FrameTimeLock;
                    break;
                case IRKeyOk:
                    userCommand = 'X';
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('m');
        }
        OSD_handleCommand('n');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_SystemSettings_LockMethod;
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('k');
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('m');
                    oled_menuItem = OLED_SystemSettings_FrameTimeLock;
                    break;
                case IRKeyOk:
                    userCommand = 'w';
                    break;
                case IRKeyExit:
                    exitToInput();
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

        OSD_handleCommand('n');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('m');
                    oled_menuItem = OLED_SystemSettings_ADCCalibration;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('m');
                    oled_menuItem = OLED_SystemSettings_ClockGenerator;
                    break;
                case IRKeyOk:
                    userCommand = '5';
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('k');
        }

        OSD_handleCommand('l');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('k');
                    oled_menuItem = OLED_SystemSettings_Force5060Hz;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_SystemSettings_ADCCalibration;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('m');
                    break;
                case IRKeyOk:
                    userCommand = 'i';
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('k');
        }

        OSD_handleCommand('l');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_SystemSettings_MatchedPresets;
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('i');
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('k');
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
                    exitToInput();
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
            OSD_handleCommand('i');
        }

        OSD_handleCommand('j');
        if (irDecode()) {
            switch (results.value) {
                case IRKeyOk:
                    if (inputType == InputTypeSV || inputType == InputTypeAV) {
                        selectedMenuLine = 1;
                        OSD_handleCommand(OSD_CROSS_TOP);
                        OSD_handleCommand('^');
                        oled_menuItem = OLED_SystemSettings_SVAVInput_DoubleLine;
                    }
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('i');
                    oled_menuItem = OLED_SystemSettings_Compatibility;
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('^');
        }
        OSD_handleCommand('&');
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('i');
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('^');
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Smooth;
                    break;
                case IRKeyOk:
                    lineOption = !lineOption;
                    settingLineOptionChanged = 1;
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('^');
        }
        OSD_handleCommand('&');
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('i');
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('^');
                    oled_menuItem = OLED_SystemSettings_SVAVInput_DoubleLine;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('^');
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Bright;
                    break;
                case IRKeyOk:
                    if (lineOption) {
                        smoothOption = !smoothOption;
                        settingSmoothOptionChanged = 1;
                    }
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('^');
        }
        OSD_handleCommand('&');
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('i');
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    saveUserPrefs();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('^');
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Smooth;
                    break;
                case IRKeyDown:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('z');
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
                    exitToInput();
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
            OSD_handleCommand('z');
        }
        OSD_handleCommand('A');
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('i');
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
                case IRKeyUp:
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('^');
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Bright;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('z');
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
                    exitToInput();
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
            OSD_handleCommand('z');
        }
        OSD_handleCommand('A');
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('i');
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('z');
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Contrast;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('z');
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
                    exitToInput();
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
            OSD_handleCommand('z');
        }
        OSD_handleCommand('A');
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('i');
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('z');
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
                    exitToInput();
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
            OSD_handleCommand('i');
        }
        OSD_handleCommand('j');
        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('i');
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('i');
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
                    exitToInput();
                    break;
            }
            irResume();
        }
        return true;
    }

    // else if (oled_menuItem == OLED_SystemSettings_ComponentVGA)
    // {
    //     display.clear();
    //     display.drawString(1, 0, "Menu-");
    //     display.drawString(1, 28, "Component/VGA");
    //     display.display();

    //     if (results.value == IRKeyUp)
    //     {
    //         OSD_writeCharRow1(icon4, P0, yellow);
    //         OSD_writeCharRow2(icon4, P0, blue_fill);
    //         OSD_writeCharRow3(icon4, P0, blue_fill);
    //         OSD_handleCommand('k');
    //     }

    //     OSD_handleCommand('l');

    //     if (irDecode())
    //     {
    //         switch (results.value)
    //         {
    //         case IRKeyMenu:
    //             OSD_handleCommand(OSD_CROSS_TOP);
    //             OSD_handleCommand('2');
    //             oled_menuItem = OLED_SystemSettings;
    //             break;
    // case IRKeyUp:
    //     oled_menuItem = OLED_SystemSettings_UseUpscaling;
    //     OSD_handleCommand(OSD_CROSS_BOTTOM);
    //     OSD_handleCommand('i');
    //     break;
    // case IRKeyDown:
    //     selectedMenuLine = 2;
    //     OSD_handleCommand('k'); //
    //     oled_menuItem = OLED_SystemSettings_Force5060Hz;
    //     break;
    //         case IRKeyRight:
    //             serialCommand = 'L';
    //             break;
    //         case IRKeyLeft:
    //             serialCommand = 'L';
    //             break;
    //         case IRKeyExit:
    //             oled_menuItem = OLED_Input;
    //             OSD_clearAll();
    //             OSD_init();
    //             break;
    //         }
    //         irResume();
    //     }
    // }

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
    //                 OSD_handleCommand('2');
    //                 oled_menuItem = OLED_SystemSettings;
    //                 break;
    //             case IRKeyDown:
    //                 selectedMenuLine = 3;
    //                 OSD_handleCommand('2');
    //                 oled_menuItem = OLED_ResetSettings;
    //                 break;
    //             case IRKeyOk:
    //                 oled_menuItem = 104;
    //                 OSD_handleCommand(OSD_CROSS_TOP);
    //                 OSD_handleCommand('q');
    //                 break;
    //             case IRKeyExit:
    //                 exitMenu();
    //                 break;
    //         }
    //         irResume();
    //     }
    // }

    // else if (oled_menuItem == OLED_MemoryAdjust) {
    //     showMenu("Menu-", "MEM left / right");
    //
    //     if (results.value == IRKeyUp) {
    //         highlightIcon(1);
    //         OSD_handleCommand('q');
    //     }
    //     OSD_handleCommand('r');
    //
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu:
    //                 OSD_handleCommand(OSD_CROSS_MID);
    //                 OSD_handleCommand('2');
    //                 oled_menuItem = OLED_Developer;
    //                 break;
    //             case IRKeyDown:
    //                 selectedMenuLine = 2;
    //                 OSD_handleCommand('q');
    //                 oled_menuItem = 105;
    //                 break;
    //             case IRKeyRight:
    //                 serialCommand = '+';
    //                 break;
    //             case IRKeyLeft:
    //                 serialCommand = '-';
    //                 break;
    //             case IRKeyExit:
    //                 exitToInput();
    //                 break;
    //         }
    //         irResume();
    //     }
    // }

    // else if (oled_menuItem == OLED_HSyncAdjust) {
    //     showMenu("Menu-", "HS left / right");
    //     if (results.value == IRKeyDown || results.value == IRKeyUp) highlightIcon(2);
    //     OSD_handleCommand('r');
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu: OSD_handleCommand(OSD_CROSS_MID); OSD_handleCommand('2'); oled_menuItem = OLED_Developer; break;
    //             case IRKeyUp: selectedMenuLine = 1; OSD_handleCommand('q'); oled_menuItem = 104; break;
    //             case IRKeyDown: selectedMenuLine = 3; OSD_handleCommand('q'); oled_menuItem = 106; break;
    //             case IRKeyRight: serialCommand = '0'; break;
    //             case IRKeyLeft: serialCommand = '1'; break;
    //             case IRKeyExit: exitToInput(); break;
    //         }
    //         irResume();
    //     }
    // }

    // else if (oled_menuItem == OLED_HTotalAdjust) {
    //     showMenu("Menu-", "HTotal - / +");
    //     if (results.value == IRKeyDown || results.value == IRKeyUp) { highlightIcon(3); OSD_handleCommand('q'); }
    //     OSD_handleCommand('r');
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu: OSD_handleCommand(OSD_CROSS_MID); OSD_handleCommand('2'); oled_menuItem = OLED_Developer; break;
    //             case IRKeyUp: selectedMenuLine = 2; OSD_handleCommand('q'); oled_menuItem = 105; break;
    //             case IRKeyDown: oled_menuItem = 107; OSD_handleCommand(OSD_CROSS_TOP); OSD_handleCommand('s'); break;
    //             case IRKeyRight: serialCommand = 'a'; break;
    //             case IRKeyLeft: serialCommand = 'A'; break;
    //             case IRKeyExit: exitToInput(); break;
    //         }
    //         irResume();
    //     }
    // }

    // else if (oled_menuItem == OLED_DebugView) {
    //     showMenu("Menu-", "Debug view");
    //     if (results.value == IRKeyUp) { highlightIcon(1); OSD_handleCommand('s'); }
    //     OSD_handleCommand('t');
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu: OSD_handleCommand(OSD_CROSS_MID); OSD_handleCommand('2'); oled_menuItem = OLED_Developer; break;
    //             case IRKeyUp: oled_menuItem = 106; OSD_handleCommand(OSD_CROSS_BOTTOM); OSD_handleCommand('q'); break;
    //             case IRKeyDown: selectedMenuLine = 2; OSD_handleCommand('s'); oled_menuItem = 108; break;
    //             case IRKeyRight: serialCommand = 'D'; break;
    //             case IRKeyLeft: serialCommand = 'D'; break;
    //             case IRKeyExit: exitToInput(); break;
    //         }
    //         irResume();
    //     }
    // }

    // else if (oled_menuItem == OLED_ADCFilter) {
    //     showMenu("Menu-", "ADC filter");
    //     if (results.value == IRKeyDown || results.value == IRKeyUp) highlightIcon(2);
    //     OSD_handleCommand('t');
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu: OSD_handleCommand(OSD_CROSS_MID); OSD_handleCommand('2'); oled_menuItem = OLED_Developer; break;
    //             case IRKeyUp: selectedMenuLine = 1; OSD_handleCommand('s'); oled_menuItem = 107; break;
    //             case IRKeyDown: selectedMenuLine = 3; OSD_handleCommand('s'); oled_menuItem = 153; break;
    //             case IRKeyRight: serialCommand = 'F'; break;
    //             case IRKeyLeft: serialCommand = 'F'; break;
    //             case IRKeyExit: exitToInput(); break;
    //         }
    //         irResume();
    //     }
    // }

    // else if (oled_menuItem == OLED_FreezeCapture) {
    //     showMenu("Menu-", "Freeze capture");
    //     if (results.value == IRKeyDown || results.value == IRKeyUp) { highlightIcon(3); OSD_handleCommand('s'); }
    //     OSD_handleCommand('t');
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu: OSD_handleCommand(OSD_CROSS_MID); OSD_handleCommand('2'); oled_menuItem = OLED_Developer; break;
    //             case IRKeyUp: selectedMenuLine = 2; OSD_handleCommand('s'); oled_menuItem = 108; break;
    //             case IRKeyDown: oled_menuItem = 104; OSD_handleCommand(OSD_CROSS_TOP); OSD_handleCommand('q'); break;
    //             case IRKeyRight: userCommand = 'F'; break;
    //             case IRKeyLeft: userCommand = 'F'; break;
    //             case IRKeyExit: exitToInput(); break;
    //         }
    //         irResume();
    //     }
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
            OSD_handleCommand('1');
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_Input_RGBs;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('@');
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
            OSD_handleCommand('@');
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
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('@');
                    oled_menuItem = OLED_Input_RGsB;
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('@');
        }
        if (irDecode()) {
            switch (results.value) {
                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    rgbComponentMode = 1;
                    InputRGsB_mode(rgbComponentMode);
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('@');
                    oled_menuItem = OLED_Input_RGBs;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('@');
                    oled_menuItem = OLED_Input_VGA;
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('@');
        }
        if (irDecode()) {
            switch (results.value) {
                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    rgbComponentMode = 0;
                    InputVGA_mode(rgbComponentMode);
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('@');
                    oled_menuItem = OLED_Input_RGsB;
                    break;
                case IRKeyDown:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('#');
                    oled_menuItem = OLED_Input_YPBPR;
                    break;
                case IRKeyExit:
                    exitToInput();
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
            OSD_handleCommand('#');
        }
        OSD_handleCommand('$');
        if (irDecode()) {
            switch (results.value) {
                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    InputYUV();
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 3;
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('@');
                    oled_menuItem = OLED_Input_VGA;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('#');
                    oled_menuItem = OLED_Input_SV;
                    break;
                case IRKeyExit:
                    exitToInput();
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Input_SV
    else if (oled_menuItem == OLED_Input_SV) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Input");
        display.drawString(1, 22, "SV");
        switch (SVModeOption) {
            case 0: display.drawString(1, 44, "Auto"); break;
            case 1: display.drawString(1, 44, "PAL"); break;
            case 2: display.drawString(1, 44, "NTSC-M"); break;
            case 3: display.drawString(1, 44, "PAL-60"); break;
            case 4: display.drawString(1, 44, "NTSC443"); break;
            case 5: display.drawString(1, 44, "NTSC-J"); break;
            case 6: display.drawString(1, 44, "PAL-N w/ p"); break;
            case 7: display.drawString(1, 44, "PAL-M w/o p"); break;
            case 8: display.drawString(1, 44, "PAL-M"); break;
            case 9: display.drawString(1, 44, "PAL Cmb -N"); break;
            case 10: display.drawString(1, 44, "PAL Cmb -N w/ p"); break;
            case 11: display.drawString(1, 44, "SECAM"); break;
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(2);
            OSD_handleCommand('#');
        }
        OSD_handleCommand('$');
        if (irDecode()) {
            switch (results.value) {
                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    InputSV_mode(SVModeOption + 1);
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('#');
                    oled_menuItem = OLED_Input_YPBPR;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('#');
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
                    exitToInput();
                    break;
            }
            irResume();
        }
        return true;
    }

    // OLED_Input_AV
    else if (oled_menuItem == OLED_Input_AV) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Input");
        display.drawString(1, 22, "AV");
        switch (AVModeOption) {
            case 0: display.drawString(1, 44, "Auto"); break;
            case 1: display.drawString(1, 44, "PAL"); break;
            case 2: display.drawString(1, 44, "NTSC-M"); break;
            case 3: display.drawString(1, 44, "PAL-60"); break;
            case 4: display.drawString(1, 44, "NTSC443"); break;
            case 5: display.drawString(1, 44, "NTSC-J"); break;
            case 6: display.drawString(1, 44, "PAL-N w/ p"); break;
            case 7: display.drawString(1, 44, "PAL-M w/o p"); break;
            case 8: display.drawString(1, 44, "PAL-M"); break;
            case 9: display.drawString(1, 44, "PAL Cmb -N"); break;
            case 10: display.drawString(1, 44, "PAL Cmb -N w/ p"); break;
            case 11: display.drawString(1, 44, "SECAM"); break;
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            highlightIcon(3);
            OSD_handleCommand('#');
        }
        OSD_handleCommand('$');
        if (irDecode()) {
            switch (results.value) {
                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    InputAV_mode(AVModeOption + 1);
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('#');
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
                    exitToInput();
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
    // OLED_Profile
    if (oled_menuItem == OLED_Profile) {
        showMenu("Menu-", "Profile");

        if (results.value == IRKeyUp) {
            highlightIcon(1);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_SaveConfirm;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot19;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'A';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_SaveConfirm) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Save;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'B';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Save) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Load;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_SaveConfirm;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'C';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Load) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Operation1;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Save;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'D';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Operation1) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Operation2;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Load;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'E';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Operation2) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Operation3;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Operation1;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'F';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Operation3) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot7;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Operation2;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'G';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_SelectSlot) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot1;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Preset12;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'A';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot1) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot2;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'B';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot2) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot3;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot1;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'C';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot3) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot4;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot2;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'D';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot4) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot5;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot3;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'E';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot5) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot6;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot4;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'F';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot6) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_SelectPreset;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot5;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'G';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot7) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot8;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Operation3;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'H';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot8) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot9;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot7;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'I';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot9) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot10;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot8;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'J';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot10) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot11;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot9;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'K';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot11) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot12;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot10;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'L';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot12) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot13;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot11;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'M';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot13) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot14;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot12;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'N';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot14) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot15;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot13;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'O';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot15) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot16;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot14;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'P';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot16) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot17;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot15;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'Q';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot17) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot18;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot16;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'R';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot18) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Slot19;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot17;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'S';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Slot19) {
        if (results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, yellow);
            OSD_writeCharRow2(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot18;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'T';
                    OSD_handleCommand('y');
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_SelectPreset) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Preset1;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Slot6;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'H';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Preset1) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Preset2;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_SelectPreset;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'I';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Preset2) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Preset3;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Preset1;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'J';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Preset3) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Preset4;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Preset2;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'K';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Preset4) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Preset5;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Preset3;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'L';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Preset5) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Preset6;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Preset4;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'M';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Preset6) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Preset7;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Preset5;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'N';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Preset7) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Preset8;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Preset6;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'O';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Preset8) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Preset9;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Preset7;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'P';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Preset9) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Preset10;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Preset8;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'Q';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Preset10) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Preset11;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Preset9;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'R';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Preset11) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_Preset12;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Preset10;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'S';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    else if (oled_menuItem == OLED_Profile_Preset12) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_writeCharRow1(icon4, P0, blue_fill);
            OSD_writeCharRow3(icon4, P0, blue_fill);
            OSD_writeCharRow2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OLED_Profile;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OLED_Profile_SelectSlot;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OLED_Profile_Preset11;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'T';
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    for (int i = 0; i <= 800; i++) {
                        OSD_writeCharRow2(O, P25, 0x14);
                        OSD_writeCharRow2(K, P26, 0x14);
                    }
                    OSD_writeCharRow2(O, P25, blue_fill);
                    OSD_writeCharRow2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

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
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_ScreenSettings_Move;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('6');
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
            OSD_handleCommand('2');
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    oled_menuItem = OLED_ResetSettings;
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_ColorSettings_RGB_R;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('d');
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
            OSD_handleCommand('2');
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyUp:
                    oled_menuItem = OLED_ScreenSettings;
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('1');
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('i');
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
            OSD_handleCommand('2');
        }

        if (irDecode()) {
            switch (results.value) {
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('2');
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

        OSD_handleCommand('v');

        if (irDecode()) {
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ResetSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('u');
                    oled_menuItem = OLED_EnableOTA;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('u');
                    oled_menuItem = OLED_ResetDefaults;
                    break;
                case IRKeyOk:
                    userCommand = 'a';
                    break;
                case IRKeyExit:
                    exitMenu();
                    break;
            }
            irResume();
        }
        return true;
    }

    // else if (oled_menuItem == OLED_EnableOTA) {
    //     showMenu("Menu-", "Enable OTA");
    //     if (results.value == IRKeyUp) { highlightIcon(1); OSD_handleCommand('u'); }
    //     OSD_handleCommand('v');
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu: OSD_handleCommand(OSD_CROSS_BOTTOM); OSD_handleCommand('2'); oled_menuItem = OLED_ResetSettings; break;
    //             case IRKeyDown: selectedMenuLine = 2; OSD_handleCommand('u'); oled_menuItem = OLED_Restart; break;
    //             case IRKeyRight: serialCommand = 'c'; break;
    //             case IRKeyLeft: serialCommand = 'c'; break;
    //             case IRKeyExit: exitToInput(); break;
    //         }
    //         irResume();
    //     }
    //     return true;
    // }

    // else if (oled_menuItem == OLED_ResetDefaults) {
    //     showMenu("Menu-", "Reset defaults");
    //     if (results.value == IRKeyDown || results.value == IRKeyUp) highlightIcon(3);
    //     OSD_handleCommand('v');
    //     if (irDecode()) {
    //         switch (results.value) {
    //             case IRKeyMenu: OSD_handleCommand(OSD_CROSS_BOTTOM); OSD_handleCommand('2'); oled_menuItem = OLED_ResetSettings; break;
    //             case IRKeyUp: selectedMenuLine = 2; OSD_handleCommand('u'); oled_menuItem = OLED_Restart; break;
    //             case IRKeyOk: userCommand = '1'; break;
    //             case IRKeyExit: exitMenu(); break;
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
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(8, 15, "Volume - / + dB");
        display.display();

        osdDisplayValue = 50 - volume;
        currentColor = yellowT;
        currentRow = ROW_1;
        OSD_writeString(1, "Line input volume");
        currentColor = main0;
        digitPos1 = _21;
        digitPos2 = _20;
        digitPos3 = 0x3D;
        OSD_writeCharRow1(o, _19, blue_fill);
        OSD_writeCharRow1(o, _22, blue_fill);
        OSD_writeCharRow1(o, _23, blue_fill);
        OSD_writeCharRow1(o, _24, blue_fill);
        displayNumber3Digit(osdDisplayValue);

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
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    for (int z = 0; z <= 800; z++) {
                        OSD_writeCharRow1(s, _19, 0x14);
                        OSD_writeCharRow1(a, _20, 0x14);
                        OSD_writeCharRow1(v, _21, 0x14);
                        OSD_writeCharRow1(i, _22, 0x14);
                        OSD_writeCharRow1(n, _23, 0x14);
                        OSD_writeCharRow1(g, _24, 0x14);
                    }
                    break;
                case IRKeyExit:
                    exitMenu();
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

static bool IR_handleInfoDisplay()
{
    if (oled_menuItem != OLED_Info_Display) {
        return false;
    }

    if (oledClearFlag) {
        display.clear();
    }
    oledClearFlag = ~0;
    display.setColor(OLEDDISPLAY_COLOR::WHITE);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(1, 0, "Menu-");
    display.drawString(1, 28, "Info");
    display.display();

    boolean vsyncActive = 0;
    boolean hsyncActive = 0;
    float ofr = getOutputFrameRate();
    uint8_t currentInput = GBS::ADC_INPUT_SEL::read();
    rto->presetID = GBS::GBS_PRESET_ID::read();

    currentColor = yellow;
    currentRow = ROW_1;
    OSD_writeString(0, "Info:");
    currentColor = main0;
    OSD_writeString(26, "Hz");

    if (rto->presetID == 0x01 || rto->presetID == 0x11) {
        OSD_writeCharRow1(n1, P6, main0);
        OSD_writeCharRow1(n2, P7, main0);
        OSD_writeCharRow1(n8, P8, main0);
        OSD_writeCharRow1(n0, P9, main0);
        OSD_writeCharRow1(x, P10, main0);
        OSD_writeCharRow1(n9, P11, main0);
        OSD_writeCharRow1(n6, P12, main0);
        OSD_writeCharRow1(n0, P13, main0);
        OSD_writeCharRow1(n4, P14, blue_fill);
    } else if (rto->presetID == 0x02 || rto->presetID == 0x12) {
        OSD_writeCharRow1(n1, P6, main0);
        OSD_writeCharRow1(n2, P7, main0);
        OSD_writeCharRow1(n8, P8, main0);
        OSD_writeCharRow1(n0, P9, main0);
        OSD_writeCharRow1(x, P10, main0);
        OSD_writeCharRow1(n1, P11, main0);
        OSD_writeCharRow1(n0, P12, main0);
        OSD_writeCharRow1(n2, P13, main0);
        OSD_writeCharRow1(n4, P14, main0);
    } else if (rto->presetID == 0x03 || rto->presetID == 0x13) {
        OSD_writeCharRow1(n1, P6, main0);
        OSD_writeCharRow1(n2, P7, main0);
        OSD_writeCharRow1(n8, P8, main0);
        OSD_writeCharRow1(n0, P9, main0);
        OSD_writeCharRow1(x, P10, main0);
        OSD_writeCharRow1(n7, P11, main0);
        OSD_writeCharRow1(n2, P12, main0);
        OSD_writeCharRow1(n0, P13, main0);
        OSD_writeCharRow1(n4, P14, blue_fill);
    } else if (rto->presetID == 0x05 || rto->presetID == 0x15) {
        OSD_writeCharRow1(n1, P6, main0);
        OSD_writeCharRow1(n9, P7, main0);
        OSD_writeCharRow1(n2, P8, main0);
        OSD_writeCharRow1(n0, P9, main0);
        OSD_writeCharRow1(x, P10, main0);
        OSD_writeCharRow1(n1, P11, main0);
        OSD_writeCharRow1(n0, P12, main0);
        OSD_writeCharRow1(n8, P13, main0);
        OSD_writeCharRow1(n0, P14, main0);
    } else if (rto->presetID == 0x06 || rto->presetID == 0x16) {
        OSD_writeCharRow1(D, P6, main0);
        OSD_writeCharRow1(o, P7, main0);
        OSD_writeCharRow1(w, P8, main0);
        OSD_writeCharRow1(n, P9, main0);
        OSD_writeCharRow1(s, P10, main0);
        OSD_writeCharRow1(c, P11, main0);
        OSD_writeCharRow1(a, P12, main0);
        OSD_writeCharRow1(l, P13, main0);
        OSD_writeCharRow1(e, P14, main0);
    } else if (rto->presetID == 0x04) {
        OSD_writeCharRow1(n7, P6, main0);
        OSD_writeCharRow1(n2, P7, main0);
        OSD_writeCharRow1(n0, P8, main0);
        OSD_writeCharRow1(x, P9, main0);
        OSD_writeCharRow1(n4, P10, main0);
        OSD_writeCharRow1(n8, P11, main0);
        OSD_writeCharRow1(n0, P12, main0);
        OSD_writeCharRow1(n8, P13, blue_fill);
        OSD_writeCharRow1(n0, P14, blue_fill);
    } else if (rto->presetID == 0x14) {
        OSD_writeCharRow1(n7, P6, main0);
        OSD_writeCharRow1(n6, P7, main0);
        OSD_writeCharRow1(n8, P8, main0);
        OSD_writeCharRow1(x, P9, main0);
        OSD_writeCharRow1(n5, P10, main0);
        OSD_writeCharRow1(n7, P11, main0);
        OSD_writeCharRow1(n6, P12, main0);
        OSD_writeCharRow1(n8, P13, blue_fill);
        OSD_writeCharRow1(n0, P14, blue_fill);
    } else {
        OSD_writeCharRow1(B, P6, main0);
        OSD_writeCharRow1(y, P7, main0);
        OSD_writeCharRow1(p, P8, main0);
        OSD_writeCharRow1(a, P9, main0);
        OSD_writeCharRow1(s, P10, main0);
        OSD_writeCharRow1(s, P11, main0);
        OSD_writeCharRow1(n6, P12, blue_fill);
        OSD_writeCharRow1(n8, P13, blue_fill);
        OSD_writeCharRow1(n0, P14, blue_fill);
    }

    if (inputType == InputTypeRGBs) {
        OSD_writeCharRow1(r, P17, blue_fill);
        OSD_writeCharRow1(R, P18, main0);
        OSD_writeCharRow1(G, P19, main0);
        OSD_writeCharRow1(B, P20, main0);
        OSD_writeCharRow1(s, P21, main0);
    } else if (inputType == InputTypeRGsB) {
        OSD_writeCharRow1(r, P17, blue_fill);
        OSD_writeCharRow1(R, P18, main0);
        OSD_writeCharRow1(G, P19, main0);
        OSD_writeCharRow1(s, P20, main0);
        OSD_writeCharRow1(B, P21, main0);
        OSD_writeCharRow1(B, P22, blue_fill);
    } else if (inputType == InputTypeVGA) {
        OSD_writeCharRow1(r, P17, blue_fill);
        OSD_writeCharRow1(V, P18, main0);
        OSD_writeCharRow1(G, P19, main0);
        OSD_writeCharRow1(A, P20, main0);
        OSD_writeCharRow1(B, P21, blue_fill);
        OSD_writeCharRow1(B, P22, blue_fill);
    } else if (inputType == InputTypeYUV) {
        OSD_writeCharRow1(r, P17, blue_fill);
        OSD_writeCharRow1(Y, P18, main0);
        OSD_writeCharRow1(P, P19, main0);
        OSD_writeCharRow1(B, P20, main0);
        OSD_writeCharRow1(P, P21, main0);
        OSD_writeCharRow1(R, P22, main0);
    } else if (inputType == InputTypeSV) {
        OSD_writeCharRow1(r, P17, blue_fill);
        OSD_writeCharRow1(Y, P18, blue_fill);
        OSD_writeCharRow1(S, P19, main0);
        OSD_writeCharRow1(V, P20, main0);
        OSD_writeCharRow1(B, P21, blue_fill);
        OSD_writeCharRow1(B, P22, blue_fill);
    } else if (inputType == InputTypeAV) {
        OSD_writeCharRow1(r, P17, blue_fill);
        OSD_writeCharRow1(Y, P18, blue_fill);
        OSD_writeCharRow1(A, P19, main0);
        OSD_writeCharRow1(V, P20, main0);
        OSD_writeCharRow1(B, P21, blue_fill);
        OSD_writeCharRow1(B, P22, blue_fill);
    } else {
        OSD_writeCharRow1(Y, P17, blue_fill);
        OSD_writeCharRow1(P, P18, blue_fill);
        OSD_writeCharRow1(b, P19, blue_fill);
        OSD_writeCharRow1(P, P20, blue_fill);
        OSD_writeCharRow1(r, P21, blue_fill);
        OSD_writeCharRow1(B, P22, blue_fill);
    }

    osdDisplayValue = ofr;
    currentColor = main0;
    currentRow = ROW_1;
    digitPos1 = _25;
    digitPos2 = _24;
    digitPos3 = 0x3D;
    displayNumber3Digit(osdDisplayValue);

    clearRowContent(ROW_2, 31, 0);

    currentColor = yellow;
    currentRow = ROW_2;

    OSD_writeString(0, "Current:");

    currentColor = main0;
    currentRow = ROW_2;

    OSD_writeString(0xFF, " ");
    if ((rto->sourceDisconnected || !rto->boardHasPower || isInfoDisplayActive == 1)) {
        OSD_writeString(0xFF, "No Input");
    } else if (((currentInput == 1) || (inputType == InputTypeRGBs || inputType == InputTypeRGsB || inputType == InputTypeVGA))) {
        OSD_writeCharRow2(B, P16, blue_fill);
        OSD_writeString(0xFF, "RGB ");
        vsyncActive = GBS::STATUS_SYNC_PROC_VSACT::read();
        if (vsyncActive) {
            hsyncActive = GBS::STATUS_SYNC_PROC_HSACT::read();
            if (hsyncActive) {
                OSD_writeString(0xFF, "HV   ");
            }
        } else if ((inputType == InputTypeVGA) && ((!vsyncActive || !hsyncActive))) {
            OSD_writeCharRow2(B, P11, blue_fill);
            OSD_writeString(0x09, "No Input");
        }
    } else if ((rto->continousStableCounter > 35 || currentInput != 1) || (inputType == InputTypeYUV || inputType == InputTypeSV || inputType == InputTypeAV)) {
        OSD_writeCharRow2(B, P16, blue_fill);
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
                selectedMenuLine = 1;
                OSD_handleCommand('0');
                oled_menuItem = OLED_Input;
                break;
            case IRKeyExit:
                if (isInfoDisplayActive) {
                    GBS::VDS_DIS_HB_ST::write(horizontalBlankStart);
                    GBS::VDS_DIS_HB_SP::write(horizontalBlankStop);
                    isInfoDisplayActive = 0;
                }
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
        OSD_writeCharRow2((secondsRemaining / 10) + '0', P11, main0);
        OSD_writeCharRow2((secondsRemaining % 10) + '0', P12, main0);
        OSD_writeString(14, " s ");
    } else {
        OSD_writeCharRow2('0', P12, blue_fill);
        OSD_writeCharRow2(secondsRemaining + '0', P11, main0);
        OSD_writeString(13, " s ");
    }

    // Countdown expired - apply resolution
    if ((lastResolutionTime - resolutionStartTime) >= OSD_RESOLUTION_CLOSE_TIME) {
        userCommand = IR_getResolutionCommand(tentativeResolution);
        OSD_handleCommand(OSD_CROSS_MID);
        OSD_handleCommand('4');
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
    fillRowBackground(ROW_1, _9, blue_fill);

    // Display on OSD
    for (int i = 0; i <= 800; i++) {
        currentColor = yellowT;
        currentRow = ROW_1;
        writeChar(M, _1), writeChar(U, _2), writeChar(T, _3), writeChar(E, _4);
        currentColor = main0;
        if (muted) {
            writeChar(O, _6), writeChar(N, _7);
        } else {
            writeChar(O, _6), writeChar(F, _7), writeChar(F, _8);
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
    fillRowBackground(ROW_1, _9, blue_fill);
    OSD_Cut_0x01();
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
        fillRowBackground(ROW_1, _27, blue_fill);
        fillRowBackground(ROW_2, _27, blue_fill);
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
        OSD_handleCommand('0');
        oled_menuItem = OLED_Input;
        display.clear();
    }
}

// Handle Save key press - opens profile menu
static void IR_handleSaveKeyPress(void)
{
    lastMenuItemTime = millis();
    NEW_OLED_MENU = false;
    OSD_handleCommand(OSD_CROSS_TOP);
    OSD_handleCommand('w');
    oled_menuItem = OLED_Profile;
}

// Handle Info key press - opens info display
static void IR_handleInfoKeyPress(void)
{
    lastMenuItemTime = millis();
    NEW_OLED_MENU = false;
    fillRowBackground(ROW_1, _27, blue_fill);
    fillRowBackground(ROW_2, _27, blue_fill);
    oled_menuItem = OLED_Info_Display;
}

// Handle Volume keys - opens volume adjust menu
static void IR_handleVolumeKeyPress(void)
{
    lastMenuItemTime = millis();
    NEW_OLED_MENU = false;
    fillRowBackground(ROW_1, _25, blue_fill);
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
