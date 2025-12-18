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
// IR_handleMenuSelection - Menu State Machine
// ====================================================================================

void IR_handleMenuSelection(void)
{
    if (oled_menuItem == OLED_None) {
        NEW_OLED_MENU = true;
    } else {
        NEW_OLED_MENU = false;
    }

    if (oled_menuItem == OLED_OutputResolution) {
        if (oledClearFlag) {
            display.clear();
        }

        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->>>");
        display.drawString(1, 28, "Output Resolution");
        display.display();

        if (results.value == IRKeyUp || results.value == IRKeyDown) {
            selectedMenuLine = 2;
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('1');
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ScreenSettings) {
        if (oledClearFlag) {
            display.clear();
        }

        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.drawString(1, 0, "Menu->>>");
        display.drawString(1, 28, "Screen Settings");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, yellow);
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ColorSettings) {
        if (oledClearFlag) {
            display.clear();
        }

        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.drawString(1, 0, "Menu->>>");
        display.drawString(1, 28, "Picture Settings");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            selectedMenuLine = 2;
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('2');
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyUp:
                    selectedMenuLine = 1;
                    // OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    oled_menuItem = OLED_ResetSettings;
                    // OSD_handleCommand('2');

                    // oled_menuItem = OLED_ResetSettings;
                    // selectedMenuLine = 3;
                    // OSD_handleCommand(OSD_CROSS_BOTTOM);
                    // OSD_handleCommand('2');

                    break;
                case IRKeyOk:
                    // oled_menuItem = OLED_ColorSettings_ADCGain;
                    oled_menuItem = OLED_ColorSettings_RGB_R;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('d');
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings) {
        if (oledClearFlag) {
            display.clear();
        }

        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.drawString(1, 0, "Menu->>>");
        display.drawString(1, 28, "System Settings");
        display.display();

        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('2');
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyUp:
                    oled_menuItem = OLED_ScreenSettings;
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('1');
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    // OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyOk:
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('i');
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    // if (oled_menuItem == OLED_Developer)
    // {

    //     if(oledClearFlag)
    // display.clear();
    // oledClearFlag = ~0;display.setColor(OLEDDISPLAY_COLOR::WHITE);
    //     display.drawString(1, 0, "Menu->>>");
    //     display.drawString(1, 28, "Developer");
    //     display.display();

    //     if (results.value == IRKeyDown  ||  results.value == IRKeyUp)
    //     {
    //         OSD_c1(icon4, P0, blue_fill);
    //         OSD_c3(icon4, P0, blue_fill);
    //         OSD_c2(icon4, P0, yellow);
    //     }
    //     else if (results.value == IRKeyUp)
    //     {
    //         OSD_c1(icon4, P0, blue_fill);
    //         OSD_c3(icon4, P0, blue_fill);
    //         OSD_c2(icon4, P0, yellow);
    //     }

    //     if (irrecv.decode(&results))
    //     {
    //         switch (results.value)
    //         {
    //         case IRKeyUp:
    //             selectedMenuLine = 1;
    //             OSD_handleCommand('2');
    //             oled_menuItem = OLED_SystemSettings;
    //             break;
    //         case IRKeyDown:
    //             selectedMenuLine = 3;
    //             OSD_handleCommand('2');
    //             oled_menuItem = OLED_ResetSettings;
    //             break;
    //         case IRKeyOk:
    //             oled_menuItem = 104;
    //             OSD_handleCommand(OSD_CROSS_TOP);
    //             OSD_handleCommand('q');
    //             break;
    //         case IRKeyExit:
    //             oled_menuItem = OLED_None;
    //             OSD_clear();
    //             OSD();
    //             break;
    //         }
    //         irrecv.resume();
    //     }
    // }

    else if (oled_menuItem == OLED_ResetSettings) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.drawString(1, 0, "Menu->>>");
        display.drawString(1, 28, "Reset Settings");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyOk) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, yellow);
            OSD_handleCommand('2');
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;

                    // oled_menuItem = OLED_ColorSettings;
                    // selectedMenuLine = 2;
                    // OSD_handleCommand(OSD_CROSS_MID);
                    // OSD_handleCommand('2');

                    break;
                case IRKeyOk:
                    userCommand = '1';
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_OutputResolution_1080) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Output");
        display.drawString(1, 28, "1920x1080");
        display.display();

        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    // uopt->preferScalingRgbhv = true;
                    userCommand = 's';
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_OutputResolution_1024) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Output");
        display.drawString(1, 28, "1280x1024");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    // uopt->preferScalingRgbhv = true;
                    userCommand = 'p';
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_OutputResolution_960) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Output");
        display.drawString(1, 28, "1280x960");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c3(icon4, P0, yellow);
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_handleCommand('3');
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    // uopt->preferScalingRgbhv = true;
                    userCommand = 'f';
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_OutputResolution_720) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Output");
        display.drawString(1, 28, "1280x720");
        display.display();

        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('4');
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    // uopt->preferScalingRgbhv = true;
                    userCommand = 'g';
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_OutputResolution_480) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Output");
        display.drawString(1, 28, "480p/576p");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    // if (oled_menuItem == OLED_OutputResolution_Downscale)
    // {
    //     if(oledClearFlag)
    // display.clear();
    // oledClearFlag = ~0;display.setColor(OLEDDISPLAY_COLOR::WHITE);
    //     display.drawString(1, 0, "Menu->Output");
    //     display.drawString(1, 28, "Downscale");
    //     display.display();

    //     if (results.value == IRKeyDown  ||  results.value == IRKeyUp)
    //     {
    //         OSD_c3(icon4, P0, yellow);
    //         OSD_c1(icon4, P0, blue_fill);
    //         OSD_c2(icon4, P0, blue_fill);
    //         OSD_handleCommand('4');
    //     }

    //     if (irrecv.decode(&results))
    //     {
    //         switch (results.value)
    //         {
    //         case IRKeyMenu:
    //             OSD_handleCommand(OSD_CROSS_MID);
    //             OSD_handleCommand('1');
    //             oled_menuItem = OLED_OutputResolution;
    //             break;
    //         case IRKeyUp:
    //             selectedMenuLine = 2;
    //             OSD_handleCommand('4');
    //             oled_menuItem = OLED_OutputResolution_480;
    //             break;
    //         case IRKeyDown:
    //             oled_menuItem = OLED_OutputResolution_PassThrough;
    //             OSD_handleCommand(OSD_CROSS_TOP);
    //             OSD_handleCommand('5');
    //             break;
    //         case IRKeyOk:
    //             userCommand = 'L';
    //             break;
    //         case IRKeyExit:
    //             oled_menuItem = OLED_Input;
    //             OSD_clear();
    //             OSD();
    //             break;
    //         }
    //         irrecv.resume();
    //     }
    // }

    else if (oled_menuItem == OLED_OutputResolution_PassThrough) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Output");
        display.drawString(1, 28, "Pass Through");
        display.display();
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('4');
        }
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                // case IRKeyDown:
                //     oled_menuItem = OLED_OutputResolution_1080;
                //     OSD_handleCommand(OSD_CROSS_TOP);
                //     OSD_handleCommand('3');
                //     break;
                case IRKeyOk:
                    // tentativeResolution = uopt->presetPreference;
                    // if(inputType == InputTypeVGA)
                    // {
                    //     uopt->preferScalingRgbhv = false;
                    // }
                    // else
                    // {
                    //   // uopt->preferScalingRgbhv = true;
                    //   serialCommand = 'K';
                    // }

                    // saveUserPrefs();

                    // OSD_handleCommand(OSD_CROSS_TOP);
                    // OSD_handleCommand('!');
                    // oled_menuItem = OLED_RetainedSettings;
                    // keepSettings = 0;
                    // resolutionStartTime = millis();
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }
    
    else if (oled_menuItem == OLED_RetainedSettings) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Retained settings?");
        // display.drawString(1, 20, "settings?");
        display.display();
        // if (results.value == IRKeyOk)
        // {
        //   OSD_handleCommand('·');
        // }
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        // printf("Save \n");
                    } else {
                        if (tentativeResolution == Output960P) // 1280x960
                            userCommand = 'f';
                        else if (tentativeResolution == Output720P) // 1280x720
                            userCommand = 'g';
                        else if (tentativeResolution == Output480P) // 480p/576p
                            userCommand = 'h';
                        else if (tentativeResolution == Output1024P) // 1280x1024
                            userCommand = 'p';
                        else if (tentativeResolution == Output1080P) // 1920x1080
                            userCommand = 's';
                        else
                            userCommand = 'g';
                    }

                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('4');
                    oled_menuItem = OLED_OutputResolution_PassThrough;
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ScreenSettings_Move) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Screen");
        display.drawString(1, 28, "Move");
        display.display();

        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    OSD_c1(0x3E, P5, main0);
                    OSD_c1(0x3E, P6, main0);
                    OSD_c1(0x3E, P7, main0);
                    OSD_c1(0x3E, P8, main0);
                    OSD_c1(0x3E, P9, main0);
                    OSD_c1(0x3E, P10, main0);
                    OSD_c1(0x3E, P11, main0);
                    OSD_c1(0x3E, P12, main0);
                    OSD_c1(0x3E, P13, main0);
                    OSD_c1(0x08, P15, yellow);
                    OSD_c1(0x18, P16, yellow);
                    OSD_c1(0x03, P14, yellow);
                    OSD_c1(0x13, P17, yellow);
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ScreenSettings_Scale) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Screen");
        display.drawString(1, 28, "Scale");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    OSD_c2(0x3E, P6, main0);
                    OSD_c2(0x3E, P7, main0);
                    OSD_c2(0x3E, P8, main0);
                    OSD_c2(0x3E, P9, main0);
                    OSD_c2(0x3E, P10, main0);
                    OSD_c2(0x3E, P11, main0);
                    OSD_c2(0x3E, P12, main0);
                    OSD_c2(0x3E, P13, main0);
                    OSD_c2(0x08, P15, yellow);
                    OSD_c2(0x18, P16, yellow);
                    OSD_c2(0x03, P14, yellow);
                    OSD_c2(0x13, P17, yellow);
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ScreenSettings_Borders) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Screen");
        display.drawString(1, 28, "Borders");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c3(icon4, P0, yellow);
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    OSD_c3(0x3E, P8, main0);
                    OSD_c3(0x3E, P9, main0);
                    OSD_c3(0x3E, P10, main0);
                    OSD_c3(0x3E, P11, main0);
                    OSD_c3(0x3E, P12, main0);
                    OSD_c3(0x3E, P13, main0);
                    OSD_c3(0x08, P15, yellow);
                    OSD_c3(0x18, P16, yellow);
                    OSD_c3(0x03, P14, yellow);
                    OSD_c3(0x13, P17, yellow);
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ScreenSettings_MoveAdjust) {
        if (results.value == IRKeyOk) {
            OSD_handleCommand('6');
            OSD_c1(0x3E, P5, main0);
            OSD_c1(0x3E, P6, main0);
            OSD_c1(0x3E, P7, main0);
            OSD_c1(0x3E, P8, main0);
            OSD_c1(0x3E, P9, main0);
            OSD_c1(0x3E, P10, main0);
            OSD_c1(0x3E, P11, main0);
            OSD_c1(0x3E, P12, main0);
            OSD_c1(0x3E, P13, main0);
            OSD_c1(0x08, P15, yellow);
            OSD_c1(0x18, P16, yellow);
            OSD_c1(0x03, P14, yellow);
            OSD_c1(0x13, P17, yellow);
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                            colour1 = red;
                            number_stroca = stroca1;
                            __(l, _20);
                            __(i, _21);
                            __(m, _22);
                            __(i, _23);
                            __(t, _24);
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca1;
                    __(l, _20);
                    __(i, _21);
                    __(m, _22);
                    __(i, _23);
                    __(t, _24);
                    __(0x0d, _25);
                    break;
                case IRKeyLeft:
                    lastMenuItemTime = millis();
                    serialCommand = '7';
                    if (GBS::IF_HBIN_SP::read() < 0x150) {
                    } else {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = red;
                            number_stroca = stroca1;
                            __(l, _20);
                            __(i, _21);
                            __(m, _22);
                            __(i, _23);
                            __(t, _24);
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca1;
                    __(l, _20);
                    __(i, _21);
                    __(m, _22);
                    __(i, _23);
                    __(t, _24);
                    __(0x0d, _25);
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
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ScreenSettings_ScaleAdjust) {
        if (results.value == IRKeyOk) {
            OSD_handleCommand('6');
            OSD_c2(0x3E, P6, main0);
            OSD_c2(0x3E, P7, main0);
            OSD_c2(0x3E, P8, main0);
            OSD_c2(0x3E, P9, main0);
            OSD_c2(0x3E, P10, main0);
            OSD_c2(0x3E, P11, main0);
            OSD_c2(0x3E, P12, main0);
            OSD_c2(0x3E, P13, main0);
            OSD_c2(0x08, P15, yellow);
            OSD_c2(0x18, P16, yellow);
            OSD_c2(0x03, P14, yellow);
            OSD_c2(0x13, P17, yellow);
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                            colour1 = red;
                            number_stroca = stroca2;
                            OSD_writeString(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca2;
                    OSD_writeString(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyLeft:
                    lastMenuItemTime = millis();
                    serialCommand = 'z';
                    if (GBS::VDS_HSCALE::read() <= 256) {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = red;
                            number_stroca = stroca2;
                            OSD_writeString(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca2;
                    OSD_writeString(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyUp:
                    lastMenuItemTime = millis();
                    serialCommand = '5';
                    if (GBS::VDS_VSCALE::read() == 1023) {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = red;
                            number_stroca = stroca2;
                            OSD_writeString(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca2;
                    OSD_writeString(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyDown:
                    lastMenuItemTime = millis();
                    serialCommand = '4';
                    if (GBS::VDS_VSCALE::read() <= 256) {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = red;
                            number_stroca = stroca2;
                            OSD_writeString(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca2;
                    OSD_writeString(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ScreenSettings_BordersAdjust) {
        if (results.value == IRKeyOk) {
            OSD_handleCommand('6');
            OSD_c3(0x3E, P8, main0);
            OSD_c3(0x3E, P9, main0);
            OSD_c3(0x3E, P10, main0);
            OSD_c3(0x3E, P11, main0);
            OSD_c3(0x3E, P12, main0);
            OSD_c3(0x3E, P13, main0);
            OSD_c3(0x08, P15, yellow);
            OSD_c3(0x18, P16, yellow);
            OSD_c3(0x03, P14, yellow);
            OSD_c3(0x13, P17, yellow);
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                            colour1 = red;
                            number_stroca = stroca3;
                            OSD_writeString(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca3;
                    OSD_writeString(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyLeft:

                    userCommand = 'B';
                    if ((GBS::VDS_DIS_HB_ST::read() < (GBS::VDS_HSYNC_RST::read() - 4)) && (GBS::VDS_DIS_HB_SP::read() > 4)) {
                    } else {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = red;
                            number_stroca = stroca3;
                            OSD_writeString(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca3;
                    OSD_writeString(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyUp:

                    userCommand = 'C';
                    if ((GBS::VDS_DIS_VB_ST::read() > 6) && (GBS::VDS_DIS_VB_SP::read() < (GBS::VDS_VSYNC_RST::read() - 4))) {
                    } else {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = red;
                            number_stroca = stroca3;
                            OSD_writeString(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca3;
                    OSD_writeString(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyDown:

                    userCommand = 'D';
                    if ((GBS::VDS_DIS_VB_ST::read() < (GBS::VDS_VSYNC_RST::read() - 4)) && (GBS::VDS_DIS_VB_SP::read() > 6)) {
                    } else {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = red;
                            number_stroca = stroca3;
                            OSD_writeString(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca3;
                    OSD_writeString(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ColorSettings_ADCGain) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Color");
        display.drawString(1, 22, "ADC gain");
        // display.drawStringf(40,44,num,"ADC :%d");
        if (uopt->enableAutoGain) {
            display.drawString(1, 44, "ON");
        } else {
            display.drawString(1, 44, "OFF");
        }
        display.display();

        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('a');
        }

        OSD_handleCommand('e');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ColorSettings_Scanlines) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Color");
        display.drawString(1, 22, "Scanlines");

        // Check if scanlines are allowed for current signal
        boolean scanlinesAllowed = areScanLinesAllowed();

        // Always show ON/OFF, the OSD TV menu will show it grayed out if disabled
        if (uopt->wantScanlines) {
            display.drawString(1, 44, "ON");
        } else {
            display.drawString(1, 44, "OFF");
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
            OSD_c3(icon4, P0, blue_fill);
        }

        OSD_handleCommand('e');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('a'); //
                    oled_menuItem = OLED_ColorSettings_ADCGain;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('a'); //
                    oled_menuItem = OLED_ColorSettings_LineFilter;
                    break;
                case IRKeyRight:
                    // Only allow changing scanlines if they're supported
                    if (scanlinesAllowed) {
                        userCommand = 'K';
                    }
                    break;
                case IRKeyLeft:
                    // Only allow changing scanlines if they're supported
                    if (scanlinesAllowed) {
                        userCommand = 'K';
                    }
                    break;
                case IRKeyOk:
                    // Only allow toggling scanlines if they're supported
                    if (scanlinesAllowed) {
                        userCommand = '7';
                    }
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ColorSettings_LineFilter) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Color");
        display.drawString(1, 22, "Line filter");
        if (uopt->wantVdsLineFilter) {
            display.drawString(1, 44, "ON");
        } else {
            display.drawString(1, 44, "OFF");
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c3(icon4, P0, yellow);
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_handleCommand('a');
        }

        OSD_handleCommand('e');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('a'); //
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
                // case IRKeyLeft:
                //   userCommand = 'm';
                //   break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ColorSettings_Sharpness) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Color");
        display.drawString(1, 22, "Sharpness");
        if (GBS::VDS_PK_LB_GAIN::read() != 0x16) {
            display.drawString(1, 44, "ON");
        } else {
            display.drawString(1, 44, "OFF");
        }
        display.display();

        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('b');
        }

        OSD_handleCommand('f');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    OSD_handleCommand('b'); //
                    oled_menuItem = OLED_ColorSettings_Peaking;
                    break;
                case IRKeyOk:
                    userCommand = 'W';
                    break;
                // case IRKeyLeft:
                //   userCommand = 'W';
                //   break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ColorSettings_Peaking) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Color");
        display.drawString(1, 22, "Peaking");

        if (isPeakingLocked()) {
             display.drawString(1, 44, "LOCKED");
        } else {
            if (uopt->wantPeaking) {
                display.drawString(1, 44, "ON");
            } else {
                display.drawString(1, 44, "OFF");
            }
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('f');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('b'); //
                    oled_menuItem = OLED_ColorSettings_Sharpness;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('b'); //
                    oled_menuItem = OLED_ColorSettings_StepResponse;
                    break;
                case IRKeyOk:
                    if (!isPeakingLocked()) {
                        serialCommand = 'f';
                    }
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ColorSettings_StepResponse) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Color");
        display.drawString(1, 22, "Step response");
        if (uopt->wantStepResponse) {
            display.drawString(1, 44, "ON");
        } else {
            display.drawString(1, 44, "OFF");
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c3(icon4, P0, yellow);
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_handleCommand('b');
        }

        OSD_handleCommand('f');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('b'); //
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
                // case IRKeyLeft:
                //   serialCommand = 'V';
                //   break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ColorSettings_RGB_R) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Color");
        display.drawString(1, 28, "R ");
        display.display();

        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('d');
        }
        OSD_handleCommand('g');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                // case IRKeyUp:
                //     oled_menuItem = OLED_ColorSettings_StepResponse;
                //     OSD_handleCommand(OSD_CROSS_BOTTOM);
                //     OSD_handleCommand('b');
                //     break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('d'); //
                    oled_menuItem = OLED_ColorSettings_RGB_G;
                    break;
                case IRKeyRight:
                    // Y_offset +
                    R_VAL = MIN(R_VAL + STEP, 255);
                    // GBS::VDS_Y_OFST::write(cur);

                    applyRGBtoYUVConversion();
                    break;
                case IRKeyLeft:
                    // userCommand = 'T';
                    // Y_offset -
                    R_VAL = MAX(0, R_VAL - STEP);
                    // GBS::VDS_Y_OFST::write(cur);
                    applyRGBtoYUVConversion();
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    // serialCommand = 'K';
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ColorSettings_RGB_G) {
        if (uopt->enableAutoGain == 1) {
            uopt->enableAutoGain = 0;
            saveUserPrefs();
        } else {
            uopt->enableAutoGain = 0;
        }
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Color");
        display.drawString(1, 28, "G ");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }
        OSD_handleCommand('g');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('d'); //
                    oled_menuItem = OLED_ColorSettings_RGB_R;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('d'); //
                    oled_menuItem = OLED_ColorSettings_RGB_B;
                    break;
                case IRKeyRight:
                    // userCommand = 'N';
                    G_VAL = MIN(G_VAL + STEP, 255);

                    applyRGBtoYUVConversion();
                    break;
                case IRKeyLeft:
                    // userCommand = 'M';
                    G_VAL = MAX(0, G_VAL - STEP);
                    applyRGBtoYUVConversion();
                    break;

                case IRKeyOk:
                    saveUserPrefs();
                    break;

                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ColorSettings_RGB_B) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Color");
        display.drawString(1, 28, "B");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c3(icon4, P0, yellow);
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_handleCommand('d');
        }
        // (signed char)GBS::VDS_Y_OFST::read() = GBS::VDS_Y_OFST::read();
        // (signed char)GBS::VDS_U_OFST::read() = GBS::VDS_U_OFST::read();
        // (signed char)GBS::VDS_V_OFST::read() = GBS::VDS_V_OFST::read();
        OSD_handleCommand('g');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('d'); //
                    oled_menuItem = OLED_ColorSettings_RGB_G;
                    break;
                case IRKeyDown:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('a');
                    oled_menuItem = OLED_ColorSettings_ADCGain;
                    break;
                case IRKeyRight:
                    // userCommand = 'Q';
                    // cur = MIN(cur + STEP, 255);
                    // GBS::VDS_V_OFST::write(cur);

                    B_VAL = MIN(B_VAL + STEP, 255);

                    applyRGBtoYUVConversion();
                    break;
                case IRKeyLeft:
                    // userCommand = 'H';
                    // cur = MAX(0, cur - STEP);
                    // GBS::VDS_V_OFST::write(cur);
                    B_VAL = MAX(0, B_VAL - STEP);
                    applyRGBtoYUVConversion();
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ColorSettings_Y_Gain) {
        uint8_t cur = GBS::VDS_Y_GAIN::read();
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Color");
        display.drawString(1, 28, "Y gain");
        display.display();

        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('c');
        }

        OSD_handleCommand('h');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    OSD_handleCommand('c'); //
                    oled_menuItem = OLED_ColorSettings_Color;
                    break;
                case IRKeyRight:
                    // userCommand = 'P';
                    cur = MIN(cur + STEP, 255);
                    GBS::VDS_Y_GAIN::write(cur);
                    break;
                case IRKeyLeft:
                    // userCommand = 'S';
                    cur = MAX(0, cur - STEP);
                    GBS::VDS_Y_GAIN::write(cur);
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ColorSettings_Color) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Color");
        display.drawString(1, 28, "Color");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }
        OSD_handleCommand('h');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_MID);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('c'); //
                    oled_menuItem = OLED_ColorSettings_Y_Gain;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('c'); //
                    oled_menuItem = OLED_ColorSettings_DefaultColor;
                    break;
                case IRKeyRight:
                    userCommand = 'V'; //++
                    // curU = MIN(curU + STEP/8, 0X41);

                    // if (GBS::VDS_UCOS_GAIN::read() < 0x39)
                    // {
                    //     curV = MIN(curV + STEP/8, 0x46);
                    //     curU = curV - 13;
                    //     GBS::VDS_VCOS_GAIN::write(curV);
                    //     GBS::VDS_UCOS_GAIN::write(curU);
                    // }
                    break;
                case IRKeyLeft:
                    userCommand = 'R';
                    // curU = MAX(0X00, curU - STEP/8);
                    // curV = MAX(0x1c, curV - STEP/8);
                    // curU = curV - 13;
                    // GBS::VDS_VCOS_GAIN::write(curV);
                    // GBS::VDS_UCOS_GAIN::write(curU);
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ColorSettings_DefaultColor) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Color");
        display.drawString(1, 28, "Default Color");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('c');
        }

        // OSD_handleCommand('h');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                // case IRKeyDown:
                //   oled_menuItem = OLED_ColorSettings_ADCGain;
                //   OSD_handleCommand(OSD_CROSS_TOP);
                //   OSD_handleCommand('a');
                // break;
                case IRKeyOk:
                    userCommand = 'U';
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings_MatchedPresets) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->System");
        display.drawString(1, 22, "Matched presets");
        if (uopt->matchPresetSource) {
            display.drawString(1, 44, "ON");
        } else {
            display.drawString(1, 44, "OFF");
        }
        display.display();

        if (results.value == IRKeyUp || results.value == IRKeyDown) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, yellow);

            OSD_handleCommand('i');
        }

        OSD_handleCommand(j);

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                // case IRKeyLeft:
                //   serialCommand = 'Z';
                //   break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_ScreenSettings_FullHeight) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Screen");
        display.drawString(1, 22, "Full height");
        if (uopt->wantFullHeight) {
            display.drawString(1, 44, "ON");
        } else {
            display.drawString(1, 44, "OFF");
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_handleCommand('o');
        }
        OSD_handleCommand('p');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                // case IRKeyDown:
                //     selectedMenuLine = 3;
                //     OSD_handleCommand('i'); //
                //     oled_menuItem = OLED_SystemSettings_UseUpscaling;
                //     break;
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
                // case IRKeyRight:
                //   userCommand = 'v';
                //   break;
                // case IRKeyLeft:
                //   userCommand = 'v';
                //   break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings_UseUpscaling) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->System");
        display.drawString(1, 22, "Use upscaling");
        if (uopt->preferScalingRgbhv) {
            display.drawString(1, 44, "ON");
        } else {
            display.drawString(1, 44, "OFF");
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c3(icon4, P0, yellow);
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_handleCommand('i');
        }

        OSD_handleCommand('j');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    // userCommand = 'x';
                    // uopt->preferScalingRgbhv = !uopt->preferScalingRgbhv;
                    // applyVideoModePreset();
                    break;
                // case IRKeyLeft:
                //   userCommand = 'x';
                //   break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    // else if (oled_menuItem == OLED_SystemSettings_ComponentVGA)
    // {
    //     display.clear();
    //     display.drawString(1, 0, "Menu-");
    //     display.drawString(1, 28, "Component/VGA");
    //     display.display();

    //     if (results.value == IRKeyUp)
    //     {
    //         OSD_c1(icon4, P0, yellow);
    //         OSD_c2(icon4, P0, blue_fill);
    //         OSD_c3(icon4, P0, blue_fill);
    //         OSD_handleCommand('k');
    //     }

    //     OSD_handleCommand('l');

    //     if (irrecv.decode(&results))
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
    //             OSD_clear();
    //             OSD();
    //             break;
    //         }
    //         irrecv.resume();
    //     }
    // }

    else if (oled_menuItem == OLED_SystemSettings_Force5060Hz) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->System");
        display.drawString(1, 22, "Force 50 / 60Hz");
        if (uopt->PalForce60) {
            display.drawString(1, 44, "ON");
        } else {
            display.drawString(1, 44, "OFF");
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('l');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('k'); //
                    oled_menuItem = OLED_SystemSettings_Deinterlace;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('k'); //
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
                // case IRKeyLeft:
                //   userCommand = '0';
                //   break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings_ClockGenerator) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->System");
        display.drawString(1, 22, "Clock generator");
        if (uopt->disableExternalClockGenerator) {
            display.drawString(1, 44, "OFF");
        } else {
            display.drawString(1, 44, "ON");
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, yellow);
            OSD_handleCommand('m');
        }
        OSD_handleCommand('n');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('m'); //
                    oled_menuItem = OLED_SystemSettings_FrameTimeLock;
                    break;
                // case IRKeyDown:
                //   oled_menuItem = OLED_SystemSettings_ADCCalibration;
                //   OSD_handleCommand(OSD_CROSS_TOP);
                //   OSD_handleCommand('m');
                //   break;
                case IRKeyOk:
                    userCommand = 'X';
                    break;
                // case IRKeyLeft:
                //   userCommand = 'X';
                //   break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings_ADCCalibration) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->System");
        display.drawString(1, 22, "ADC calibration");
        if (uopt->enableCalibrationADC) {
            display.drawString(1, 44, "ON");
        } else {
            display.drawString(1, 44, "OFF");
        }
        display.display();

        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('m');
        }
        OSD_handleCommand('n');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    OSD_handleCommand('m'); //
                    oled_menuItem = OLED_SystemSettings_FrameTimeLock;
                    break;
                case IRKeyOk:
                    userCommand = 'w';
                    break;
                // case IRKeyLeft:
                //   userCommand = 'w';
                //   break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings_FrameTimeLock) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->System");
        display.drawString(1, 22, "Frame Time Lock");
        if (uopt->enableFrameTimeLock) {
            display.drawString(1, 44, "ON");
        } else {
            display.drawString(1, 44, "OFF");
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('n');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('m'); //
                    oled_menuItem = OLED_SystemSettings_ADCCalibration;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('m'); //
                    oled_menuItem = OLED_SystemSettings_ClockGenerator;
                    break;
                case IRKeyOk:
                    userCommand = '5';
                    break;
                // case IRKeyLeft:
                //   userCommand = '5';
                //   break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings_LockMethod) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->System");
        display.drawString(1, 22, "Lock Method");
        if (uopt->frameTimeLockMethod) {
            display.drawString(1, 44, "1Vtotal only");
        } else {
            display.drawString(1, 44, "0Vtotal+VSST");
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c3(icon4, P0, yellow);
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_handleCommand('k');
        }

        OSD_handleCommand('l');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('k'); //
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
                // case IRKeyLeft:
                //   userCommand = 'i';
                //   break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings_Deinterlace) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->System");
        display.drawString(1, 22, "Deinterlace");
        if (uopt->deintMode) {
            display.drawString(1, 44, "Bob");
        } else {
            display.drawString(1, 44, "Adaptive");
        }
        display.display();

        if (results.value == IRKeyUp || results.value == IRKeyDown) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('k');
        }

        OSD_handleCommand('l');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    OSD_handleCommand('k'); //
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
                // case IRKeyRight:
                //   userCommand = 'q';
                //   break;
                // case IRKeyLeft:
                //   userCommand = 'r';
                //   break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    // else if (oled_menuItem == OLED_MemoryAdjust)
    // {
    //   display.clear();
    //   display.drawString(1, 0, "Menu-");
    //   display.drawString(1, 28, "MEM left / righ");
    //   display.display();

    //   if (results.value == IRKeyUp)
    //   {
    //     OSD_c1(icon4, P0, yellow);
    //     OSD_c2(icon4, P0, blue_fill);
    //     OSD_c3(icon4, P0, blue_fill);
    //     OSD_handleCommand('q');
    //   }

    //   OSD_handleCommand('r');

    //   if (irrecv.decode(&results))
    //   {
    //     irDecodedFlag = 1;
    //     switch (results.value)
    //     {
    //     case IRKeyMenu:
    //       OSD_handleCommand(OSD_CROSS_MID);
    //       OSD_handleCommand('2');
    //       oled_menuItem = OLED_Developer;
    //       break;
    //     case IRKeyDown:
    //       selectedMenuLine = 2;
    //       OSD_handleCommand('q'); //
    //       oled_menuItem = 105;
    //       break;
    //     case IRKeyRight:
    //       serialCommand = '+';
    //       break;
    //     case IRKeyLeft:
    //       serialCommand = '-';
    //       break;
    //     case IRKeyExit:
    //       oled_menuItem = OLED_Input;
    //       OSD_clear();
    //       OSD();
    //       break;
    //     }
    //     irrecv.resume();
    //   }
    // }

    // else if (oled_menuItem == OLED_HSyncAdjust)
    // {

    //   display.clear();
    //   display.drawString(1, 0, "Menu-");
    //   display.drawString(1, 28, "HS left / right");
    //   display.display();

    //   if (results.value == IRKeyDown || results.value == IRKeyUp)
    //   {
    //     OSD_c1(icon4, P0, blue_fill);
    //     OSD_c3(icon4, P0, blue_fill);
    //     OSD_c2(icon4, P0, yellow);
    //   }

    //   OSD_handleCommand('r');

    //   if (irrecv.decode(&results))
    //   {
    //     irDecodedFlag = 1;
    //     switch (results.value)
    //     {
    //     case IRKeyMenu:
    //       OSD_handleCommand(OSD_CROSS_MID);
    //       OSD_handleCommand('2');
    //       oled_menuItem = OLED_Developer;
    //       break;
    //     case IRKeyUp:
    //       selectedMenuLine = 1;
    //       OSD_handleCommand('q'); //
    //       oled_menuItem = 104;
    //       break;
    //     case IRKeyDown:
    //       selectedMenuLine = 3;
    //       OSD_handleCommand('q'); //
    //       oled_menuItem = 106;
    //       break;
    //     case IRKeyRight:
    //       serialCommand = '0';
    //       break;
    //     case IRKeyLeft:
    //       serialCommand = '1';
    //       break;
    //     case IRKeyExit:
    //       oled_menuItem = OLED_Input;
    //       OSD_clear();
    //       OSD();
    //       break;
    //     }
    //     irrecv.resume();
    //   }
    // }

    // else if (oled_menuItem == OLED_HTotalAdjust)
    // {
    //   display.clear();
    //   display.drawString(1, 0, "Menu-");
    //   display.drawString(1, 28, "HTotal - / +");
    //   display.display();

    //   if (results.value == IRKeyDown || results.value == IRKeyUp)
    //   {
    //     OSD_c3(icon4, P0, yellow);
    //     OSD_c1(icon4, P0, blue_fill);
    //     OSD_c2(icon4, P0, blue_fill);
    //     OSD_handleCommand('q');
    //   }

    //   OSD_handleCommand('r');

    //   if (irrecv.decode(&results))
    //   {
    //     irDecodedFlag = 1;
    //     switch (results.value)
    //     {
    //     case IRKeyMenu:
    //       OSD_handleCommand(OSD_CROSS_MID);
    //       OSD_handleCommand('2');
    //       oled_menuItem = OLED_Developer;
    //       break;
    //     case IRKeyUp:
    //       selectedMenuLine = 2;
    //       OSD_handleCommand('q'); //
    //       oled_menuItem = 105;
    //       break;
    //     case IRKeyDown:
    //       oled_menuItem = 107;
    //       OSD_handleCommand(OSD_CROSS_TOP);
    //       OSD_handleCommand('s');
    //       break;
    //     case IRKeyRight:
    //       serialCommand = 'a';
    //       break;
    //     case IRKeyLeft:
    //       serialCommand = 'A';
    //       break;
    //     case IRKeyExit:
    //       oled_menuItem = OLED_Input;
    //       OSD_clear();
    //       OSD();
    //       break;
    //     }
    //     irrecv.resume();
    //   }
    // }

    // else if (oled_menuItem == OLED_DebugView)
    // {
    //   display.clear();
    //   display.drawString(1, 0, "Menu-");
    //   display.drawString(1, 28, "Debug view");
    //   display.display();

    //   if (results.value == IRKeyUp)
    //   {
    //     OSD_c1(icon4, P0, yellow);
    //     OSD_c2(icon4, P0, blue_fill);
    //     OSD_c3(icon4, P0, blue_fill);
    //     OSD_handleCommand('s');
    //   }

    //   OSD_handleCommand('t');

    //   if (irrecv.decode(&results))
    //   {
    //     irDecodedFlag = 1;
    //     switch (results.value)
    //     {
    //     case IRKeyMenu:
    //       OSD_handleCommand(OSD_CROSS_MID);
    //       OSD_handleCommand('2');
    //       oled_menuItem = OLED_Developer;
    //       break;
    //     case IRKeyUp:
    //       oled_menuItem = 106;
    //       OSD_handleCommand(OSD_CROSS_BOTTOM);
    //       OSD_handleCommand('q');
    //       break;
    //     case IRKeyDown:
    //       selectedMenuLine = 2;
    //       OSD_handleCommand('s'); //
    //       oled_menuItem = 108;
    //       break;
    //     case IRKeyRight:
    //       serialCommand = 'D';
    //       break;
    //     case IRKeyLeft:
    //       serialCommand = 'D';
    //       break;
    //     case IRKeyExit:
    //       oled_menuItem = OLED_Input;
    //       OSD_clear();
    //       OSD();
    //       break;
    //     }
    //     irrecv.resume();
    //   }
    // }

    // else if (oled_menuItem == OLED_ADCFilter)
    // {
    //   display.clear();
    //   display.drawString(1, 0, "Menu-");
    //   display.drawString(1, 28, "ADC filter");
    //   display.display();

    //   if (results.value == IRKeyDown || results.value == IRKeyUp)
    //   {
    //     OSD_c1(icon4, P0, blue_fill);
    //     OSD_c3(icon4, P0, blue_fill);
    //     OSD_c2(icon4, P0, yellow);
    //   }

    //   OSD_handleCommand('t');

    //   if (irrecv.decode(&results))
    //   {
    //     irDecodedFlag = 1;
    //     switch (results.value)
    //     {
    //     case IRKeyMenu:
    //       OSD_handleCommand(OSD_CROSS_MID);
    //       OSD_handleCommand('2');
    //       oled_menuItem = OLED_Developer;
    //       break;
    //     case IRKeyUp:
    //       selectedMenuLine = 1;
    //       OSD_handleCommand('s'); //
    //       oled_menuItem = 107;
    //       break;
    //     case IRKeyDown:
    //       selectedMenuLine = 3;
    //       OSD_handleCommand('s');
    //       oled_menuItem = 153;
    //       break;
    //     case IRKeyRight:
    //       serialCommand = 'F';
    //       break;
    //     case IRKeyLeft:
    //       serialCommand = 'F';
    //       break;
    //     case IRKeyExit:
    //       oled_menuItem = OLED_Input;
    //       OSD_clear();
    //       OSD();
    //       break;
    //     }
    //     irrecv.resume();
    //   }
    // }

    // else if (oled_menuItem == OLED_FreezeCapture)
    // {
    //   display.clear();
    //   display.drawString(1, 0, "Menu-");
    //   display.drawString(1, 28, "Freeze capture");
    //   display.display();

    //   if (results.value == IRKeyDown || results.value == IRKeyUp)
    //   {
    //     OSD_c3(icon4, P0, yellow);
    //     OSD_c1(icon4, P0, blue_fill);
    //     OSD_c2(icon4, P0, blue_fill);
    //     OSD_handleCommand('s'); //
    //   }

    //   OSD_handleCommand('t');

    //   if (irrecv.decode(&results))
    //   {
    //     irDecodedFlag = 1;
    //     switch (results.value)
    //     {
    //     case IRKeyMenu:
    //       OSD_handleCommand(OSD_CROSS_MID);
    //       OSD_handleCommand('2');
    //       oled_menuItem = OLED_Developer;
    //       break;
    //     case IRKeyUp:
    //       selectedMenuLine = 2;
    //       OSD_handleCommand('s'); //
    //       oled_menuItem = 108;
    //       break;
    //     case IRKeyDown:
    //       oled_menuItem = 104;
    //       OSD_handleCommand(OSD_CROSS_TOP);
    //       OSD_handleCommand('q');
    //       break;
    //     case IRKeyRight:
    //       userCommand = 'F';
    //       break;
    //     case IRKeyLeft:
    //       userCommand = 'F';
    //       break;
    //     case IRKeyExit:
    //       oled_menuItem = OLED_Input;
    //       OSD_clear();
    //       OSD();
    //       break;
    //     }
    //     irrecv.resume();
    //   }
    // }

    // else if (oled_menuItem == OLED_EnableOTA)
    // {
    //   display.clear();
    //   display.drawString(1, 0, "Menu-");
    //   display.drawString(1, 28, "Enable OTA");
    //   display.display();

    //   if (results.value == IRKeyUp)
    //   {
    //     OSD_c1(icon4, P0, yellow);
    //     OSD_c2(icon4, P0, blue_fill);
    //     OSD_c3(icon4, P0, blue_fill);
    //     OSD_handleCommand('u');
    //   }

    //   OSD_handleCommand('v');

    //   if (irrecv.decode(&results))
    //   {
    //     irDecodedFlag = 1;
    //     switch (results.value)
    //     {
    //     case IRKeyMenu:
    //       OSD_handleCommand(OSD_CROSS_BOTTOM);
    //       OSD_handleCommand('2');
    //       oled_menuItem = OLED_ResetSettings;
    //       break;
    //     case IRKeyDown:
    //       selectedMenuLine = 2;
    //       OSD_handleCommand('u'); //
    //       oled_menuItem = OLED_Restart;
    //       break;
    //     case IRKeyRight:
    //       serialCommand = 'c';
    //       break;
    //     case IRKeyLeft:
    //       serialCommand = 'c';
    //       break;
    //     case IRKeyExit:
    //       oled_menuItem = OLED_Input;
    //       OSD_clear();
    //       OSD();
    //       break;
    //     }
    //     irrecv.resume();
    //   }
    // }

    else if (oled_menuItem == OLED_Restart) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.drawString(1, 0, "Menu-");
        display.drawString(1, 28, "Restart");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('v');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_ResetSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('u'); //
                    oled_menuItem = OLED_EnableOTA;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('u'); //
                    oled_menuItem = OLED_ResetDefaults;
                    break;
                case IRKeyOk:
                    userCommand = 'a';
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    // else if (oled_menuItem == OLED_ResetDefaults)
    // {
    //     display.clear();
    //     display.drawString(1, 0, "Menu-");
    //     display.drawString(1, 28, "Reset defaults");
    //     display.display();

    //     if (results.value == IRKeyDown  ||  results.value == IRKeyUp)
    //     {
    //         OSD_c3(icon4, P0, yellow);
    //         OSD_c1(icon4, P0, blue_fill);
    //         OSD_c2(icon4, P0, blue_fill);
    //     }

    //     OSD_handleCommand('v');

    //     if (irrecv.decode(&results))
    //     {
    //         switch (results.value)
    //         {
    //         case IRKeyMenu:
    //             OSD_handleCommand(OSD_CROSS_BOTTOM);
    //             OSD_handleCommand('2');
    //             oled_menuItem = OLED_ResetSettings;
    //             break;
    //         case IRKeyUp:
    //             selectedMenuLine = 2;
    //             OSD_handleCommand('u'); //
    //             oled_menuItem = OLED_Restart;
    //             break;
    //         case IRKeyOk:
    //             userCommand = '1';
    //             break;
    //         case IRKeyExit:
    //             oled_menuItem = OLED_None;
    //             OSD_clear();
    //             OSD();
    //             break;
    //         }
    //         irrecv.resume();
    //     }
    // }

    else if (oled_menuItem == OLED_Input) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->>>");
        display.drawString(1, 28, "Input");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            selectedMenuLine = 1;
            OSD_c1(icon4, P0, yellow);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_handleCommand('1');
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None; // 154
                    OSD_clear();
                    OSD();
                    break;

                    // case IRKeyInfo:
                    //     OSD_clear();
                    //     OSD();
                    //     lastMenuItemTime = millis();
                    //     NEW_OLED_MENU = false;
                    //     background_up(stroca1, _27, blue_fill);
                    //     background_up(stroca2, _27, blue_fill);
                    //     oled_menuItem = OLED_Info_Display;
                    //     break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Input_RGBs) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Input");
        display.drawString(1, 28, "RGBs");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('@');
        }
        // OSD_handleCommand('*');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    OSD_handleCommand('1'); //
                    oled_menuItem = OLED_Input;
                    break;
                // case IRKeyUp:
                //     selectedMenuLine = 3;
                //     OSD_handleCommand(OSD_CROSS_BOTTOM);
                //     OSD_handleCommand('#');
                //     oled_menuItem = OLED_Input_AV;
                //     break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('@');
                    oled_menuItem = OLED_Input_RGsB;
                    break;

                    // case IRKeyLeft:
                    //     RGBs_Com = !RGBs_Com;
                    //     RGBs_CompatibilityChanged = 1;
                    //     break;
                    // case IRKeyRight:
                    //     RGBs_Com = !RGBs_Com;
                    //     RGBs_CompatibilityChanged = 1;
                    //     break;

                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Input_RGsB) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Input");
        display.drawString(1, 28, "RGsB");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('@');
        }
        // OSD_handleCommand('*');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    rgbComponentMode = 1;
                    InputRGsB_mode(rgbComponentMode);
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1'); //
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

                    // case IRKeyLeft:
                    //     RGsB_Com = !RGsB_Com;
                    //     RGsB_CompatibilityChanged = 1;
                    //     break;
                    // case IRKeyRight:
                    //     RGsB_Com = !RGsB_Com;
                    //     RGsB_CompatibilityChanged = 1;
                    //     break;

                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }
    
    else if (oled_menuItem == OLED_Input_VGA) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Input");
        display.drawString(1, 28, "VGA");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, yellow);

            OSD_handleCommand('@');
        }
        // OSD_handleCommand('*');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    rgbComponentMode = 0;
                    InputVGA_mode(rgbComponentMode);
                    // printf("\n\n rgbComponentMode %d \n\n", rgbComponentMode);
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1'); //
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

                    // case IRKeyLeft:
                    //     VGA_Com = !VGA_Com;
                    //     VGA_CompatibilityChanged = 1;
                    //     break;
                    // case IRKeyRight:
                    //     VGA_Com = !VGA_Com;
                    //     VGA_CompatibilityChanged = 1;
                    //     break;

                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }
    
    else if (oled_menuItem == OLED_Input_YPBPR) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Input");
        display.drawString(1, 28, "YPBPR");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {

            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('#');
        }
        OSD_handleCommand('$');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {

                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    InputYUV();
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1'); //
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
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }
    
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
        // display.drawString(1, 45, "Format:");
        switch (SVModeOption) {
            case 0: {
                display.drawString(1, 44, "Auto");
            } break;
            case 1: {
                display.drawString(1, 44, "PAL");
            } break;
            case 2: {
                display.drawString(1, 44, "NTSC-M");
            } break;
            case 3: {
                display.drawString(1, 44, "PAL-60");
            } break;
            case 4: {
                display.drawString(1, 44, "NTSC443");
            } break;
            case 5: {
                display.drawString(1, 44, "NTSC-J");
            } break;
            case 6: {
                display.drawString(1, 44, "PAL-N w/ p");
            } break;
            case 7: {
                display.drawString(1, 44, "PAL-M w/o p");
            } break;
            case 8: {
                display.drawString(1, 44, "PAL-M");
            } break;
            case 9: {
                display.drawString(1, 44, "PAL Cmb -N");
            } break;
            case 10: {
                display.drawString(1, 44, "PAL Cmb -N w/ p");
            } break;
            case 11: {
                display.drawString(1, 44, "SECAM");
            } break;
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('#');
        }
        OSD_handleCommand('$');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {

                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    InputSV_mode(SVModeOption + 1);
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1'); //
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
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }
    
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
        // display.drawString(1, 45, "Format:");
        switch (AVModeOption) {
            case 0: {
                display.drawString(1, 44, "Auto");
            } break;
            case 1: {
                display.drawString(1, 44, "PAL");
            } break;
            case 2: {
                display.drawString(1, 44, "NTSC-M");
            } break;
            case 3: {
                display.drawString(1, 44, "PAL-60");
            } break;
            case 4: {
                display.drawString(1, 44, "NTSC443");
            } break;
            case 5: {
                display.drawString(1, 44, "NTSC-J");
            } break;
            case 6: {
                display.drawString(1, 44, "PAL-N w/ p");
            } break;
            case 7: {
                display.drawString(1, 44, "PAL-M w/o p");
            } break;
            case 8: {
                display.drawString(1, 44, "PAL-M");
            } break;
            case 9: {
                display.drawString(1, 44, "PAL Cmb -N");
            } break;
            case 10: {
                display.drawString(1, 44, "PAL Cmb -N w/ p");
            } break;
            case 11: {
                display.drawString(1, 44, "SECAM");
            } break;
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {

            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, yellow);

            OSD_handleCommand('#');
        }
        OSD_handleCommand('$');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyOk:
                    isInfoDisplayActive = 0;
                    InputAV_mode(AVModeOption + 1);
                    // rto->isInLowPowerMode = false;
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1'); //
                    oled_menuItem = OLED_Input;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('#');
                    oled_menuItem = OLED_Input_SV;
                    break;
                // case IRKeyDown:
                //     oled_menuItem = OLED_Input_RGBs;
                //     selectedMenuLine = 1;
                //     OSD_handleCommand(OSD_CROSS_TOP);
                //     OSD_handleCommand('@');
                //     break;
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
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings_SVAVInputSettings) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->System");
        display.drawString(1, 28, "Sv-Av InputSet");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);

            OSD_handleCommand('i');
        }

        OSD_handleCommand('j');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                // case IRKeyUp:
                //     selectedMenuLine = 2;
                //     OSD_handleCommand(OSD_CROSS_MID);
                //     OSD_handleCommand('o');
                //     oled_menuItem = OLED_SystemSettings_MatchedPresets;
                //     break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('i');
                    oled_menuItem = OLED_SystemSettings_Compatibility;
                    break;

                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_DoubleLine) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "M>Sys>SvAv Set");
        display.drawString(1, 22, "DoubleLine");
        if (lineOption) {
            display.drawString(1, 44, "2X");
        } else {
            display.drawString(1, 44, "1X");
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);

            OSD_handleCommand('^');
        }
        OSD_handleCommand('&');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('i'); //
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;

                // case IRKeyUp:
                //   selectedMenuLine = 2;
                //   OSD_handleCommand('^');
                //   oled_menuItem = OLED_SystemSettings_SVAVInput_Smooth;
                //   break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_handleCommand('^');
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Smooth;
                    break;

                case IRKeyOk:
                    lineOption = !lineOption;
                    settingLineOptionChanged = 1;
                    break;
                // case IRKeyRight:
                //   lineOption = !lineOption;
                //   settingLineOptionChanged = 1;
                //   break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Smooth) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "M>Sys>SvAv Set");
        display.drawString(1, 22, "Smooth");
        if (smoothOption) {
            display.drawString(1, 44, "ON");
        } else {
            display.drawString(1, 44, "OFF");
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
            OSD_c3(icon4, P0, blue_fill);

            OSD_handleCommand('^');
        }
        OSD_handleCommand('&');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('i'); //
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
                // case IRKeyRight:
                //   smoothOption = !smoothOption;
                //   settingSmoothOptionChanged = 1;
                //   break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Bright) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "M>Sys>SvAv Set");
        display.drawString(1, 22, "Bright");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, yellow);

            OSD_handleCommand('^');
        }
        OSD_handleCommand('&');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('i'); //
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    saveUserPrefs();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_handleCommand('^');
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Smooth;
                    break;
                case IRKeyDown:

                    // selectedMenuLine = 3;
                    // OSD_handleCommand('^');
                    // oled_menuItem = OLED_SystemSettings_SVAVInput_Contrast;

                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('z');
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Contrast;
                    break;
                case IRKeyRight:
                    brightness = MIN(brightness + STEP, 254);
                    ADV_sendBCSH(0x0a, brightness - 128);
                    // printf("brightness: 0x%02x \n",brightness);
                    break;
                case IRKeyLeft:
                    brightness = MAX(brightness - STEP, 0);
                    ADV_sendBCSH(0x0a, brightness - 128);
                    // printf("brightness: 0x%02x \n",brightness);
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Contrast) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "M>Sys>SvAv Set");
        display.drawString(1, 22, "Contrast");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);

            OSD_handleCommand('z');
        }
        OSD_handleCommand('A');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('i'); //
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
                case IRKeyUp:
                    OSD_handleCommand(OSD_CROSS_BOTTOM);
                    OSD_handleCommand('^');
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Bright;
                    // selectedMenuLine = 2;
                    // OSD_handleCommand('^');
                    // oled_menuItem = OLED_SystemSettings_SVAVInput_Bright;
                    break;

                case IRKeyDown:
                    // OSD_handleCommand(OSD_CROSS_TOP);
                    // OSD_handleCommand('z');
                    // oled_menuItem = OLED_SystemSettings_SVAVInput_Saturation;

                    selectedMenuLine = 2;
                    OSD_handleCommand('z');
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Saturation;

                    break;
                case IRKeyRight:
                    contrast = MIN(contrast + STEP, 254);
                    ADV_sendBCSH(0x08, contrast);
                    // printf("contrast: 0x%02x \n",contrast);
                    break;
                case IRKeyLeft:
                    contrast = MAX(contrast - STEP, 0);
                    ADV_sendBCSH(0x08, contrast);
                    // printf("contrast: 0x%02x \n",contrast);
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Saturation) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "M>Sys>SvAv Set");
        display.drawString(1, 22, "Saturation");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
            OSD_c3(icon4, P0, blue_fill);

            OSD_handleCommand('z');
        }
        OSD_handleCommand('A');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('i'); //
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('z');
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Contrast;

                    // OSD_handleCommand(OSD_CROSS_BOTTOM);
                    // OSD_handleCommand('^');
                    // oled_menuItem = OLED_SystemSettings_SVAVInput_Contrast;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('z');
                    oled_menuItem = OLED_SystemSettings_SVAVInput_Default;
                    break;
                case IRKeyRight:
                    saturation = MIN(saturation + STEP, 254);
                    ADV_sendBCSH(0xe3, saturation);
                    // printf("saturation: 0x%02x \n",saturation);
                    break;
                case IRKeyLeft:
                    saturation = MAX(saturation - STEP, 0);
                    ADV_sendBCSH(0xe3, saturation);
                    // printf("saturation: 0x%02x \n",saturation);
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings_SVAVInput_Default) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "M>Sys>SvAv Set");
        display.drawString(1, 22, "Default");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, yellow);

            OSD_handleCommand('z');
        }
        OSD_handleCommand('A');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('i'); //
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
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_SystemSettings_Compatibility) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->System");
        display.drawString(1, 22, "Compatibility");
        if (rgbComponentMode == 1) {
            display.drawString(1, 44, "ON");
        } else {
            display.drawString(1, 44, "OFF");
        }
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
            OSD_c3(icon4, P0, blue_fill);

            OSD_handleCommand('i');
        }
        OSD_handleCommand('j');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('2');
                    oled_menuItem = OLED_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_handleCommand('i'); //
                    oled_menuItem = OLED_SystemSettings_SVAVInputSettings;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_handleCommand('i'); //
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
                // case IRKeyRight:
                //   rgbComponentMode = !rgbComponentMode;
                //   if (rgbComponentMode > 1)
                //     rgbComponentMode = 0;
                //   ADV_sendCompatibility(rgbComponentMode);
                //   break;
                case IRKeyExit:
                    OSD_handleCommand(OSD_CROSS_TOP);
                    OSD_handleCommand('1');
                    oled_menuItem = OLED_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu-");
        display.drawString(1, 28, "Profile");
        display.display();

        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_SaveConfirm) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Save) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Load) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Operation1) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Operation2) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Operation3) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_SelectSlot) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot1) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot2) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot3) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot4) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot5) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot6) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot7) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot8) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot9) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot10) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot11) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot12) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot13) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot14) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot15) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot16) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot17) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot18) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Slot19) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_handleCommand('w');
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_SelectPreset) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Preset1) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Preset2) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Preset3) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Preset4) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Preset5) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Preset6) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Preset7) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Preset8) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Preset9) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Preset10) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Preset11) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Profile_Preset12) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_handleCommand('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                        OSD_c2(O, P25, 0x14);
                        OSD_c2(K, P26, 0x14);
                    }
                    OSD_c2(O, P25, blue_fill);
                    OSD_c2(K, P26, blue_fill);
                    break;
                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Volume_Adjust) {
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
        colour1 = yellowT;
        number_stroca = stroca1;
        OSD_writeString(1, "Line input volume");
        colour1 = main0;
        sequence_number1 = _21;
        sequence_number2 = _20;
        sequence_number3 = 0x3D;
        // __(d, _23), __(B, _24);
        OSD_c1(o, _19, blue_fill);

        OSD_c1(o, _22, blue_fill);
        OSD_c1(o, _23, blue_fill);
        OSD_c1(o, _24, blue_fill);
        Typ(osdDisplayValue);
        // if (Volume <= 0)
        // {
        //   OSD_c1(o, _19, blue_fill);
        // }
        // else if (Volume > 0)
        // {
        //   __(0x3E, _19);
        // }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case kRecv2: // ++
                    volume = MAX(volume - 1, 0);
                    osdDisplayValue = 50 - volume;
                    PT_2257(volume); // 0-50 maps to 0-50 dB (actually 0-39 dB usable)
                    break;
                case kRecv3: // --
                    volume = MIN(volume + 1, 50);
                    osdDisplayValue = 50 - volume;
                    PT_2257(volume); // 0-50 maps to 0-50 dB (actually 0-39 dB usable)
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_handleCommand('0');
                    oled_menuItem = OLED_OutputResolution;
                    break;

                case IRKeyOk:
                    saveUserPrefs();
                    for (int z = 0; z <= 800; z++) {
                        OSD_c1(s, _19, 0x14);
                        OSD_c1(a, _20, 0x14);
                        OSD_c1(v, _21, 0x14);
                        OSD_c1(i, _22, 0x14);
                        OSD_c1(n, _23, 0x14);
                        OSD_c1(g, _24, 0x14);
                    }
                    break;

                case IRKeyExit:
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OLED_Info_Display) {
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

        colour1 = yellow;
        number_stroca = stroca1;
        OSD_writeString(0, "Info:");
        colour1 = main0;
        OSD_writeString(26, "Hz");

        if (rto->presetID == 0x01 || rto->presetID == 0x11) {
            // OSD_writeString(6,1,"1280x960 ");

            OSD_c1(n1, P6, main0);
            OSD_c1(n2, P7, main0);
            OSD_c1(n8, P8, main0);
            OSD_c1(n0, P9, main0);
            OSD_c1(x, P10, main0);
            OSD_c1(n9, P11, main0);
            OSD_c1(n6, P12, main0);
            OSD_c1(n0, P13, main0);
            OSD_c1(n4, P14, blue_fill);
        } else if (rto->presetID == 0x02 || rto->presetID == 0x12) {
            // OSD_writeString(6,1,"1280x1024");
            OSD_c1(n1, P6, main0);
            OSD_c1(n2, P7, main0);
            OSD_c1(n8, P8, main0);
            OSD_c1(n0, P9, main0);
            OSD_c1(x, P10, main0);
            OSD_c1(n1, P11, main0);
            OSD_c1(n0, P12, main0);
            OSD_c1(n2, P13, main0);
            OSD_c1(n4, P14, main0);
        } else if (rto->presetID == 0x03 || rto->presetID == 0x13) {
            // OSD_writeString(6,1,"1280x720 ");
            OSD_c1(n1, P6, main0);
            OSD_c1(n2, P7, main0);
            OSD_c1(n8, P8, main0);
            OSD_c1(n0, P9, main0);
            OSD_c1(x, P10, main0);
            OSD_c1(n7, P11, main0);
            OSD_c1(n2, P12, main0);
            OSD_c1(n0, P13, main0);
            OSD_c1(n4, P14, blue_fill);
        } else if (rto->presetID == 0x05 || rto->presetID == 0x15) {
            // OSD_writeString(6,1,"1920x1080");
            OSD_c1(n1, P6, main0);
            OSD_c1(n9, P7, main0);
            OSD_c1(n2, P8, main0);
            OSD_c1(n0, P9, main0);
            OSD_c1(x, P10, main0);
            OSD_c1(n1, P11, main0);
            OSD_c1(n0, P12, main0);
            OSD_c1(n8, P13, main0);
            OSD_c1(n0, P14, main0);
        } else if (rto->presetID == 0x06 || rto->presetID == 0x16) {
            // OSD_writeString(6,1,"Downscale");
            OSD_c1(D, P6, main0);
            OSD_c1(o, P7, main0);
            OSD_c1(w, P8, main0);
            OSD_c1(n, P9, main0);
            OSD_c1(s, P10, main0);
            OSD_c1(c, P11, main0);
            OSD_c1(a, P12, main0);
            OSD_c1(l, P13, main0);
            OSD_c1(e, P14, main0);
        } else if (rto->presetID == 0x04) {
            // OSD_writeString(6,1,"720x480  ");
            OSD_c1(n7, P6, main0);
            OSD_c1(n2, P7, main0);
            OSD_c1(n0, P8, main0);
            OSD_c1(x, P9, main0);
            OSD_c1(n4, P10, main0);
            OSD_c1(n8, P11, main0);
            OSD_c1(n0, P12, main0);
            OSD_c1(n8, P13, blue_fill);
            OSD_c1(n0, P14, blue_fill);
        } else if (rto->presetID == 0x14) {
            // OSD_writeString(6,1,"768x576  ");
            OSD_c1(n7, P6, main0);
            OSD_c1(n6, P7, main0);
            OSD_c1(n8, P8, main0);
            OSD_c1(x, P9, main0);
            OSD_c1(n5, P10, main0);
            OSD_c1(n7, P11, main0);
            OSD_c1(n6, P12, main0);
            OSD_c1(n8, P13, blue_fill);
            OSD_c1(n0, P14, blue_fill);
        } else {
            // OSD_writeString(6,1,"Bypass   ");
            OSD_c1(B, P6, main0);
            OSD_c1(y, P7, main0);
            OSD_c1(p, P8, main0);
            OSD_c1(a, P9, main0);
            OSD_c1(s, P10, main0);
            OSD_c1(s, P11, main0);
            OSD_c1(n6, P12, blue_fill);
            OSD_c1(n8, P13, blue_fill);
            OSD_c1(n0, P14, blue_fill);
        }

        if (inputType == InputTypeRGBs) {
            // OSD_writeString(17,1," RGBs");
            OSD_c1(r, P17, blue_fill);
            OSD_c1(R, P18, main0);
            OSD_c1(G, P19, main0);
            OSD_c1(B, P20, main0);
            OSD_c1(s, P21, main0);
        } else if (inputType == InputTypeRGsB) {
            // OSD_writeString(17,1," RGsB ");
            OSD_c1(r, P17, blue_fill);
            OSD_c1(R, P18, main0);
            OSD_c1(G, P19, main0);
            OSD_c1(s, P20, main0);
            OSD_c1(B, P21, main0);
            OSD_c1(B, P22, blue_fill);
        } else if (inputType == InputTypeVGA) {
            // OSD_writeString(17,1," VGA  ");
            OSD_c1(r, P17, blue_fill);
            OSD_c1(V, P18, main0);
            OSD_c1(G, P19, main0);
            OSD_c1(A, P20, main0);
            OSD_c1(B, P21, blue_fill);
            OSD_c1(B, P22, blue_fill);
        } else if (inputType == InputTypeYUV) {
            OSD_c1(r, P17, blue_fill);
            OSD_c1(Y, P18, main0);
            OSD_c1(P, P19, main0);
            OSD_c1(B, P20, main0);
            OSD_c1(P, P21, main0);
            OSD_c1(R, P22, main0);
        } else if (inputType == InputTypeSV) {
            OSD_c1(r, P17, blue_fill);
            OSD_c1(Y, P18, blue_fill);
            OSD_c1(S, P19, main0);
            OSD_c1(V, P20, main0);
            OSD_c1(B, P21, blue_fill);
            OSD_c1(B, P22, blue_fill);
        } else if (inputType == InputTypeAV) {
            OSD_c1(r, P17, blue_fill);
            OSD_c1(Y, P18, blue_fill);
            OSD_c1(A, P19, main0);
            OSD_c1(V, P20, main0);
            OSD_c1(B, P21, blue_fill);
            OSD_c1(B, P22, blue_fill);
        } else {
            OSD_c1(Y, P17, blue_fill);
            OSD_c1(P, P18, blue_fill);
            OSD_c1(b, P19, blue_fill);
            OSD_c1(P, P20, blue_fill);
            OSD_c1(r, P21, blue_fill);
            OSD_c1(B, P22, blue_fill);
        }

        osdDisplayValue = ofr;
        colour1 = main0;
        number_stroca = stroca1;
        sequence_number1 = _25;
        sequence_number2 = _24; //_24
        sequence_number3 = 0x3D;
        Typ(osdDisplayValue);

        clean_up(stroca2, 31, 0); // 17  31

        colour1 = yellow;
        number_stroca = stroca2;

        OSD_writeString(0, "Current:");

        colour1 = main0;
        number_stroca = stroca2;

        OSD_writeString(0xFF, " ");
        if ((rto->sourceDisconnected || !rto->boardHasPower || isInfoDisplayActive == 1)) {
            OSD_writeString(0xFF, "No Input");
        } else if (((currentInput == 1) || (inputType == InputTypeRGBs || inputType == InputTypeRGsB || inputType == InputTypeVGA))) {
            OSD_c2(B, P16, blue_fill);
            OSD_writeString(0xFF, "RGB ");
            vsyncActive = GBS::STATUS_SYNC_PROC_VSACT::read();
            if (vsyncActive) {
                // OSD_writeString(0xFF,"H");
                hsyncActive = GBS::STATUS_SYNC_PROC_HSACT::read();
                if (hsyncActive) {
                    OSD_writeString(0xFF, "HV   ");
                }
            } else if ((inputType == InputTypeVGA) && ((!vsyncActive || !hsyncActive))) {
                OSD_c2(B, P11, blue_fill);
                OSD_writeString(0x09, "No Input");
            }
        } else if ((rto->continousStableCounter > 35 || currentInput != 1) || (inputType == InputTypeYUV || inputType == InputTypeSV || inputType == InputTypeAV)) {
            OSD_c2(B, P16, blue_fill);
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
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
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
                    oled_menuItem = OLED_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    if (
        (
            (results.value == IRKeyMenu) ||
            (results.value == IRKeySave) ||
            (results.value == IRKeyInfo) ||
            (results.value == IRKeyRight) ||
            (results.value == IRKeyLeft) ||
            (results.value == IRKeyUp) ||
            (results.value == IRKeyDown) ||
            (results.value == IRKeyOk) ||
            (results.value == IRKeyExit) ||
            (results.value == IRKeyMute) ||
            (results.value == kRecv2) ||
            (results.value == kRecv3)) &&
        (irDecodedFlag == 1) &&
        (oled_menuItem != 0)) {
        // printf("Delay success \n");
        lastMenuItemTime = millis();
        irDecodedFlag = 0;
        resetOLEDScreenSaverTimer();
    }

    if ((millis() - lastResolutionTime) >= OSD_RESOLUTION_UP_TIME && oled_menuItem == OLED_RetainedSettings) {
        lastMenuItemTime = millis(); // updata osd close
        lastResolutionTime = millis();
        uint8_t T_tim = OSD_RESOLUTION_CLOSE_TIME / 1000 - ((lastResolutionTime - resolutionStartTime) / 1000);
        // colour1 = A2_main0;
        number_stroca = stroca2;
        if (T_tim >= 10) {
            OSD_c2((T_tim / 10) + '0', P11, main0);
            OSD_c2((T_tim % 10) + '0', P12, main0);

            OSD_writeString(14, " s ");
        } else {
            OSD_c2('0', P12, blue_fill);
            OSD_c2(T_tim + '0', P11, main0); //

            OSD_writeString(13, " s ");
        }
        // printf(" TIM :%d \n",(uint8_t)((lastResolutionTime - resolutionStartTime)/10));

        if ((lastResolutionTime - resolutionStartTime) >= OSD_RESOLUTION_CLOSE_TIME) {

            // uopt->preferScalingRgbhv = true;
            if (tentativeResolution == Output960P) // 1280x960
                userCommand = 'f';
            else if (tentativeResolution == Output720P) // 1280x720
                userCommand = 'g';
            else if (tentativeResolution == Output480P) // 480p/576p
                userCommand = 'h';
            else if (tentativeResolution == Output1024P) // 1280x1024
                userCommand = 'p';
            else if (tentativeResolution == Output1080P) // 1920x1080
                userCommand = 's';
            else
                userCommand = 'g';
            // printf("%c \n",userCommand);

            OSD_handleCommand(OSD_CROSS_MID);
            OSD_handleCommand('4');
            oled_menuItem = OLED_OutputResolution_PassThrough;
        }
    }

    if (lastOledMenuItem != oled_menuItem && oled_menuItem != 0) {
        lastMenuItemTime = millis();
        oledClearFlag = 1;
        // printf("freq:%d \n", system_get_cpu_freq());
        // printf("oled_menuItem:%d \n", oled_menuItem);
    }
    if ((millis() - lastMenuItemTime) >= OSD_CLOSE_TIME && oled_menuItem != 0) {
        // 菜单关闭
        if (isInfoDisplayActive) {
            GBS::VDS_DIS_HB_ST::write(horizontalBlankStart);
            GBS::VDS_DIS_HB_SP::write(horizontalBlankStop);
            isInfoDisplayActive = 0;
        }
        oled_menuItem = OLED_None;
        lastOledMenuItem = 0;
        OSD_clear();
        OSD();
    }
    lastOledMenuItem = oled_menuItem;
}

// ====================================================================================
// IR_handleInput - IR Remote Input Handler
// ====================================================================================

void IR_handleInput()
{
    if (irrecv.decode(&results)) {
        irDecodedFlag = 1;
        if (results.value == IRKeyMenu) {
            lastMenuItemTime = millis();
            if (rto->sourceDisconnected || !rto->boardHasPower || GBS::PAD_CKIN_ENZ::read()) // || !GBS::STATUS_MISC_VSYNC::read()
            {

                NEW_OLED_MENU = false;
                background_up(stroca1, _27, blue_fill);
                background_up(stroca2, _27, blue_fill);

                oled_menuItem = OLED_Info_Display;

                // Save horizontal blank values before modifying
                isInfoDisplayActive = 1;
                horizontalBlankStart = GBS::VDS_DIS_HB_ST::read();
                horizontalBlankStop = GBS::VDS_DIS_HB_SP::read();

                // loadDefaultUserOptions();
                writeProgramArrayNew(ntsc_720x480, false);
                doPostPresetLoadSteps();
                GBS::VDS_DIS_HB_ST::write(0x00);
                GBS::VDS_DIS_HB_SP::write(0xffff);
                freezeVideo();
                GBS::SP_CLAMP_MANUAL::write(1);
                // GBS::VDS_U_OFST::write(GBS::VDS_U_OFST::read() + 100);
            } else {
                NEW_OLED_MENU = false;
                selectedMenuLine = 1;
                OSD_handleCommand('0');
                oled_menuItem = OLED_Input;
                display.clear();
                // display.init();
                // display.flipScreenVertically();
                // printf("Oled Init\n");
            }
        }

        // if (results.value == kRecv14)
        // {
        //     NEW_OLED_MENU = false;
        //     background_up(stroca1, _10, blue_fill);
        //     for (int i = 0; i <= 800; i++)
        //     {
        //         colour1 = yellowT;
        //         number_stroca = stroca1;
        //         __(R, _2), __(e, _3), __(s, _4), __(t, _5), __(a, _6), __(r, _7), __(t, _8);
        //         display.clear();
        //         display.setTextAlignment(TEXT_ALIGN_LEFT);
        //         display.setFont(ArialMT_Plain_16);
        //         display.drawString(8, 15, "Resetting GBS");
        //         display.drawString(8, 35, "Please Wait...");
        //         display.display();
        //     }
        //     webSocket.disconnect();
        //     delay(60);
        //     ESP.reset();
        //     oled_menuItem = OLED_None;
        //     PT_MUTE(0x79);
        // }

        if (results.value == IRKeySave) {
            lastMenuItemTime = millis();
            NEW_OLED_MENU = false;
            OSD_handleCommand(OSD_CROSS_TOP);
            OSD_handleCommand('w');
            oled_menuItem = OLED_Profile;
        }

        if (results.value == IRKeyInfo) {
            lastMenuItemTime = millis();
            NEW_OLED_MENU = false;
            background_up(stroca1, _27, blue_fill);
            background_up(stroca2, _27, blue_fill);
            oled_menuItem = OLED_Info_Display;
        }

        switch (results.value) {
            case IRKeyMute:
                lastMenuItemTime = millis();
                if (audioMuted == 0) {
                    PT_MUTE(0x79);
                    NEW_OLED_MENU = false;
                    background_up(stroca1, _9, blue_fill);
                    for (int i = 0; i <= 800; i++) {
                        colour1 = yellowT;
                        number_stroca = stroca1;
                        __(M, _1), __(U, _2), __(T, _3), __(E, _4);
                        colour1 = main0;
                        __(O, _6), __(N, _7);
                        display.clear();
                        display.flipScreenVertically();
                        display.setTextAlignment(TEXT_ALIGN_LEFT);
                        display.setFont(ArialMT_Plain_16);
                        display.drawString(8, 15, "MUTE ON");
                        display.display();
                    }
                    oled_menuItem = OLED_None;
                    background_up(stroca1, _9, blue_fill);
                    OSD_Cut_0x01();
                    OSD();
                    audioMuted = 1;
                } else if (audioMuted == 1) {
                    PT_MUTE(0x78);
                    NEW_OLED_MENU = false;
                    background_up(stroca1, _9, blue_fill);
                    for (int i = 0; i <= 800; i++) {
                        colour1 = yellowT;
                        number_stroca = stroca1;
                        __(M, _1), __(U, _2), __(T, _3), __(E, _4);
                        colour1 = main0;
                        __(O, _6), __(F, _7), __(F, _8);
                        display.clear();
                        display.setTextAlignment(TEXT_ALIGN_LEFT);
                        display.setFont(ArialMT_Plain_16);
                        display.drawString(8, 15, "MUTE OFF");
                        display.display();
                    }
                    oled_menuItem = OLED_None;
                    background_up(stroca1, _9, blue_fill);
                    OSD_Cut_0x01();
                    OSD();
                    audioMuted = 0;
                }
                break;
            case kRecv2:
                lastMenuItemTime = millis();
                NEW_OLED_MENU = false;
                background_up(stroca1, _25, blue_fill);
                oled_menuItem = OLED_Volume_Adjust;
                break;
            case kRecv3:
                lastMenuItemTime = millis();
                NEW_OLED_MENU = false;
                background_up(stroca1, _25, blue_fill);
                oled_menuItem = OLED_Volume_Adjust;
                break;
        }

        irrecv.resume();
        delay(5);
    }
}
