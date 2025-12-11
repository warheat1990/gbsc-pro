// GBSC-Pro Implementation
// Contains all GBSC-Pro specific functionality

#include "gbs-control-pro.h"
#include "tv5725.h"
#include "osd.h"
#include "SSD1306Wire.h"
#include "options.h"
#include "OLEDMenuImplementation.h"

// OSD Global Variables
char colour1;
char number_stroca;
char sequence_number1;
char sequence_number2;
char sequence_number3;
char x1 = 0x2E, x2 = 0x2E, x3 = 0x2E, x4 = 0x2E, x5 = 0x2E, x6 = 0x2E, x7 = 0x2E, x8 = 0x2E, x9 = 0x2E;

#include <IRremoteESP8266.h>
#include <IRutils.h>
#include "OSD_TV/remote.h"
#include "OSD_TV/OSD_stv9426.h"
#include "OSD_TV/profile_name.h"
#include "OSD_TV/PT2257.h"

// External references to gbs-control.ino globals
extern SSD1306Wire display;
typedef TV5725<GBS_ADDR> GBS;
extern struct runTimeOptions *rto;
extern struct userOptions *uopt;
extern char serialCommand;
extern char userCommand;

// External function references
#include "ntsc_720x480.h"
extern uint8_t getVideoMode();
extern void applyPresets(uint8_t videoMode);
extern void setOutModeHdBypass(bool);
extern void shiftVerticalUpIF();
extern void shiftVerticalDownIF();
extern void shiftVertical(uint16_t amountToAdd, bool isUp);
extern void shiftHorizontalRightIF();
extern void shiftHorizontalLeftIF();
extern void shiftHorizontal(uint16_t amountToAdd, bool isRightShift);
extern void scaleHorizontal(uint16_t amountToAdd, bool largerScale);
extern void resetDigital();
extern void SyncProcessorOffOn();
extern void loadHdBypassSection();
extern boolean isInLowPowerMode();
extern void goLowPowerMode();
extern void goHighPowerMode();
extern void setResetParameters();
extern void saveUserPrefs();
extern void saveRGBPatternPreset(byte slot, byte colorMode);
extern void writeOneByte(uint8_t slaveRegister, uint8_t value);
extern void disableMotionAdaptDeinterlace();
extern void disableScanlines();
extern float getOutputFrameRate();
extern void writeProgramArrayNew(const uint8_t *programArray, boolean skipMDSection);
extern void doPostPresetLoadSteps();
extern void freezeVideo();

// External variable/function pointer for OSD
void (*osd_cx_ptr)(int, int, int) = nullptr;

// ============================================================================
// GLOBAL VARIABLE DEFINITIONS
// ============================================================================

const int kRecvPin = 2;
IRrecv irrecv(kRecvPin);
decode_results results;
uint8_t brightnessOrContrastOption = 0;
uint8_t Info = 0;
uint8_t selectedInputSource = 0;


uint8_t infoState = 0;
bool irDecodedFlag = false;
unsigned long lastSignalTime = 0;
unsigned long lastSystemTime = 0;
unsigned long lastWebUpdateTime = 0;
unsigned long lastMenuItemTime = 0;
unsigned long lastResolutionTime = 0;
unsigned long resolutionStartTime = 0;

uint16_t horizontalBlankStart = 0;
uint16_t horizontalBlankStop = 0;

unsigned char R_VAL = 128;
unsigned char G_VAL = 128;
unsigned char B_VAL = 128;

char osdDisplayValue = 0;
boolean irEnabled = true;
int selectedMenuLine = 0;
uint8_t Volume = 0;
boolean audioMuted = true;
int oled_menuItem = OSD_None;
int lastOledMenuItem = 0;
uint8_t oledClearFlag = 0;
boolean NEW_OLED_MENU = false;

uint8_t SVModeOption = 0;
uint8_t AVModeOption = 0;
uint8_t SVModeOptionChanged = 0;
uint8_t AVModeOptionChanged = 0;
uint8_t smoothOption = 0;
uint8_t lineOption = 0;
bool settingLineOptionChanged = false;
bool settingSmoothOptionChanged = false;

uint8_t brightness = 128;
uint8_t contrast = 128;
uint8_t saturation = 128;

uint8_t rgbComponentMode = 0;
uint8_t keepSettings = 0;
uint8_t tentativeResolution = 0;

int A1_yellow = 0;
int A2_main0 = 0;
int A3_main0 = 0;

// ============================================================================
// MENU TABLE
// ============================================================================

const MenuEntry menuTable[] = {
    {'0', handle_0},
    {'1', handle_1},
    {'2', handle_2},
    {'3', handle_3},
    {'4', handle_4},
    {'5', handle_5},
    {'6', handle_6},
    {'7', handle_7},
    {'8', handle_8},
    {'9', handle_9},
    {'a', handle_a},
    {'b', handle_b},
    {'c', handle_c},
    {'d', handle_d},
    {'e', handle_e},
    {'f', handle_f},
    {'g', handle_g},
    {'h', handle_h},
    {'i', handle_i},
    {'j', handle_j},
    {'k', handle_k},
    {'l', handle_l},
    {'m', handle_m},
    {'n', handle_n},
    {'o', handle_o},
    {'p', handle_p},
    {'q', handle_q},
    {'r', handle_r},
    {'s', handle_s},
    {'t', handle_t},
    {'u', handle_u},
    {'v', handle_v},
    {'w', handle_w},
    {'x', handle_x},
    {'y', handle_y},
    {'z', handle_z},
    {'A', handle_A},
    {'^', handle_caret},
    {'@', handle_at},
    {'!', handle_exclamation},
    {'#', handle_hash},
    {'$', handle_dollar},
    {'%', handle_percent},
    {'&', handle_ampersand},
    {'*', handle_asterisk}};

// ============================================================================
// FUNCTION IMPLEMENTATIONS
// ============================================================================

// Convert current R_VAL, G_VAL, B_VAL to YUV and write to video registers
// Uses ITU-R BT.601 standard for RGB to YUV conversion
void applyRGBtoYUVConversion(void)
{
    GBS::VDS_Y_OFST::write((signed char)((0.299f * (R_VAL - 128)) + (0.587f * (G_VAL - 128)) + (0.114f * (B_VAL - 128))));
    GBS::VDS_U_OFST::write((signed char)((-0.14713f * (R_VAL - 128)) - (0.28886f * (G_VAL - 128)) + (0.436f * (B_VAL - 128))));
    GBS::VDS_V_OFST::write((signed char)((0.615f * (R_VAL - 128)) - (0.51499f * (G_VAL - 128)) - (0.10001f * (B_VAL - 128))));
}

// Read YUV from video registers and convert to R_VAL, G_VAL, B_VAL
// Uses ITU-R BT.601 standard for YUV to RGB conversion
void readYUVtoRGBConversion(void)
{
    R_VAL = GBS::VDS_Y_OFST::read() + (1.13983f * GBS::VDS_V_OFST::read()) + 128;
    G_VAL = GBS::VDS_Y_OFST::read() - (0.39465f * GBS::VDS_U_OFST::read()) - (0.58060f * GBS::VDS_V_OFST::read()) + 128;
    B_VAL = GBS::VDS_Y_OFST::read() + (2.03211f * GBS::VDS_U_OFST::read()) + 128;
}

void UpDisplay(void)
{
    uint8_t videoMode = getVideoMode();
    if (videoMode == 0 && GBS::STATUS_SYNC_PROC_HSACT::read()) {
        videoMode = rto->videoStandardInput;
    }
    if (uopt->presetPreference != 2) {
        rto->useHdmiSyncFix = 1;
        if (rto->videoStandardInput == 14) {
            rto->videoStandardInput = 15;
        } else {
            applyPresets(videoMode);
        }
    } else {
        // printf("out off-1\n");
        setOutModeHdBypass(false); //    false
        if (rto->videoStandardInput != 15) {
            rto->autoBestHtotalEnabled = 0;
            if (rto->applyPresetDoneStage == 11) {
                rto->applyPresetDoneStage = 1;
            } else {
                rto->applyPresetDoneStage = 10;
            }
        } else {
            rto->applyPresetDoneStage = 1;
        }
    }
}
void Mode_Option(void)
{
    // Parameter validation with debugging information
    static char max_index = sizeof(modes) / sizeof(modes[0]) - 1;
    if (SVModeOptionChanged) {
        SVModeOptionChanged = 0;


        if (SVModeOption >= 0 && SVModeOption <= max_index) {
            Send_TvMode(modes[SVModeOption]);
        } else {
            Send_TvMode(modes[1]); // Force a fallback to Auto mode
        }
    }
    if (AVModeOptionChanged) {
        AVModeOptionChanged = 0;

        if (AVModeOption >= 0 && AVModeOption <= max_index) {
            Send_TvMode(modes[AVModeOption]);
        } else {
            Send_TvMode(modes[1]);
        }
    }
    if (settingLineOptionChanged) {
        settingLineOptionChanged = 0;
        Send_Line(lineOption);
    }
    if (settingSmoothOptionChanged) {
        settingSmoothOptionChanged = 0;
        Send_Smooth(smoothOption);
    }
}
boolean CheckInputFrequency()
{
    unsigned char freq = 0;
    static unsigned char freq_last;
    freq = getOutputFrameRate();
    if ((abs(freq_last - freq) < 9) || (freq_last == 0)) {
        freq_last = freq;
        return 0;
    }

    // Serial.print("freq");
    // Serial.printf("%d\n", freq);
    freq_last = freq;
    return 1;
}
void OSD_DISPLAY(const int T, const char C)
{
    __(T, (C * 2) + 1);
}
void ChangeSVModeOption(uint8_t num)
{
    SVModeOption = num;
    saveUserPrefs();
}
void ChangeAVModeOption(uint8_t num)
{
    AVModeOption = num;
    saveUserPrefs();
}
void Osd_Display(uint8_t start, const char str[])
{
    static uint8_t start_last = 0;
    if (str == NULL) {
        return;
    }
    if (start == 0XFF)
        start = start_last;
    else
        start_last = start;

    for (uint8_t count = 0; str[count] != '\0'; count++) {
        start_last = count + start + 1;

        if (str[count] == ' ') //
            continue;
        else if (str[count] == '=')
            OSD_DISPLAY(0x3D, count + start);
        else if (str[count] == '.')
            OSD_DISPLAY(0x2E, count + start);
        else if (str[count] == '\'')
            OSD_DISPLAY(0x27, count + start);
        else if (str[count] == '-')
            OSD_DISPLAY(0x3E, count + start);
        else if (str[count] == '/')
            OSD_DISPLAY(0x2F, count + start);
        else if (str[count] == ':')
            OSD_DISPLAY(0x3A, count + start);
        else
            OSD_DISPLAY(str[count], count + start);
    }
}
void OSD_writeString(int startPos, int row, const char *str)
{
    int pos = startPos;
    while (*str != '\0') {

        if (row == 1)
            osd_cx_ptr = OSD_c1;
        else if (row == 2)
            osd_cx_ptr = OSD_c2;
        else if (row == 3)
            osd_cx_ptr = OSD_c3;

        if (*str == ' ')
            osd_cx_ptr(*str, 1 + pos * 2, blue_fill);
        else if (*str == '=')
            osd_cx_ptr(0x3D, 1 + pos * 2, main0);
        else if (*str == '.')
            osd_cx_ptr(0x2E, 1 + pos * 2, main0);
        else if (*str == '\'')
            osd_cx_ptr(0x27, 1 + pos * 2, main0);
        else if (*str == '-')
            osd_cx_ptr(0x3E, 1 + pos * 2, main0);
        else if (*str == '/')
            osd_cx_ptr(0x2F, 1 + pos * 2, main0);
        else if (*str == ':')
            osd_cx_ptr(0x3A, 1 + pos * 2, main0);
        else
            osd_cx_ptr(*str, 1 + pos * 2, main0);

        pos++;
        str++;
    }
}
void OSD_selectOption()
{
    if (oled_menuItem == OSD_None) {
        NEW_OLED_MENU = true;
    } else {
        NEW_OLED_MENU = false;
    }

    if (oled_menuItem == OSD_Resolution) {
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
            OSD_menu_F('1');
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_menu_F('1');
                    oled_menuItem = OSD_ScreenSettings;
                    break;
                case IRKeyOk:
                    oled_menuItem = OSD_Resolution_1080;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('3');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ScreenSettings) {
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
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 1;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_SystemSettings;
                    break;
                case IRKeyOk:
                    oled_menuItem = OSD_ScreenSettings_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('6');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ColorSettings) {
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
            OSD_menu_F('2');
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyUp:
                    selectedMenuLine = 1;
                    // OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_SystemSettings;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    oled_menuItem = OSD_ResetDefault;
                    // OSD_menu_F('2');

                    // oled_menuItem = OSD_ResetDefault;
                    // selectedMenuLine = 3;
                    // OSD_menu_F(OSD_CROSS_BOTTOM);
                    // OSD_menu_F('2');

                    break;
                case IRKeyOk:
                    // oled_menuItem = OSD_ColorSettings_Bright;
                    oled_menuItem = OSD_ColorSettings_RGB_R;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('d');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings) {
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
            OSD_menu_F('2');
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyUp:
                    oled_menuItem = OSD_ScreenSettings;
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('1');
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    // OSD_menu_F('2');
                    oled_menuItem = OSD_ColorSettings;
                    break;
                case IRKeyOk:
                    oled_menuItem = OSD_SystemSettings_SVAVInput;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('i');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    // if (oled_menuItem == OSD_Developer)
    // {

    //     if(oledClearFlag)
    // display.clear();
    // oledClearFlag = ~0;display.setColor(OLEDDISPLAY_COLOR::WHITE);
    //     display.drawString(1, 0, "Menu-");
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
    //             OSD_menu_F('2');
    //             oled_menuItem = OSD_SystemSettings;
    //             break;
    //         case IRKeyDown:
    //             selectedMenuLine = 3;
    //             OSD_menu_F('2');
    //             oled_menuItem = OSD_ResetDefault;
    //             break;
    //         case IRKeyOk:
    //             oled_menuItem = 104;
    //             OSD_menu_F(OSD_CROSS_TOP);
    //             OSD_menu_F('q');
    //             break;
    //         case IRKeyExit:
    //             oled_menuItem = OSD_None;
    //             OSD_clear();
    //             OSD();
    //             break;
    //         }
    //         irrecv.resume();
    //     }
    // }

    else if (oled_menuItem == OSD_ResetDefault) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.drawString(1, 0, "Menu-");
        display.drawString(1, 28, "Reset Settings");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyOk) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, yellow);
            OSD_menu_F('2');
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_menu_F('2');
                    oled_menuItem = OSD_ColorSettings;

                    // oled_menuItem = OSD_ColorSettings;
                    // selectedMenuLine = 2;
                    // OSD_menu_F(OSD_CROSS_MID);
                    // OSD_menu_F('2');

                    break;
                case IRKeyOk:
                    userCommand = '1';
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Resolution_1080) {
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
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_menu_F('3');
                    oled_menuItem = OSD_Resolution_1024;
                    break;
                case IRKeyOk:
                    // uopt->preferScalingRgbhv = true;
                    userCommand = 's';
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Resolution_1024) {
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
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('3');
                    oled_menuItem = OSD_Resolution_1080;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_menu_F('3');
                    oled_menuItem = OSD_Resolution_960;
                    break;
                case IRKeyOk:
                    // uopt->preferScalingRgbhv = true;
                    userCommand = 'p';
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Resolution_960) {
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
            OSD_menu_F('3');
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_menu_F('3');
                    oled_menuItem = OSD_Resolution_1024;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Resolution_720;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('4');
                    break;
                case IRKeyOk:
                    // uopt->preferScalingRgbhv = true;
                    userCommand = 'f';
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Resolution_720) {
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
            OSD_menu_F('4');
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Resolution_960;
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('3');
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_menu_F('4');
                    oled_menuItem = OSD_Resolution_480;
                    break;
                case IRKeyOk:
                    // uopt->preferScalingRgbhv = true;
                    userCommand = 'g';
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Resolution_480) {
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
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('4');
                    oled_menuItem = OSD_Resolution_720;
                    break;
                case IRKeyOk:
                    userCommand = 'h';
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    // if (oled_menuItem == OSD_Resolution_Downscale)
    // {
    //     if(oledClearFlag)
    // display.clear();
    // oledClearFlag = ~0;display.setColor(OLEDDISPLAY_COLOR::WHITE);
    //     display.drawString(1, 0, "Menu-");
    //     display.drawString(1, 28, "Downscale");
    //     display.display();

    //     if (results.value == IRKeyDown  ||  results.value == IRKeyUp)
    //     {
    //         OSD_c3(icon4, P0, yellow);
    //         OSD_c1(icon4, P0, blue_fill);
    //         OSD_c2(icon4, P0, blue_fill);
    //         OSD_menu_F('4');
    //     }

    //     if (irrecv.decode(&results))
    //     {
    //         switch (results.value)
    //         {
    //         case IRKeyMenu:
    //             OSD_menu_F(OSD_CROSS_MID);
    //             OSD_menu_F('1');
    //             oled_menuItem = OSD_Resolution;
    //             break;
    //         case IRKeyUp:
    //             selectedMenuLine = 2;
    //             OSD_menu_F('4');
    //             oled_menuItem = OSD_Resolution_480;
    //             break;
    //         case IRKeyDown:
    //             oled_menuItem = OSD_Resolution_pass;
    //             OSD_menu_F(OSD_CROSS_TOP);
    //             OSD_menu_F('5');
    //             break;
    //         case IRKeyOk:
    //             userCommand = 'L';
    //             break;
    //         case IRKeyExit:
    //             oled_menuItem = OSD_Input;
    //             OSD_clear();
    //             OSD();
    //             break;
    //         }
    //         irrecv.resume();
    //     }
    // }

    else if (oled_menuItem == OSD_Resolution_pass) {
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
            OSD_menu_F('4');
        }
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('4');
                    oled_menuItem = OSD_Resolution_720;
                    break;
                // case IRKeyDown:
                //     oled_menuItem = OSD_Resolution_1080;
                //     OSD_menu_F(OSD_CROSS_TOP);
                //     OSD_menu_F('3');
                //     break;
                case IRKeyOk:
                    // tentativeResolution = uopt->presetPreference;
                    // if(Info == InfoVGA)
                    // {
                    //     uopt->preferScalingRgbhv = false;
                    // }
                    // else
                    // {
                    //   // uopt->preferScalingRgbhv = true;
                    //   serialCommand = 'K';
                    // }

                    // saveUserPrefs();

                    // OSD_menu_F(OSD_CROSS_TOP);
                    // OSD_menu_F('!');
                    // oled_menuItem = OSD_Resolution_RetainedSettings;
                    // keepSettings = 0;
                    // resolutionStartTime = millis();
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    } else if (oled_menuItem == OSD_Resolution_RetainedSettings) {
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
        //   OSD_menu_F('·');
        // }
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('4');
                    oled_menuItem = OSD_Resolution_pass;
                    break;
                case IRKeyRight:
                    keepSettings = 0;
                    OSD_menu_F('!');
                    break;
                case IRKeyLeft:
                    keepSettings = 1;
                    OSD_menu_F('!');
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

                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('4');
                    oled_menuItem = OSD_Resolution_pass;
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ScreenSettings_Main) {
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
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_ScreenSettings;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_menu_F('6');
                    oled_menuItem = OSD_ScreenSettings_Scale;
                    break;
                case IRKeyOk:
                    oled_menuItem = OSD_ScreenSettings_ScaleAdjust;
                    OSD_menu_F('6');
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ScreenSettings_Scale) {
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
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_ScreenSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('6');
                    oled_menuItem = OSD_ScreenSettings_Main;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_menu_F('6');
                    oled_menuItem = OSD_ScreenSettings_Position;
                    break;
                case IRKeyOk:
                    oled_menuItem = OSD_ScreenSettings_PositionAdjust;
                    OSD_menu_F('6');
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ScreenSettings_Position) {
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
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_ScreenSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_menu_F('6');
                    oled_menuItem = OSD_ScreenSettings_Scale;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_SystemSettings_AV;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('o');
                    break;
                case IRKeyOk:
                    oled_menuItem = OSD_ScreenSettings_Advanced;
                    OSD_menu_F('6');
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ScreenSettings_ScaleAdjust) {
        if (results.value == IRKeyOk) {
            OSD_menu_F('6');
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('6');
                    oled_menuItem = OSD_ScreenSettings_Main;
                    break;
                case IRKeyOk:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('6');
                    oled_menuItem = OSD_ScreenSettings_Main;
                    break;
                case IRKeyRight:
                    lastMenuItemTime = millis();
                    serialCommand = '6';
                    if (GBS::IF_HBIN_SP::read() >= 10) {
                    } else {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = 0x14;
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
                            colour1 = 0x14;
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ScreenSettings_PositionAdjust) {
        if (results.value == IRKeyOk) {
            OSD_menu_F('6');
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
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('6');
                    oled_menuItem = OSD_ScreenSettings_Scale;
                    break;
                case IRKeyOk:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('6');
                    oled_menuItem = OSD_ScreenSettings_Scale;
                    break;
                case IRKeyRight:
                    lastMenuItemTime = millis();
                    serialCommand = 'h';
                    if (GBS::VDS_HSCALE::read() == 1023) {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = 0x14;
                            number_stroca = stroca2;
                            Osd_Display(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca2;
                    Osd_Display(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyLeft:
                    lastMenuItemTime = millis();
                    serialCommand = 'z';
                    if (GBS::VDS_HSCALE::read() <= 256) {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = 0x14;
                            number_stroca = stroca2;
                            Osd_Display(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca2;
                    Osd_Display(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyUp:
                    lastMenuItemTime = millis();
                    serialCommand = '5';
                    if (GBS::VDS_VSCALE::read() == 1023) {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = 0x14;
                            number_stroca = stroca2;
                            Osd_Display(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca2;
                    Osd_Display(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyDown:
                    lastMenuItemTime = millis();
                    serialCommand = '4';
                    if (GBS::VDS_VSCALE::read() <= 256) {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = 0x14;
                            number_stroca = stroca2;
                            Osd_Display(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca2;
                    Osd_Display(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ScreenSettings_Advanced) {
        if (results.value == IRKeyOk) {
            OSD_menu_F('6');
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
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('6');
                    oled_menuItem = OSD_ScreenSettings_Position;
                    break;
                case IRKeyOk:
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('6');
                    oled_menuItem = OSD_ScreenSettings_Position;
                    break;
                case IRKeyRight:

                    userCommand = 'A';
                    if ((GBS::VDS_DIS_HB_ST::read() > 4) && (GBS::VDS_DIS_HB_SP::read() < (GBS::VDS_HSYNC_RST::read() - 4))) {
                    } else {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = 0x14;
                            number_stroca = stroca3;
                            Osd_Display(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca3;
                    Osd_Display(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyLeft:

                    userCommand = 'B';
                    if ((GBS::VDS_DIS_HB_ST::read() < (GBS::VDS_HSYNC_RST::read() - 4)) && (GBS::VDS_DIS_HB_SP::read() > 4)) {
                    } else {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = 0x14;
                            number_stroca = stroca3;
                            Osd_Display(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca3;
                    Osd_Display(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyUp:

                    userCommand = 'C';
                    if ((GBS::VDS_DIS_VB_ST::read() > 6) && (GBS::VDS_DIS_VB_SP::read() < (GBS::VDS_VSYNC_RST::read() - 4))) {
                    } else {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = 0x14;
                            number_stroca = stroca3;
                            Osd_Display(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca3;
                    Osd_Display(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyDown:

                    userCommand = 'D';
                    if ((GBS::VDS_DIS_VB_ST::read() < (GBS::VDS_VSYNC_RST::read() - 4)) && (GBS::VDS_DIS_VB_SP::read() > 6)) {
                    } else {
                        for (int p = 0; p <= 400; p++) {
                            colour1 = 0x14;
                            number_stroca = stroca3;
                            Osd_Display(20, "limit");
                            __(0x0d, _25);
                        }
                    }
                    colour1 = blue_fill;
                    number_stroca = stroca3;
                    Osd_Display(20, "limit");
                    __(0x0d, _25);
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ColorSettings_Bright) {
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
            OSD_menu_F('a');
        }

        OSD_menu_F('e');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_ColorSettings;

                    break;
                case IRKeyUp:
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('d');
                    oled_menuItem = OSD_ColorSettings_RGB_B;

                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_menu_F('a');
                    oled_menuItem = OSD_ColorSettings_Contrast;
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ColorSettings_Contrast) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Color");
        display.drawString(1, 22, "Scanlines");
        // uopt->scanlineStrength
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

        OSD_menu_F('e');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('a'); //
                    oled_menuItem = OSD_ColorSettings_Bright;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_menu_F('a'); //
                    oled_menuItem = OSD_ColorSettings_Saturation;
                    break;
                case IRKeyRight:
                    userCommand = 'K';
                    break;
                case IRKeyLeft:
                    userCommand = 'K';
                    break;
                case IRKeyOk:
                    userCommand = '7';
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ColorSettings_Saturation) {
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
            OSD_menu_F('a');
        }

        OSD_menu_F('e');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_menu_F('a'); //
                    oled_menuItem = OSD_ColorSettings_Contrast;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_ColorSettings_Sharpness;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('b');
                    break;
                case IRKeyOk:
                    userCommand = 'm';
                    break;
                // case IRKeyLeft:
                //   userCommand = 'm';
                //   break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ColorSettings_Sharpness) {
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
            OSD_menu_F('b');
        }

        OSD_menu_F('f');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_ColorSettings;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_ColorSettings_Saturation;
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('a');
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_menu_F('b'); //
                    oled_menuItem = OSD_ColorSettings_ColorTemp;
                    break;
                case IRKeyOk:
                    userCommand = 'W';
                    break;
                // case IRKeyLeft:
                //   userCommand = 'W';
                //   break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ColorSettings_ColorTemp) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->Color");
        display.drawString(1, 22, "Peaking");
        if (uopt->wantPeaking) {
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

        OSD_menu_F('f');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('b'); //
                    oled_menuItem = OSD_ColorSettings_Sharpness;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_menu_F('b'); //
                    oled_menuItem = OSD_ColorSettings_Advanced;
                    break;
                case IRKeyOk:
                    serialCommand = 'f';
                    break;
                // case IRKeyLeft:
                //   serialCommand = 'f';
                //   break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ColorSettings_Advanced) {
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
            OSD_menu_F('b');
        }

        OSD_menu_F('f');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_menu_F('b'); //
                    oled_menuItem = OSD_ColorSettings_ColorTemp;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_ColorSettings_RGB_Load;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('c');
                    break;
                case IRKeyOk:
                    serialCommand = 'V';
                    break;
                // case IRKeyLeft:
                //   serialCommand = 'V';
                //   break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ColorSettings_RGB_R) // D
    {
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
            OSD_menu_F('d');
        }
        OSD_menu_F('g');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_ColorSettings;
                    break;
                // case IRKeyUp:
                //     oled_menuItem = OSD_ColorSettings_Advanced;
                //     OSD_menu_F(OSD_CROSS_BOTTOM);
                //     OSD_menu_F('b');
                //     break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_menu_F('d'); //
                    oled_menuItem = OSD_ColorSettings_RGB_G;
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ColorSettings_RGB_G) {
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
        OSD_menu_F('g');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('d'); //
                    oled_menuItem = OSD_ColorSettings_RGB_R;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_menu_F('d'); //
                    oled_menuItem = OSD_ColorSettings_RGB_B;
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ColorSettings_RGB_B) {
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
            OSD_menu_F('d');
        }
        // (signed char)GBS::VDS_Y_OFST::read() = GBS::VDS_Y_OFST::read();
        // (signed char)GBS::VDS_U_OFST::read() = GBS::VDS_U_OFST::read();
        // (signed char)GBS::VDS_V_OFST::read() = GBS::VDS_V_OFST::read();
        OSD_menu_F('g');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_menu_F('d'); //
                    oled_menuItem = OSD_ColorSettings_RGB_G;
                    break;
                case IRKeyDown:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('a');
                    oled_menuItem = OSD_ColorSettings_Bright;
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ColorSettings_RGB_Preset) {
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
            OSD_menu_F('c');
        }

        OSD_menu_F('h');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_ColorSettings;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_ColorSettings_RGB_B;
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('d');
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_menu_F('c'); //
                    oled_menuItem = OSD_ColorSettings_RGB_Save;
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ColorSettings_RGB_Save) {
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
        OSD_menu_F('h');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_ColorSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('c'); //
                    oled_menuItem = OSD_ColorSettings_RGB_Preset;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_menu_F('c'); //
                    oled_menuItem = OSD_ColorSettings_RGB_Load;
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_ColorSettings_RGB_Load) {
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
            OSD_menu_F('c');
        }

        // OSD_menu_F('h');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_ColorSettings;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_ColorSettings_Advanced;
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('b');
                    break;
                // case IRKeyDown:
                //   oled_menuItem = OSD_ColorSettings_Bright;
                //   OSD_menu_F(OSD_CROSS_TOP);
                //   OSD_menu_F('a');
                // break;
                case IRKeyOk:
                    userCommand = 'U';
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings_SV) {
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

            OSD_menu_F('i');
        }

        OSD_menu_F(j);

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_menu_F('i');
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Compatibility;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Mode;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('k');
                    break;
                case IRKeyOk:
                    serialCommand = 'Z';
                    break;
                // case IRKeyLeft:
                //   serialCommand = 'Z';
                //   break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings_AV) {
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
            OSD_menu_F('o');
        }
        OSD_menu_F('p');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_ScreenSettings;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_ScreenSettings_Position;
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('6');
                    break;
                // case IRKeyDown:
                //     selectedMenuLine = 3;
                //     OSD_menu_F('i'); //
                //     oled_menuItem = OSD_SystemSettings_Upscaling;
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
                    UpDisplay();
                } break;
                // case IRKeyRight:
                //   userCommand = 'v';
                //   break;
                // case IRKeyLeft:
                //   userCommand = 'v';
                //   break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings_Upscaling) {
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
            OSD_menu_F('i');
        }

        OSD_menu_F('j');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_menu_F('i');
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Compatibility;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Mode;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('k');
                    break;
                case IRKeyOk:
                    // userCommand = 'x';
                    // uopt->preferScalingRgbhv = !uopt->preferScalingRgbhv;
                    // UpDisplay();
                    break;
                // case IRKeyLeft:
                //   userCommand = 'x';
                //   break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    // else if (oled_menuItem == OSD_SystemSettings_ComponentVGA)
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
    //         OSD_menu_F('k');
    //     }

    //     OSD_menu_F('l');

    //     if (irrecv.decode(&results))
    //     {
    //         switch (results.value)
    //         {
    //         case IRKeyMenu:
    //             OSD_menu_F(OSD_CROSS_TOP);
    //             OSD_menu_F('2');
    //             oled_menuItem = OSD_SystemSettings;
    //             break;
    // case IRKeyUp:
    //     oled_menuItem = OSD_SystemSettings_Upscaling;
    //     OSD_menu_F(OSD_CROSS_BOTTOM);
    //     OSD_menu_F('i');
    //     break;
    // case IRKeyDown:
    //     selectedMenuLine = 2;
    //     OSD_menu_F('k'); //
    //     oled_menuItem = OSD_SystemSettings_SVAVInput_Line;
    //     break;
    //         case IRKeyRight:
    //             serialCommand = 'L';
    //             break;
    //         case IRKeyLeft:
    //             serialCommand = 'L';
    //             break;
    //         case IRKeyExit:
    //             oled_menuItem = OSD_Input;
    //             OSD_clear();
    //             OSD();
    //             break;
    //         }
    //         irrecv.resume();
    //     }
    // }

    else if (oled_menuItem == OSD_SystemSettings_SVAVInput_Line) {
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

        OSD_menu_F('l');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('k'); //
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Mode;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_menu_F('k'); //
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Saturation_Main;
                    break;
                case IRKeyOk:
                    if (uopt->PalForce60 == 0) {
                        uopt->PalForce60 = 1;
                    } else {
                        uopt->PalForce60 = 0;
                    }
                    saveUserPrefs();
                    UpDisplay();
                    break;
                // case IRKeyLeft:
                //   userCommand = '0';
                //   break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings_SVAVInput_Smooth_Main) {
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
            OSD_menu_F('m');
        }
        OSD_menu_F('n');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_menu_F('m'); //
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Contrast_Main;
                    break;
                // case IRKeyDown:
                //   oled_menuItem = OSD_SystemSettings_SVAVInput_Bright_Main;
                //   OSD_menu_F(OSD_CROSS_TOP);
                //   OSD_menu_F('m');
                //   break;
                case IRKeyOk:
                    userCommand = 'X';
                    break;
                // case IRKeyLeft:
                //   userCommand = 'X';
                //   break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings_SVAVInput_Bright_Main) {
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
            OSD_menu_F('m');
        }
        OSD_menu_F('n');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_SystemSettings;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Saturation_Main;
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('k');
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_menu_F('m'); //
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Contrast_Main;
                    break;
                case IRKeyOk:
                    userCommand = 'w';
                    break;
                // case IRKeyLeft:
                //   userCommand = 'w';
                //   break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings_SVAVInput_Contrast_Main) {
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

        OSD_menu_F('n');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('m'); //
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Bright_Main;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_menu_F('m'); //
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Smooth_Main;
                    break;
                case IRKeyOk:
                    userCommand = '5';
                    break;
                // case IRKeyLeft:
                //   userCommand = '5';
                //   break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings_SVAVInput_Saturation_Main) {
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
            OSD_menu_F('k');
        }

        OSD_menu_F('l');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_menu_F('k'); //
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Line;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Bright_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('m');
                    break;
                case IRKeyOk:
                    userCommand = 'i';
                    break;
                // case IRKeyLeft:
                //   userCommand = 'i';
                //   break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings_SVAVInput_Mode) {
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
            OSD_menu_F('k');
        }

        OSD_menu_F('l');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_SystemSettings;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_SystemSettings_SV;
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('i');
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_menu_F('k'); //
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Line;
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
                    UpDisplay();
                    break;
                // case IRKeyRight:
                //   userCommand = 'q';
                //   break;
                // case IRKeyLeft:
                //   userCommand = 'r';
                //   break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    // else if (oled_menuItem == OSD_Advanced_MemoryAdjust)
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
    //     OSD_menu_F('q');
    //   }

    //   OSD_menu_F('r');

    //   if (irrecv.decode(&results))
    //   {
    //     irDecodedFlag = 1;
    //     switch (results.value)
    //     {
    //     case IRKeyMenu:
    //       OSD_menu_F(OSD_CROSS_MID);
    //       OSD_menu_F('2');
    //       oled_menuItem = OSD_Developer;
    //       break;
    //     case IRKeyDown:
    //       selectedMenuLine = 2;
    //       OSD_menu_F('q'); //
    //       oled_menuItem = 105;
    //       break;
    //     case IRKeyRight:
    //       serialCommand = '+';
    //       break;
    //     case IRKeyLeft:
    //       serialCommand = '-';
    //       break;
    //     case IRKeyExit:
    //       oled_menuItem = OSD_Input;
    //       OSD_clear();
    //       OSD();
    //       break;
    //     }
    //     irrecv.resume();
    //   }
    // }

    // else if (oled_menuItem == OSD_Advanced_HSyncAdjust)
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

    //   OSD_menu_F('r');

    //   if (irrecv.decode(&results))
    //   {
    //     irDecodedFlag = 1;
    //     switch (results.value)
    //     {
    //     case IRKeyMenu:
    //       OSD_menu_F(OSD_CROSS_MID);
    //       OSD_menu_F('2');
    //       oled_menuItem = OSD_Developer;
    //       break;
    //     case IRKeyUp:
    //       selectedMenuLine = 1;
    //       OSD_menu_F('q'); //
    //       oled_menuItem = 104;
    //       break;
    //     case IRKeyDown:
    //       selectedMenuLine = 3;
    //       OSD_menu_F('q'); //
    //       oled_menuItem = 106;
    //       break;
    //     case IRKeyRight:
    //       serialCommand = '0';
    //       break;
    //     case IRKeyLeft:
    //       serialCommand = '1';
    //       break;
    //     case IRKeyExit:
    //       oled_menuItem = OSD_Input;
    //       OSD_clear();
    //       OSD();
    //       break;
    //     }
    //     irrecv.resume();
    //   }
    // }

    // else if (oled_menuItem == OSD_Advanced_HTotalAdjust)
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
    //     OSD_menu_F('q');
    //   }

    //   OSD_menu_F('r');

    //   if (irrecv.decode(&results))
    //   {
    //     irDecodedFlag = 1;
    //     switch (results.value)
    //     {
    //     case IRKeyMenu:
    //       OSD_menu_F(OSD_CROSS_MID);
    //       OSD_menu_F('2');
    //       oled_menuItem = OSD_Developer;
    //       break;
    //     case IRKeyUp:
    //       selectedMenuLine = 2;
    //       OSD_menu_F('q'); //
    //       oled_menuItem = 105;
    //       break;
    //     case IRKeyDown:
    //       oled_menuItem = 107;
    //       OSD_menu_F(OSD_CROSS_TOP);
    //       OSD_menu_F('s');
    //       break;
    //     case IRKeyRight:
    //       serialCommand = 'a';
    //       break;
    //     case IRKeyLeft:
    //       serialCommand = 'A';
    //       break;
    //     case IRKeyExit:
    //       oled_menuItem = OSD_Input;
    //       OSD_clear();
    //       OSD();
    //       break;
    //     }
    //     irrecv.resume();
    //   }
    // }

    // else if (oled_menuItem == OSD_Advanced_DebugView)
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
    //     OSD_menu_F('s');
    //   }

    //   OSD_menu_F('t');

    //   if (irrecv.decode(&results))
    //   {
    //     irDecodedFlag = 1;
    //     switch (results.value)
    //     {
    //     case IRKeyMenu:
    //       OSD_menu_F(OSD_CROSS_MID);
    //       OSD_menu_F('2');
    //       oled_menuItem = OSD_Developer;
    //       break;
    //     case IRKeyUp:
    //       oled_menuItem = 106;
    //       OSD_menu_F(OSD_CROSS_BOTTOM);
    //       OSD_menu_F('q');
    //       break;
    //     case IRKeyDown:
    //       selectedMenuLine = 2;
    //       OSD_menu_F('s'); //
    //       oled_menuItem = 108;
    //       break;
    //     case IRKeyRight:
    //       serialCommand = 'D';
    //       break;
    //     case IRKeyLeft:
    //       serialCommand = 'D';
    //       break;
    //     case IRKeyExit:
    //       oled_menuItem = OSD_Input;
    //       OSD_clear();
    //       OSD();
    //       break;
    //     }
    //     irrecv.resume();
    //   }
    // }

    // else if (oled_menuItem == OSD_Advanced_ADCFilter)
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

    //   OSD_menu_F('t');

    //   if (irrecv.decode(&results))
    //   {
    //     irDecodedFlag = 1;
    //     switch (results.value)
    //     {
    //     case IRKeyMenu:
    //       OSD_menu_F(OSD_CROSS_MID);
    //       OSD_menu_F('2');
    //       oled_menuItem = OSD_Developer;
    //       break;
    //     case IRKeyUp:
    //       selectedMenuLine = 1;
    //       OSD_menu_F('s'); //
    //       oled_menuItem = 107;
    //       break;
    //     case IRKeyDown:
    //       selectedMenuLine = 3;
    //       OSD_menu_F('s');
    //       oled_menuItem = 153;
    //       break;
    //     case IRKeyRight:
    //       serialCommand = 'F';
    //       break;
    //     case IRKeyLeft:
    //       serialCommand = 'F';
    //       break;
    //     case IRKeyExit:
    //       oled_menuItem = OSD_Input;
    //       OSD_clear();
    //       OSD();
    //       break;
    //     }
    //     irrecv.resume();
    //   }
    // }

    // else if (oled_menuItem == OSD_Advanced_FreezeCapture)
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
    //     OSD_menu_F('s'); //
    //   }

    //   OSD_menu_F('t');

    //   if (irrecv.decode(&results))
    //   {
    //     irDecodedFlag = 1;
    //     switch (results.value)
    //     {
    //     case IRKeyMenu:
    //       OSD_menu_F(OSD_CROSS_MID);
    //       OSD_menu_F('2');
    //       oled_menuItem = OSD_Developer;
    //       break;
    //     case IRKeyUp:
    //       selectedMenuLine = 2;
    //       OSD_menu_F('s'); //
    //       oled_menuItem = 108;
    //       break;
    //     case IRKeyDown:
    //       oled_menuItem = 104;
    //       OSD_menu_F(OSD_CROSS_TOP);
    //       OSD_menu_F('q');
    //       break;
    //     case IRKeyRight:
    //       userCommand = 'F';
    //       break;
    //     case IRKeyLeft:
    //       userCommand = 'F';
    //       break;
    //     case IRKeyExit:
    //       oled_menuItem = OSD_Input;
    //       OSD_clear();
    //       OSD();
    //       break;
    //     }
    //     irrecv.resume();
    //   }
    // }

    // else if (oled_menuItem == OSD_SystemSettings_Volume)
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
    //     OSD_menu_F('u');
    //   }

    //   OSD_menu_F('v');

    //   if (irrecv.decode(&results))
    //   {
    //     irDecodedFlag = 1;
    //     switch (results.value)
    //     {
    //     case IRKeyMenu:
    //       OSD_menu_F(OSD_CROSS_BOTTOM);
    //       OSD_menu_F('2');
    //       oled_menuItem = OSD_ResetDefault;
    //       break;
    //     case IRKeyDown:
    //       selectedMenuLine = 2;
    //       OSD_menu_F('u'); //
    //       oled_menuItem = OSD_SystemSettings_Restart;
    //       break;
    //     case IRKeyRight:
    //       serialCommand = 'c';
    //       break;
    //     case IRKeyLeft:
    //       serialCommand = 'c';
    //       break;
    //     case IRKeyExit:
    //       oled_menuItem = OSD_Input;
    //       OSD_clear();
    //       OSD();
    //       break;
    //     }
    //     irrecv.resume();
    //   }
    // }

    else if (oled_menuItem == OSD_SystemSettings_Restart) {
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

        OSD_menu_F('v');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_ResetDefault;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('u'); //
                    oled_menuItem = OSD_SystemSettings_Volume;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_menu_F('u'); //
                    oled_menuItem = OSD_SystemSettings_Info;
                    break;
                case IRKeyOk:
                    userCommand = 'a';
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    // else if (oled_menuItem == OSD_SystemSettings_Info)
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

    //     OSD_menu_F('v');

    //     if (irrecv.decode(&results))
    //     {
    //         switch (results.value)
    //         {
    //         case IRKeyMenu:
    //             OSD_menu_F(OSD_CROSS_BOTTOM);
    //             OSD_menu_F('2');
    //             oled_menuItem = OSD_ResetDefault;
    //             break;
    //         case IRKeyUp:
    //             selectedMenuLine = 2;
    //             OSD_menu_F('u'); //
    //             oled_menuItem = OSD_SystemSettings_Restart;
    //             break;
    //         case IRKeyOk:
    //             userCommand = '1';
    //             break;
    //         case IRKeyExit:
    //             oled_menuItem = OSD_None;
    //             OSD_clear();
    //             OSD();
    //             break;
    //         }
    //         irrecv.resume();
    //     }
    // }

    else if (oled_menuItem == OSD_Input) {
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
            OSD_menu_F('1');
        }

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Resolution;

                    break;
                case IRKeyOk:
                    oled_menuItem = OSD_Input_RGBs;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('@');
                    selectedMenuLine = 1;
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None; // 154
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
                    //     oled_menuItem = OSD_Info_Display;
                    //     break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Input_RGBs) {
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
            OSD_menu_F('@');
        }
        // OSD_menu_F('*');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyOk:
                    infoState = 0;
                    rgbComponentMode = 1;
                    InputRGBs_mode(rgbComponentMode);
                    rto->isInLowPowerMode = false;
                    break;

                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1'); //
                    oled_menuItem = OSD_Input;
                    break;
                // case IRKeyUp:
                //     selectedMenuLine = 3;
                //     OSD_menu_F(OSD_CROSS_BOTTOM);
                //     OSD_menu_F('#');
                //     oled_menuItem = OSD_Input_AV;
                //     break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_menu_F('@');
                    oled_menuItem = OSD_Input_RGsB;
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Input_RGsB) {
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
            OSD_menu_F('@');
        }
        // OSD_menu_F('*');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyOk:
                    infoState = 0;
                    rgbComponentMode = 1;
                    InputRGsB_mode(rgbComponentMode);
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1'); //
                    oled_menuItem = OSD_Input;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('@');
                    oled_menuItem = OSD_Input_RGBs;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_menu_F('@');
                    oled_menuItem = OSD_Input_VGA;
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    } else if (oled_menuItem == OSD_Input_VGA) {
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

            OSD_menu_F('@');
        }
        // OSD_menu_F('*');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyOk:
                    infoState = 0;
                    rgbComponentMode = 0;
                    InputVGA_mode(rgbComponentMode);
                    // printf("\n\n rgbComponentMode %d \n\n", rgbComponentMode);
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1'); //
                    oled_menuItem = OSD_Input;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_menu_F('@');
                    oled_menuItem = OSD_Input_RGsB;
                    break;
                case IRKeyDown:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('#');
                    oled_menuItem = OSD_Input_YPBPR;
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    } else if (oled_menuItem == OSD_Input_YPBPR) {
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
            OSD_menu_F('#');
        }
        OSD_menu_F('$');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {

                case IRKeyOk:
                    infoState = 0;
                    InputYUV();
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1'); //
                    oled_menuItem = OSD_Input;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 3;
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('@');
                    oled_menuItem = OSD_Input_VGA;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_menu_F('#');
                    oled_menuItem = OSD_Input_SV;
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    } else if (oled_menuItem == OSD_Input_SV) {
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
            OSD_menu_F('#');
        }
        OSD_menu_F('$');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {

                case IRKeyOk:
                    infoState = 0;
                    InputSV_mode(SVModeOption + 1);
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1'); //
                    oled_menuItem = OSD_Input;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('#');
                    oled_menuItem = OSD_Input_YPBPR;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_menu_F('#');
                    oled_menuItem = OSD_Input_AV;
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    } else if (oled_menuItem == OSD_Input_AV) {
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

            OSD_menu_F('#');
        }
        OSD_menu_F('$');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyOk:
                    infoState = 0;
                    InputAV_mode(AVModeOption + 1);
                    // rto->isInLowPowerMode = false;
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1'); //
                    oled_menuItem = OSD_Input;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_menu_F('#');
                    oled_menuItem = OSD_Input_SV;
                    break;
                // case IRKeyDown:
                //     oled_menuItem = OSD_Input_RGBs;
                //     selectedMenuLine = 1;
                //     OSD_menu_F(OSD_CROSS_TOP);
                //     OSD_menu_F('@');
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings_SVAVInput) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(1, 0, "Menu->System");
        display.drawString(1, 28, "Sv-Av InPutSet");
        display.display();

        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);

            OSD_menu_F('i');
        }

        OSD_menu_F('j');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyOk:
                    if (Info == InfoSV || Info == InfoAV) {
                        selectedMenuLine = 1;
                        OSD_menu_F(OSD_CROSS_TOP);
                        OSD_menu_F('^');
                        oled_menuItem = OSD_SystemSettings_SVAVInput_DoubleLine;
                    }
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_SystemSettings;
                    break;
                // case IRKeyUp:
                //     selectedMenuLine = 2;
                //     OSD_menu_F(OSD_CROSS_MID);
                //     OSD_menu_F('o');
                //     oled_menuItem = OSD_SystemSettings_SV;
                //     break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_menu_F('i');
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Compatibility;
                    break;

                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings_SVAVInput_DoubleLine) {
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

            OSD_menu_F('^');
        }
        OSD_menu_F('&');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('i'); //
                    oled_menuItem = OSD_SystemSettings_SVAVInput;
                    break;

                // case IRKeyUp:
                //   selectedMenuLine = 2;
                //   OSD_menu_F('^');
                //   oled_menuItem = OSD_SystemSettings_SVAVInput_Smooth;
                //   break;
                case IRKeyDown:
                    selectedMenuLine = 2;
                    OSD_menu_F('^');
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Smooth;
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings_SVAVInput_Smooth) {
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

            OSD_menu_F('^');
        }
        OSD_menu_F('&');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('i'); //
                    oled_menuItem = OSD_SystemSettings_SVAVInput;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('^');
                    oled_menuItem = OSD_SystemSettings_SVAVInput_DoubleLine;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_menu_F('^');
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Bright;
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
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings_SVAVInput_Bright) {
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

            OSD_menu_F('^');
        }
        OSD_menu_F('&');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('i'); //
                    oled_menuItem = OSD_SystemSettings_SVAVInput;
                    saveUserPrefs();
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_menu_F('^');
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Smooth;
                    break;
                case IRKeyDown:

                    // selectedMenuLine = 3;
                    // OSD_menu_F('^');
                    // oled_menuItem = OSD_SystemSettings_SVAVInput_contrast;

                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('z');
                    oled_menuItem = OSD_SystemSettings_SVAVInput_contrast;
                    break;
                case IRKeyRight:
                    brightness = MIN(brightness + STEP, 254);
                    SetReg(0x0a, brightness - 128);
                    // printf("brightness: 0x%02x \n",brightness);
                    break;
                case IRKeyLeft:
                    brightness = MAX(brightness - STEP, 0);
                    SetReg(0x0a, brightness - 128);
                    // printf("brightness: 0x%02x \n",brightness);
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings_SVAVInput_contrast) {
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

            OSD_menu_F('z');
        }
        OSD_menu_F('A');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('i'); //
                    oled_menuItem = OSD_SystemSettings_SVAVInput;
                    break;
                case IRKeyUp:
                    OSD_menu_F(OSD_CROSS_BOTTOM);
                    OSD_menu_F('^');
                    oled_menuItem = OSD_SystemSettings_SVAVInput_Bright;
                    // selectedMenuLine = 2;
                    // OSD_menu_F('^');
                    // oled_menuItem = OSD_SystemSettings_SVAVInput_Bright;
                    break;

                case IRKeyDown:
                    // OSD_menu_F(OSD_CROSS_TOP);
                    // OSD_menu_F('z');
                    // oled_menuItem = OSD_SystemSettings_SVAVInput_saturation;

                    selectedMenuLine = 2;
                    OSD_menu_F('z');
                    oled_menuItem = OSD_SystemSettings_SVAVInput_saturation;

                    break;
                case IRKeyRight:
                    contrast = MIN(contrast + STEP, 254);
                    SetReg(0x08, contrast);
                    // printf("contrast: 0x%02x \n",contrast);
                    break;
                case IRKeyLeft:
                    contrast = MAX(contrast - STEP, 0);
                    SetReg(0x08, contrast);
                    // printf("contrast: 0x%02x \n",contrast);
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings_SVAVInput_saturation) {
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

            OSD_menu_F('z');
        }
        OSD_menu_F('A');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('i'); //
                    oled_menuItem = OSD_SystemSettings_SVAVInput;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('z');
                    oled_menuItem = OSD_SystemSettings_SVAVInput_contrast;

                    // OSD_menu_F(OSD_CROSS_BOTTOM);
                    // OSD_menu_F('^');
                    // oled_menuItem = OSD_SystemSettings_SVAVInput_contrast;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_menu_F('z');
                    oled_menuItem = OSD_SystemSettings_SVAVInput_default;
                    break;
                case IRKeyRight:
                    saturation = MIN(saturation + STEP, 254);
                    SetReg(0xe3, saturation);
                    // printf("saturation: 0x%02x \n",saturation);
                    break;
                case IRKeyLeft:
                    saturation = MAX(saturation - STEP, 0);
                    SetReg(0xe3, saturation);
                    // printf("saturation: 0x%02x \n",saturation);
                    break;
                case IRKeyOk:
                    saveUserPrefs();
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }


    else if (oled_menuItem == OSD_SystemSettings_SVAVInput_default) {
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

            OSD_menu_F('z');
        }
        OSD_menu_F('A');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('i'); //
                    oled_menuItem = OSD_SystemSettings_SVAVInput;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 2;
                    OSD_menu_F('z');
                    oled_menuItem = OSD_SystemSettings_SVAVInput_saturation;
                    break;
                case IRKeyOk:
                    SetReg('D', 'E');
                    brightness = 128;
                    contrast = 128;
                    saturation = 128;
                    saveUserPrefs();
                    break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_SystemSettings_SVAVInput_Compatibility) {
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

            OSD_menu_F('i');
        }
        OSD_menu_F('j');
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('2');
                    oled_menuItem = OSD_SystemSettings;
                    break;
                case IRKeyUp:
                    selectedMenuLine = 1;
                    OSD_menu_F('i'); //
                    oled_menuItem = OSD_SystemSettings_SVAVInput;
                    break;
                case IRKeyDown:
                    selectedMenuLine = 3;
                    OSD_menu_F('i'); //
                    oled_menuItem = OSD_SystemSettings_SV;
                    break;
                case IRKeyOk:
                    rgbComponentMode = !rgbComponentMode;
                    if (rgbComponentMode > 1)
                        rgbComponentMode = 0;
                    Send_Compatibility(rgbComponentMode);
                    if (GBS::ADC_INPUT_SEL::read())
                        UpDisplay();
                    break;
                // case IRKeyRight:
                //   rgbComponentMode = !rgbComponentMode;
                //   if (rgbComponentMode > 1)
                //     rgbComponentMode = 0;
                //   Send_Compatibility(rgbComponentMode);
                //   break;
                case IRKeyExit:
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('1');
                    oled_menuItem = OSD_Input;
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Main) {
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
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_SaveConfirm;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot19;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'A';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_SaveConfirm) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Save;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Main;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'B';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Save) // save
    {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Load;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_SaveConfirm;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'C';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Load) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Operation1;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Save;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'D';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Operation1) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Operation2;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Load;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'E';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Operation2) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Operation3;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Operation1;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'F';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Operation3) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot7;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Operation2;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'G';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_SelectSlot) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot1;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Preset12;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot1) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot2;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_SelectSlot;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot2) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot3;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot1;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot3) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot4;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot2;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot4) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot5;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot3;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot5) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot6;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot4;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot6) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_SelectPreset;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot5;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot7) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot8;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Operation3;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'H';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot8) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot9;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot7;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'I';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot9) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot10;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot8;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'J';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot10) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot11;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot9;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'K';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot11) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot12;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot10;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'L';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot12) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot13;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot11;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'M';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot13) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot14;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot12;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'N';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot14) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot15;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot13;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'O';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot15) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot16;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot14;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'P';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot16) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot17;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot15;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'Q';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot17) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot18;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot16;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'R';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot18) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Slot19;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot17;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'S';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Slot19) {
        if (results.value == IRKeyUp) {
            OSD_c1(icon4, P0, yellow);
            OSD_c2(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_menu_F('w');
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyDown:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    OSD_menu_F(OSD_CROSS_MID);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Main;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot18;
                    break;
                case IRKeyOk:
                    uopt->presetSlot = 'T';
                    OSD_menu_F('y');
                    break;
                case IRKeyExit:
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_SelectPreset) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Preset1;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Slot6;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Preset1) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Preset2;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_SelectPreset;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Preset2) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Preset3;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Preset1;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Preset3) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Preset4;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Preset2;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Preset4) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Preset5;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Preset3;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Preset5) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Preset6;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Preset4;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Preset6) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Preset7;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Preset5;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Preset7) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Preset8;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Preset6;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Preset8) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Preset9;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Preset7;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Preset9) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Preset10;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Preset8;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Preset10) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Preset11;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Preset9;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Preset11) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_Preset12;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Preset10;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Profile_Preset12) {
        if (results.value == IRKeyDown || results.value == IRKeyUp) {
            OSD_c1(icon4, P0, blue_fill);
            OSD_c3(icon4, P0, blue_fill);
            OSD_c2(icon4, P0, yellow);
        }

        OSD_menu_F('x');

        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
                    break;
                case IRKeyUp:
                    oled_menuItem = OSD_Profile_Main;
                    OSD_menu_F(OSD_CROSS_TOP);
                    OSD_menu_F('w');
                    break;
                case IRKeyRight:
                    oled_menuItem = OSD_Profile_SelectSlot;
                    break;
                case IRKeyLeft:
                    oled_menuItem = OSD_Profile_Preset11;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Volume_Adjust) {
        if (oledClearFlag) {
            display.clear();
        }
        oledClearFlag = ~0;
        display.setColor(OLEDDISPLAY_COLOR::WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(8, 15, "Volume - / + dB");
        display.display();

        osdDisplayValue = 50 - Volume;
        colour1 = yellowT;
        number_stroca = stroca1;
        Osd_Display(1, "Line input volume");
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
                    Volume = MAX(Volume - 1, 0);
                    osdDisplayValue = 50 - Volume;
                    PT_2257(Volume); // 0-50 maps to 0-50 dB (actually 0-39 dB usable)
                    break;
                case kRecv3: // --
                    Volume = MIN(Volume + 1, 50);
                    osdDisplayValue = 50 - Volume;
                    PT_2257(Volume); // 0-50 maps to 0-50 dB (actually 0-39 dB usable)
                    break;
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Resolution;
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
                    oled_menuItem = OSD_None;
                    OSD_clear();
                    OSD();
                    break;
            }
            irrecv.resume();
        }
    }

    else if (oled_menuItem == OSD_Info_Display) {
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
        Osd_Display(0, "Info:");
        colour1 = main0;
        Osd_Display(26, "Hz");

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

        if (Info == InfoRGBs) {
            // OSD_writeString(17,1," RGBs");
            OSD_c1(r, P17, blue_fill);
            OSD_c1(R, P18, main0);
            OSD_c1(G, P19, main0);
            OSD_c1(B, P20, main0);
            OSD_c1(s, P21, main0);
        } else if (Info == InfoRGsB) {
            // OSD_writeString(17,1," RGsB ");
            OSD_c1(r, P17, blue_fill);
            OSD_c1(R, P18, main0);
            OSD_c1(G, P19, main0);
            OSD_c1(s, P20, main0);
            OSD_c1(B, P21, main0);
            OSD_c1(B, P22, blue_fill);
        } else if (Info == InfoVGA) {
            // OSD_writeString(17,1," VGA  ");
            OSD_c1(r, P17, blue_fill);
            OSD_c1(V, P18, main0);
            OSD_c1(G, P19, main0);
            OSD_c1(A, P20, main0);
            OSD_c1(B, P21, blue_fill);
            OSD_c1(B, P22, blue_fill);
        } else if (Info == InfoYUV) {
            OSD_c1(r, P17, blue_fill);
            OSD_c1(Y, P18, main0);
            OSD_c1(P, P19, main0);
            OSD_c1(B, P20, main0);
            OSD_c1(P, P21, main0);
            OSD_c1(R, P22, main0);
        } else if (Info == InfoSV) {
            OSD_c1(r, P17, blue_fill);
            OSD_c1(Y, P18, blue_fill);
            OSD_c1(S, P19, main0);
            OSD_c1(V, P20, main0);
            OSD_c1(B, P21, blue_fill);
            OSD_c1(B, P22, blue_fill);
        } else if (Info == InfoAV) {
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

        Osd_Display(0, "Current:");

        colour1 = main0;
        number_stroca = stroca2;

        Osd_Display(0xFF, " ");
        // if (( rto->sourceDisconnected || !rto->boardHasPower || infoState == 1) && rto->HdmiHoldDetection)
        if ((rto->sourceDisconnected || !rto->boardHasPower || infoState == 1)) {
            Osd_Display(0xFF, "No Input");
        } else if (((currentInput == 1) || (Info == InfoRGBs || Info == InfoRGsB || Info == InfoVGA))) {
            OSD_c2(B, P16, blue_fill);
            Osd_Display(0xFF, "RGB ");
            vsyncActive = GBS::STATUS_SYNC_PROC_VSACT::read();
            if (vsyncActive) {
                // Osd_Display(0xFF,"H");
                hsyncActive = GBS::STATUS_SYNC_PROC_HSACT::read();
                if (hsyncActive) {
                    Osd_Display(0xFF, "HV   ");
                }
            } else if ((Info == InfoVGA) && ((!vsyncActive || !hsyncActive))) {
                OSD_c2(B, P11, blue_fill);
                Osd_Display(0x09, "No Input");
            }
        } else if ((rto->continousStableCounter > 35 || currentInput != 1) || (Info == InfoYUV || Info == InfoSV || Info == InfoAV)) {
            OSD_c2(B, P16, blue_fill);
            if (Info == InfoYUV)
                Osd_Display(0xFF, "  YPBPR  ");
            else if (Info == InfoSV)
                Osd_Display(0xFF, "   SV    ");
            else if (Info == InfoAV)
                Osd_Display(0xFF, "   AV    ");
        } else {
            Osd_Display(0xFF, "No Input");
        }
#if 1
        static uint8_t S0_Read_Resolution;
        static unsigned long Tim_info = 0;
        if ((millis() - Tim_info) >= 1000) {
            S0_Read_Resolution = GBS::REG_S0_00::read();

            // GBS::IF_LD_RAM_BYPS::write(1);
            // printf( "Scanning method: %d\n",GBS::STATUS_SYNC_PROC_VTOTAL::read() );   // 0x%02x
            // printf( "Scanning method: %d\n",GBS::STATUS_VDS_VERT_COUNT::read() );
            // printf( "H_TOTAL: %d      ",(GBS::H_TOTAL_HIGH::read() << 8) + GBS::H_TOTAL_LOW::read() *4  );

            // printf( "V_TOTAL: %d\n",(GBS::V_TOTAL_HIGH::read() << 7) + GBS::V_TOTAL_LOW::read() );

            Tim_info = millis();
        }

        if (S0_Read_Resolution & 0x80) {
            if (S0_Read_Resolution & 0x40) {
                Osd_Display(0xFF, "   576p");
            } else if (S0_Read_Resolution & 0x20) {
                if (abs(GBS::STATUS_SYNC_PROC_VTOTAL::read() - 312) <= 10)
                    Osd_Display(0xFF, "   288p");
                else
                    Osd_Display(0xFF, "   576i");
            } else if (S0_Read_Resolution & 0x10) {
                Osd_Display(0xFF, "   480p");
            } else if (S0_Read_Resolution & 0x08) {
                if (abs(GBS::STATUS_SYNC_PROC_VTOTAL::read() - 262) <= 10)
                    Osd_Display(0xFF, "   240p");
                else
                    Osd_Display(0xFF, "   480i");
            } else
                Osd_Display(0xFF, "   Err");
        } else
            Osd_Display(0xFF, "   Err");

#endif
        if (irrecv.decode(&results)) {
            irDecodedFlag = 1;
            switch (results.value) {
                case IRKeyMenu:
                    selectedMenuLine = 1;
                    OSD_menu_F('0');
                    oled_menuItem = OSD_Input;
                    break;
                case IRKeyExit:
                    if (infoState) {
                        GBS::VDS_DIS_HB_ST::write(horizontalBlankStart);
                        GBS::VDS_DIS_HB_SP::write(horizontalBlankStop);
                        infoState = 0;
                    }
                    // infoState = 0;
                    oled_menuItem = OSD_None;
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
        OledUpdataTime = 1;
    }

    if ((millis() - lastResolutionTime) >= OSD_RESOLUTION_UP_TIME && oled_menuItem == OSD_Resolution_RetainedSettings) {
        lastMenuItemTime = millis(); // updata osd close
        lastResolutionTime = millis();
        // printf(" status:%02x  \n",GBS::PAD_CONTROL_01_0x49::read());
        // printf(" %02x \n",GBS::STATUS_MISC::read());
        uint8_t T_tim = OSD_RESOLUTION_CLOSE_TIME / 1000 - ((lastResolutionTime - resolutionStartTime) / 1000);
        // colour1 = A2_main0;
        number_stroca = stroca2;
        if (T_tim >= 10) {
            OSD_c2((T_tim / 10) + '0', P11, main0);
            OSD_c2((T_tim % 10) + '0', P12, main0);

            Osd_Display(14, " s ");
        } else {
            OSD_c2('0', P12, blue_fill);
            OSD_c2(T_tim + '0', P11, main0); //

            Osd_Display(13, " s ");
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

            OSD_menu_F(OSD_CROSS_MID);
            OSD_menu_F('4');
            oled_menuItem = OSD_Resolution_pass;
        }
    }

    if (lastOledMenuItem != oled_menuItem && oled_menuItem != 0) {
        lastMenuItemTime = millis();
        oledClearFlag = 1;
        // printf("freq:%d \n", system_get_cpu_freq());
        // printf("oled_menuItem:%d \n", oled_menuItem);
        // printf("infoState:%d \n", infoState);
    }
    if ((millis() - lastMenuItemTime) >= OSD_CLOSE_TIME && oled_menuItem != 0) {
        // 菜单关闭
        if (infoState) {
            GBS::VDS_DIS_HB_ST::write(horizontalBlankStart);
            GBS::VDS_DIS_HB_SP::write(horizontalBlankStop);
            infoState = 0;
        }
        oled_menuItem = OSD_None;
        lastOledMenuItem = 0;
        OSD_clear();
        OSD();
    }
    lastOledMenuItem = oled_menuItem;
}

void OSD_menu_F(char incomingByte)
{
    const size_t tableSize = sizeof(menuTable) / sizeof(menuTable[0]);
    const unsigned char key = (unsigned char)incomingByte;

    // 线性查找实�?
    for (size_t i = 0; i < tableSize; i++) {
        if (menuTable[i].key == key) {
            menuTable[i].handler();
            return;
        }
    }


    // OSD_default_F();
}

void OSD_IR()
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

                oled_menuItem = OSD_Info_Display;

                // InputINFO();
                //////////new
                infoState = 1;
                horizontalBlankStart = GBS::VDS_DIS_HB_ST::read();
                horizontalBlankStop = GBS::VDS_DIS_HB_SP::read();

                /////////new
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
                OSD_menu_F('0');
                oled_menuItem = OSD_Input;
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
        //     oled_menuItem = OSD_None;
        //     PT_MUTE(0x79);
        // }

        if (results.value == IRKeySave) {
            lastMenuItemTime = millis();
            NEW_OLED_MENU = false;
            OSD_menu_F(OSD_CROSS_TOP);
            OSD_menu_F('w');
            oled_menuItem = OSD_Profile_Main;
        }

        if (results.value == IRKeyInfo) {
            lastMenuItemTime = millis();
            NEW_OLED_MENU = false;
            background_up(stroca1, _27, blue_fill);
            background_up(stroca2, _27, blue_fill);
            oled_menuItem = OSD_Info_Display;
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
                    oled_menuItem = OSD_None;
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
                    oled_menuItem = OSD_None;
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
                oled_menuItem = OSD_Volume_Adjust;
                break;
            case kRecv3:
                lastMenuItemTime = millis();
                NEW_OLED_MENU = false;
                background_up(stroca1, _25, blue_fill);
                oled_menuItem = OSD_Volume_Adjust;
                break;
        }

        irrecv.resume();
        delay(5);
    }
}


void handle_0(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    // OSD_c2(0x15, P9 , blue_fill);
    // OSD_c3(0x15, P18, blue_fill);

    OSD_background();
    colour1 = blue_fill;
    number_stroca = stroca2;
    __(icon4, _0);
    number_stroca = stroca3;
    __(icon4, _0);
    colour1 = yellow;
    number_stroca = stroca1;
    __(icon4, _0);

    colour1 = blue;

    // number_stroca = stroca1;
    // __(icon5, _27);
    number_stroca = stroca2;
    __('1', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "1 Input");
    OSD_c1(0x15, P8, yellowT);

    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "2 Output Resolution");

    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "3 Screen Settings");
};
void handle_1(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;

        OSD_c1(0x15, P8, yellowT);
        OSD_c2(0x15, P20, blue_fill);
        OSD_c3(0x15, P18, blue_fill);
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;

        OSD_c1(0x15, P8, blue_fill);
        OSD_c2(0x15, P20, yellowT);
        OSD_c3(0x15, P18, blue_fill);
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;

        OSD_c1(0x15, P8, blue_fill);
        OSD_c2(0x15, P20, blue_fill);
        OSD_c3(0x15, P18, yellowT);
    }

    colour1 = blue;
    // number_stroca = stroca1;
    // __(icon5, _27);
    number_stroca = stroca2;
    __('1', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "1 Input");

    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "2 Output Resolution"); //__(0X15, _9);

    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "3 Screen Settings"); //__(0X15, _18);
};
void handle_2(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;

        OSD_c1(0x15, P18, yellowT);
        OSD_c2(0x15, P19, blue_fill);
        // OSD_c3(0x15, P15 , blue_fill  );
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;

        OSD_c1(0x15, P18, blue_fill);
        OSD_c2(0x15, P19, yellowT);
        // OSD_c3(0x15, P15 , blue_fill  );
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;

        OSD_c1(0x15, P18, blue_fill);
        OSD_c2(0x15, P19, blue_fill);
        // OSD_c3(0x15, P15 , blue_fill  );
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('2', _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "4 System Settings");
    colour1 = A2_main0;
    number_stroca = stroca2;
    // Osd_Display(1, "5 Color Settings");
    Osd_Display(1, "5 Picture Settings");
    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "6 Reset Settings");
};
void handle_3(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    // number_stroca = stroca1;
    // __(icon5, _27);
    number_stroca = stroca2;
    __('1', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "1920x1080");
    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "1280x1024");
    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "1280x960");
};
void handle_4(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    }

    colour1 = blue;

    number_stroca = stroca1;
    __(icon5, _27);

    number_stroca = stroca2;
    __('2', _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "1280x720");

    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "480p/576p");
    // colour1 = A3_main0;
    // number_stroca = stroca3;
    // __(D, _1), __(o, _2), __(w, _3), __(n, _4), __(s, _5), __(c, _6), __(a, _7), __(l, _8), __(e, _9), __(n1, _11), __(n5, _12), __(K, _13), __(H, _14), __(z, _15);
};
void handle_5(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __(I, _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "Pass through");
};
void handle_6(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;

    // number_stroca = stroca1;
    // __(icon5, _27);
    number_stroca = stroca2;
    __('1', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "Move");
    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "Scale");
    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "Borders");
};
void handle_7(void)
{
    OSD_background();
    OSD_c1(icon4, P0, yellow);
    OSD_c2(icon4, P0, blue_fill);
    OSD_c3(icon4, P0, blue_fill);
    selectedMenuLine = 1;
};
void handle_8(void)
{
    OSD_background();
    OSD_c1(icon4, P0, blue_fill);
    OSD_c2(icon4, P0, yellow);
    OSD_c3(icon4, P0, blue_fill);
    selectedMenuLine = 2;
};
void handle_9(void)
{
    OSD_background();
    OSD_c1(icon4, P0, blue_fill);
    OSD_c2(icon4, P0, blue_fill);
    OSD_c3(icon4, P0, yellow);
    selectedMenuLine = 3;
};
void handle_a(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;

    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('2', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "ADC gain");
    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "Scanlines");
    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "Line filter");
};
void handle_b(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('3', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "Sharpness");
    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "Peaking");
    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "Step response");
};
void handle_c(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('4', _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "Default Color");
    // Osd_Display(1, "Y gain");
    // colour1 = A2_main0;
    // number_stroca = stroca2;
    // Osd_Display(1, "Color");
    // colour1 = A3_main0;
    // number_stroca = stroca3;
};
void handle_d(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    // number_stroca = stroca1;
    // __(icon5, _27);
    number_stroca = stroca2;
    __('1', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "R");
    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "G");
    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "B");
};
void handle_e(void)
{
    OSD_c1(0x3E, P9, main0);
    OSD_c1(0x3E, P10, main0);
    OSD_c1(0x3E, P11, main0);
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P22, main0);

    OSD_c2(0x3E, P10, main0);
    OSD_c2(0x3E, P11, main0);
    OSD_c2(0x3E, P12, main0);
    OSD_c2(0x3E, P13, main0);
    OSD_c2(0x3E, P14, main0);
    OSD_c2(0x3E, P15, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P22, main0);
    OSD_c3(0x3E, P12, main0);
    OSD_c3(0x3E, P13, main0);
    OSD_c3(0x3E, P14, main0);
    OSD_c3(0x3E, P15, main0);
    OSD_c3(0x3E, P16, main0);
    OSD_c3(0x3E, P17, main0);
    OSD_c3(0x3E, P18, main0);
    OSD_c3(0x3E, P19, main0);
    OSD_c3(0x3E, P20, main0);
    OSD_c3(0x3E, P21, main0);
    OSD_c3(0x3E, P22, main0);

    if (uopt->wantScanlines) {
        OSD_c2(O, P23, main0);
        OSD_c2(N, P24, main0);
        OSD_c2(F, P25, blue_fill);
    } else {
        OSD_c2(O, P23, main0);
        OSD_c2(F, P24, main0);
        OSD_c2(F, P25, main0);
    }

    if (uopt->wantVdsLineFilter) {
        OSD_c3(O, P23, main0);
        OSD_c3(N, P24, main0);
        OSD_c3(F, P25, blue_fill);
    } else {
        OSD_c3(O, P23, main0);
        OSD_c3(F, P24, main0);
        OSD_c3(F, P25, main0);
    }
    osdDisplayValue = GBS::ADC_RGCTRL::read();
    Type4(osdDisplayValue);

    osdDisplayValue = uopt->scanlineStrength;
    if (osdDisplayValue == 0x00) {
        OSD_c2(n0, P21, main0);
        OSD_c2(n0, P20, main0);
    } else if (osdDisplayValue == 0x10) {
        OSD_c2(n0, P21, main0);
        OSD_c2(n1, P20, main0);
    } else if (osdDisplayValue == 0x20) {
        OSD_c2(n0, P21, main0);
        OSD_c2(n2, P20, main0);
    } else if (osdDisplayValue == 0x30) {
        OSD_c2(n0, P21, main0);
        OSD_c2(n3, P20, main0);
    } else if (osdDisplayValue == 0x40) {
        OSD_c2(n0, P21, main0);
        OSD_c2(n4, P20, main0);
    } else if (osdDisplayValue == 0x50) {
        OSD_c2(n0, P21, main0);
        OSD_c2(n5, P20, main0);
    }

    if (uopt->enableAutoGain == 0) {
        OSD_c1(O, P23, main0);
        OSD_c1(F, P24, main0);
        OSD_c1(F, P25, main0);
    } else {
        OSD_c1(O, P23, main0);
        OSD_c1(N, P24, main0);
        OSD_c1(0x3E, P25, blue_fill);
    }
};
void handle_f(void)
{
    OSD_c1(0x3E, P10, main0);
    OSD_c1(0x3E, P11, main0);
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);
    OSD_c2(0x3E, P8, main0);
    OSD_c2(0x3E, P9, main0);
    OSD_c2(0x3E, P10, main0);
    OSD_c2(0x3E, P11, main0);
    OSD_c2(0x3E, P12, main0);
    OSD_c2(0x3E, P13, main0);
    OSD_c2(0x3E, P14, main0);
    OSD_c2(0x3E, P15, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P20, main0);
    OSD_c2(0x3E, P21, main0);
    OSD_c2(0x3E, P22, main0);
    OSD_c3(0x3E, P14, main0);
    OSD_c3(0x3E, P15, main0);
    OSD_c3(0x3E, P16, main0);
    OSD_c3(0x3E, P17, main0);
    OSD_c3(0x3E, P18, main0);
    OSD_c3(0x3E, P19, main0);
    OSD_c3(0x3E, P20, main0);
    OSD_c3(0x3E, P21, main0);
    OSD_c3(0x3E, P22, main0);

    if (GBS::VDS_PK_LB_GAIN::read() == 0x16) {
        OSD_c1(O, P23, main0);
        OSD_c1(F, P24, main0);
        OSD_c1(F, P25, main0);
    } else {
        OSD_c1(O, P23, main0);
        OSD_c1(N, P24, main0);
        OSD_c1(F, P25, blue_fill);
    }

    if (uopt->wantPeaking == 0) {
        OSD_c2(O, P23, main0);
        OSD_c2(F, P24, main0);
        OSD_c2(F, P25, main0);
    } else {
        OSD_c2(O, P23, main0);
        OSD_c2(N, P24, main0);
        OSD_c2(F, P25, blue_fill);
    }

    if (uopt->wantStepResponse) {
        OSD_c3(O, P23, main0);
        OSD_c3(N, P24, main0);
        OSD_c3(F, P25, blue_fill);
    } else {
        OSD_c3(O, P23, main0);
        OSD_c3(F, P24, main0);
        OSD_c3(F, P25, main0);
    }
};
void handle_g(void)
{ // OSD_c1(0x3E, P2, main0);
    // OSD_c1(0x3E, P3, main0);
    // OSD_c1(0x3E, P4, main0);
    OSD_c1(0x3E, P5, main0);
    OSD_c1(0x3E, P6, main0);
    OSD_c1(0x3E, P7, main0);
    OSD_c1(0x3E, P8, main0);
    OSD_c1(0x3E, P9, main0);
    OSD_c1(0x3E, P10, main0);
    OSD_c1(0x3E, P11, main0);
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);

    // OSD_c2(0x3E, P2, main0);
    // OSD_c2(0x3E, P3, main0);
    // OSD_c2(0x3E, P4, main0);
    OSD_c2(0x3E, P5, main0);
    OSD_c2(0x3E, P6, main0);
    OSD_c2(0x3E, P7, main0);
    OSD_c2(0x3E, P8, main0);
    OSD_c2(0x3E, P9, main0);
    OSD_c2(0x3E, P10, main0);
    OSD_c2(0x3E, P11, main0);
    OSD_c2(0x3E, P12, main0);
    OSD_c2(0x3E, P13, main0);
    OSD_c2(0x3E, P14, main0);
    OSD_c2(0x3E, P15, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P20, main0);
    OSD_c2(0x3E, P21, main0);
    OSD_c2(0x3E, P22, main0);

    // OSD_c3(0x3E, P2, main0);
    // OSD_c3(0x3E, P3, main0);
    // OSD_c3(0x3E, P4, main0);
    OSD_c3(0x3E, P5, main0);
    OSD_c3(0x3E, P6, main0);
    OSD_c3(0x3E, P7, main0);
    OSD_c3(0x3E, P8, main0);
    OSD_c3(0x3E, P9, main0);
    OSD_c3(0x3E, P10, main0);
    OSD_c3(0x3E, P11, main0);
    OSD_c3(0x3E, P12, main0);
    OSD_c3(0x3E, P13, main0);
    OSD_c3(0x3E, P14, main0);
    OSD_c3(0x3E, P15, main0);
    OSD_c3(0x3E, P16, main0);
    OSD_c3(0x3E, P17, main0);
    OSD_c3(0x3E, P18, main0);
    OSD_c3(0x3E, P19, main0);
    OSD_c3(0x3E, P20, main0);
    OSD_c3(0x3E, P21, main0);
    OSD_c3(0x3E, P22, main0);

    // osdDisplayValue = (128 + GBS::VDS_Y_OFST::read());  //R
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() + 1.402 * ((signed char)GBS::VDS_V_OFST::read()-128));  //R
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() + 1.5 * ((signed char)GBS::VDS_V_OFST::read()));  //R
    // osdDisplayValue= (signed char)GBS::VDS_Y_OFST::read()+1.402*((signed char)GBS::VDS_V_OFST::read()-128);
    // osdDisplayValue = R_VAL;
    colour1 = main0;
    number_stroca = stroca1;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    // Typ(((signed char)((signed char)GBS::VDS_Y_OFST::read()) +(float)( 1.402     * (signed char)((signed char)GBS::VDS_V_OFST::read()) )) + 128);
    Typ(R_VAL);
    // osdDisplayValue = (128 + GBS::VDS_U_OFST::read());  //G
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() - 0.88 * ((signed char)GBS::VDS_U_OFST::read()) - 0.764 * ((signed char)GBS::VDS_V_OFST::read()));  //G
    // osdDisplayValue = (signed char)GBS::VDS_Y_OFST::read()-0.344136*((signed char)GBS::VDS_U_OFST::read()-128)-0.714136*((signed char)GBS::VDS_V_OFST::read()-128);
    // osdDisplayValue = G_VAL;
    colour1 = main0;
    number_stroca = stroca2;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    // Typ(((signed char)((signed char)GBS::VDS_Y_OFST::read()) -(float)( 0.344136  * (signed char)((signed char)GBS::VDS_U_OFST::read()) )- 0.714136 * (signed char)((signed char)GBS::VDS_V_OFST::read()) ) + 128);
    Typ(G_VAL);

    // osdDisplayValue = (128 + GBS::VDS_V_OFST::read());  //B
    // osdDisplayValue = ((signed char)GBS::VDS_Y_OFST::read() + 2 * ((signed char)GBS::VDS_U_OFST::read()));  //B
    // osdDisplayValue = (signed char)GBS::VDS_Y_OFST::read()+1.772*((signed char)GBS::VDS_U_OFST::read()-128);
    // osdDisplayValue = B_VAL;
    colour1 = main0;
    number_stroca = stroca3;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    // Typ(((signed char)((signed char)GBS::VDS_Y_OFST::read()) +(float)( 1.772     * (signed char)((signed char)GBS::VDS_U_OFST::read()) )) + 128);
    Typ(B_VAL);
};
void handle_h(void)
{
    OSD_c1(0x3E, P7, main0);
    OSD_c1(0x3E, P8, main0);
    OSD_c1(0x3E, P9, main0);
    OSD_c1(0x3E, P10, main0);
    OSD_c1(0x3E, P11, main0);
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);
    OSD_c2(0x3E, P6, main0);
    OSD_c2(0x3E, P7, main0);
    OSD_c2(0x3E, P8, main0);
    OSD_c2(0x3E, P9, main0);
    OSD_c2(0x3E, P10, main0);
    OSD_c2(0x3E, P11, main0);
    OSD_c2(0x3E, P12, main0);
    OSD_c2(0x3E, P13, main0);
    OSD_c2(0x3E, P14, main0);
    OSD_c2(0x3E, P15, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P20, main0);
    OSD_c2(0x3E, P21, main0);
    OSD_c2(0x3E, P22, main0);

    osdDisplayValue = GBS::VDS_Y_GAIN::read();
    colour1 = main0;
    number_stroca = stroca1;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    Typ(osdDisplayValue);
    osdDisplayValue = GBS::VDS_VCOS_GAIN::read();
    colour1 = main0;
    number_stroca = stroca2;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    Typ(osdDisplayValue);
};
void handle_i(void)
{
    if (selectedMenuLine == 1) {
        if ((Info != InfoSV) && (Info != InfoAV)) {
            A1_yellow = 0X14;
        } else {
            A1_yellow = yellowT;
        }

        A2_main0 = main0;
        A3_main0 = main0;

        OSD_c1(0x15, P21, yellowT);
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;

        OSD_c1(0x15, P21, blue_fill);
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;

        OSD_c1(0x15, P21, blue_fill);
    }

    colour1 = blue;
    // number_stroca = stroca1;
    // __(icon5, _27);
    number_stroca = stroca2;
    __('1', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "SV/AV Input Settings");

    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "Compatibility Mode");

    colour1 = A3_main0;
    number_stroca = stroca3;
    // Osd_Display(1, "Lowres:use upscaling");
    Osd_Display(1, "Matched presets");
};
void handle_j(void)
{
    // OSD_c2(0x3E, P16, main0);
    // OSD_c2(0x3E, P17, main0);
    // OSD_c2(0x3E, P18, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P20, main0);
    OSD_c2(0x3E, P21, main0);
    OSD_c2(0x3E, P22, main0);
    if (rgbComponentMode == 1) {
        OSD_c2(O, P23, main0);
        OSD_c2(N, P24, main0);
        OSD_c2(F, P25, blue_fill);
    } else {
        OSD_c2(O, P23, main0);
        OSD_c2(F, P24, main0);
        OSD_c2(F, P25, main0);
    }
    OSD_c3(0x3E, P16, main0);
    OSD_c3(0x3E, P17, main0);
    OSD_c3(0x3E, P18, main0);
    OSD_c3(0x3E, P19, main0);
    OSD_c3(0x3E, P20, main0);
    OSD_c3(0x3E, P21, main0);
    OSD_c3(0x3E, P22, main0);
    if (uopt->matchPresetSource) {
        OSD_c3(O, P23, main0);
        OSD_c3(N, P24, main0);     // ON
        OSD_c3(F, P25, blue_fill); // ON
    } else {
        OSD_c3(O, P23, main0);
        OSD_c3(F, P24, main0);
        OSD_c3(F, P25, main0); // OFF
    }
    /*
    upscaling
        // OSD_c3(0x3E, P21, main0);
        // OSD_c3(0x3E, P22, main0);
        // if (uopt->preferScalingRgbhv)
        // {
        //   OSD_c3(O, P23, main0);
        //   OSD_c3(N, P24, main0);
        //   OSD_c3(F, P25, blue_fill);
        // }
        // else
        // {
        //   OSD_c3(O, P23, main0);
        //   OSD_c3(F, P24, main0);
        //   OSD_c3(F, P25, main0);
        // }
    */
};
void handle_k(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('2', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "Deinterlace");
    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "Force:50Hz to 60Hz");
    colour1 = A3_main0;
    number_stroca = stroca3;
    // Osd_Display(1, "Clock generator");
    Osd_Display(1, "Lock method");
};
void handle_l(void)
{
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    if (uopt->deintMode == 0) {
        OSD_c1(A, P18, main0);
        OSD_c1(d, P19, main0);
        OSD_c1(a, P20, main0);
        OSD_c1(p, P21, main0);
        OSD_c1(t, P22, main0);
        OSD_c1(i, P23, main0);
        OSD_c1(v, P24, main0);
        OSD_c1(e, P25, main0);
    } else {
        OSD_c1(0x3E, P18, main0);
        OSD_c1(0x3E, P19, main0);
        OSD_c1(0x3E, P20, main0);
        OSD_c1(0x3E, P21, main0);
        OSD_c1(0x3E, P22, main0);
        OSD_c1(B, P23, main0);
        OSD_c1(o, P24, main0);
        OSD_c1(b, P25, main0);
    }

    // OSD_c1(0x3E, P21, main0);
    // OSD_c1(0x3E, P22, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P20, main0);
    OSD_c2(0x3E, P21, main0);
    OSD_c2(0x3E, P22, main0);
    // OSD_c3(0x3E, P16, main0);
    // OSD_c3(0x3E, P17, main0);
    // OSD_c3(0x3E, P18, main0);
    // OSD_c3(0x3E, P19, main0);
    // OSD_c3(0x3E, P20, main0);
    // OSD_c3(0x3E, P21, main0);
    // OSD_c3(0x3E, P22, main0);
    OSD_c3(0x3E, P12, main0);
    OSD_c3(0x3E, P13, main0);

    // if (uopt->wantOutputComponent)
    // {
    //     OSD_c1(O, P23, main0);
    //     OSD_c1(N, P24, main0);
    //     OSD_c1(F, P25, blue_fill);
    // }
    // else
    // {
    //     OSD_c1(O, P23, main0);
    //     OSD_c1(F, P24, main0);
    //     OSD_c1(F, P25, main0);
    // }

    if (uopt->PalForce60) {
        OSD_c2(O, P23, main0);
        OSD_c2(N, P24, main0);
        OSD_c2(F, P25, blue_fill);
    } else {
        OSD_c2(O, P23, main0);
        OSD_c2(F, P24, main0);
        OSD_c2(F, P25, main0);
    }

    // if (uopt->disableExternalClockGenerator)
    // {
    //   OSD_c3(O, P23, main0);
    //   OSD_c3(F, P24, main0);
    //   OSD_c3(F, P25, main0);
    // }
    // else
    // {
    //   OSD_c3(O, P23, main0);
    //   OSD_c3(N, P24, main0);
    //   OSD_c3(F, P25, blue_fill);
    // }

    if (uopt->frameTimeLockMethod == 0) {
        OSD_c3(n0, P14, main0);
        OSD_c3(V, P15, main0);
        OSD_c3(t, P16, main0);
        OSD_c3(o, P17, main0);
        OSD_c3(t, P18, main0);
        OSD_c3(a, P19, main0);
        OSD_c3(l, P20, main0);
        OSD_c3(0x3C, P21, main0);
        OSD_c3(V, P22, main0);
        OSD_c3(S, P23, main0);
        OSD_c3(S, P24, main0);
        OSD_c3(T, P25, main0);
    } else {
        OSD_c3(n1, P14, main0);
        OSD_c3(V, P15, main0);
        OSD_c3(t, P16, main0);
        OSD_c3(o, P17, main0);
        OSD_c3(t, P18, main0);
        OSD_c3(a, P19, main0);
        OSD_c3(l, P20, main0);
        OSD_c3(o, P22, main0);
        OSD_c3(n, P23, main0);
        OSD_c3(l, P24, main0);
        OSD_c3(y, P25, main0);
        OSD_c3(F, P21, blue_fill);
    }
};
void handle_m(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('3', _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "ADC calibration");
    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "Frame Time lock");
    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "EnableFrameTimeLock");
};
void handle_n(void)
{
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P20, main0);
    OSD_c2(0x3E, P21, main0);
    OSD_c2(0x3E, P22, main0);
    // OSD_c3(0x3E, P16, main0);
    // OSD_c3(0x3E, P17, main0);
    // OSD_c3(0x3E, P18, main0);
    // OSD_c3(0x3E, P19, main0);
    OSD_c3(0x3E, P20, main0);
    OSD_c3(0x3E, P21, main0);
    OSD_c3(0x3E, P22, main0);

    if (uopt->enableCalibrationADC) {
        OSD_c1(O, P23, main0);
        OSD_c1(N, P24, main0);
        OSD_c1(F, P25, blue_fill);
    } else {
        OSD_c1(O, P23, main0);
        OSD_c1(F, P24, main0);
        OSD_c1(F, P25, main0);
    }

    if (uopt->enableFrameTimeLock) {
        OSD_c2(O, P23, main0);
        OSD_c2(N, P24, main0);
        OSD_c2(F, P25, blue_fill);
    } else {
        OSD_c2(O, P23, main0);
        OSD_c2(F, P24, main0);
        OSD_c2(F, P25, main0);
    }

    if (uopt->disableExternalClockGenerator) {
        OSD_c3(O, P23, main0);
        OSD_c3(F, P24, main0);
        OSD_c3(F, P25, main0);
    } else {
        OSD_c3(O, P23, main0);
        OSD_c3(N, P24, main0);
        OSD_c3(F, P25, blue_fill);
    }
};
void handle_o(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('2', _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "Full height");

    // colour1 = A2_main0;
    // number_stroca = stroca2;
    // __(M, _1), __(a, _2), __(t, _3), __(c, _4), __(h, _5), __(e, _6), __(d, _7), __(p, _9), __(r, _10), __(e, _11), __(s, _12), __(e, _13), __(t, _14), __(s, _15);
};
void handle_p(void)
{
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);
    // OSD_c3(0x3E, P22, main0);

    if (uopt->wantFullHeight) {
        OSD_c1(O, P23, main0);
        OSD_c1(N, P24, main0);
        OSD_c1(F, P25, blue_fill);
    } else {
        OSD_c1(O, P23, main0);
        OSD_c1(F, P24, main0);
        OSD_c1(F, P25, main0);
    }
};
void handle_q(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __(I, _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "MEM left/right");
    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "HS left/right");
    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "HTotal");
};
void handle_r(void)
{
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);
    OSD_c1(0x03, P23, yellow);
    OSD_c1(0x13, P24, yellow);
    OSD_c2(0x3E, P14, main0);
    OSD_c2(0x3E, P15, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P20, main0);
    OSD_c2(0x3E, P21, main0);
    OSD_c2(0x3E, P22, main0);
    OSD_c2(0x03, P23, yellow);
    OSD_c2(0x13, P24, yellow);
    OSD_c3(0x3E, P7, main0);
    OSD_c3(0x3E, P8, main0);
    OSD_c3(0x3E, P9, main0);
    OSD_c3(0x3E, P10, main0);
    OSD_c3(0x3E, P11, main0);
    OSD_c3(0x3E, P12, main0);
    OSD_c3(0x3E, P13, main0);
    OSD_c3(0x3E, P14, main0);
    OSD_c3(0x3E, P15, main0);
    OSD_c3(0x3E, P16, main0);
    OSD_c3(0x3E, P17, main0);
    OSD_c3(0x3E, P18, main0);
    OSD_c3(0x3E, P19, main0);
    OSD_c3(0x3E, P20, main0);
    OSD_c3(0x3E, P21, main0);
    OSD_c3(0x3E, P22, main0);
    osdDisplayValue = GBS::VDS_HSYNC_RST::read();
    colour1 = main0;
    number_stroca = stroca3;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    Typ(osdDisplayValue);
};
void handle_s(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __(I, _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "Debug view");
    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "ADC filter");
    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "Freeze capture");
};
void handle_t(void)
{
    OSD_c1(0x3E, P11, main0);
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);
    OSD_c2(0x3E, P11, main0);
    OSD_c2(0x3E, P12, main0);
    OSD_c2(0x3E, P13, main0);
    OSD_c2(0x3E, P14, main0);
    OSD_c2(0x3E, P15, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);
    OSD_c2(0x3E, P19, main0);
    OSD_c2(0x3E, P20, main0);
    OSD_c2(0x3E, P21, main0);
    OSD_c2(0x3E, P22, main0);
    colour1 = main0;
    number_stroca = stroca3;
    __(0x3E, _15), __(0x3E, _16), __(0x3E, _17), __(0x3E, _18), __(0x3E, _19), __(0x3E, _20), __(0x3E, _21), __(0x3E, _22);

    if (GBS::ADC_UNUSED_62::read() == 0x00) {
        OSD_c1(O, P23, main0);
        OSD_c1(F, P24, main0);
        OSD_c1(F, P25, main0);
    } else {
        OSD_c1(O, P23, main0);
        OSD_c1(N, P24, main0);
        OSD_c1(F, P25, blue_fill);
    }

    if (GBS::ADC_FLTR::read() > 0) {
        OSD_c2(O, P23, main0);
        OSD_c2(N, P24, main0);
        OSD_c2(F, P25, blue_fill);
    } else {
        OSD_c2(O, P23, main0);
        OSD_c2(F, P24, main0);
        OSD_c2(F, P25, main0);
    }

    if (GBS::CAPTURE_ENABLE::read() > 0) {
        OSD_c3(O, P23, main0);
        OSD_c3(F, P24, main0);
        OSD_c3(F, P25, main0);
    } else {
        OSD_c3(O, P23, main0);
        OSD_c3(N, P24, main0);
        OSD_c3(F, P25, blue_fill);
    }
};
void handle_u(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __(I, _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "Enable OTA");
    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "Restart");
    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "Reset defaults");
};
void handle_v(void)
{
    OSD_c1(0x3E, P11, main0);
    OSD_c1(0x3E, P12, main0);
    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);
    OSD_c1(0x3E, P19, main0);
    OSD_c1(0x3E, P20, main0);
    OSD_c1(0x3E, P21, main0);
    OSD_c1(0x3E, P22, main0);

    if (rto->allowUpdatesOTA) {
        OSD_c1(O, P23, main0);
        OSD_c1(N, P24, main0);
        OSD_c1(F, P25, blue_fill);
    } else {
        OSD_c1(O, P23, main0);
        OSD_c1(F, P24, main0);
        OSD_c1(F, P25, main0);
    }
};
void handle_w(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    }
    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "Loadprofile:");

    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "Saveprofile:");

    colour1 = yellowT;
    number_stroca = stroca3;
    Osd_Display(1, "Active save:");
};
void handle_x(void)
{
    if (oled_menuItem == OSD_Profile_Main) {
        colour1 = main0;
        number_stroca = stroca1;
        sending1();
        nameP();
    } else if (oled_menuItem == OSD_Profile_SaveConfirm) {
        colour1 = main0;
        number_stroca = stroca1;
        sending2();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Save) {
        colour1 = main0;
        number_stroca = stroca1;
        sending3();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Load) {
        colour1 = main0;
        number_stroca = stroca1;
        sending4();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Operation1) {
        colour1 = main0;
        number_stroca = stroca1;
        sending5();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Operation2) {
        colour1 = main0;
        number_stroca = stroca1;
        sending6();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Operation3) {
        colour1 = main0;
        number_stroca = stroca1;
        sending7();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot7) {
        colour1 = main0;
        number_stroca = stroca1;
        sending8();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot8) {
        colour1 = main0;
        number_stroca = stroca1;
        sending9();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot9) {
        colour1 = main0;
        number_stroca = stroca1;
        sending10();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot10) {
        colour1 = main0;
        number_stroca = stroca1;
        sending11();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot11) {
        colour1 = main0;
        number_stroca = stroca1;
        sending12();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot12) {
        colour1 = main0;
        number_stroca = stroca1;
        sending13();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot13) {
        colour1 = main0;
        number_stroca = stroca1;
        sending14();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot14) {
        colour1 = main0;
        number_stroca = stroca1;
        sending15();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot15) {
        colour1 = main0;
        number_stroca = stroca1;
        sending16();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot16) {
        colour1 = main0;
        number_stroca = stroca1;
        sending17();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot17) {
        colour1 = main0;
        number_stroca = stroca1;
        sending18();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot18) {
        colour1 = main0;
        number_stroca = stroca1;
        sending19();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot19) {
        colour1 = main0;
        number_stroca = stroca1;
        sending20();
        nameP();
    }

    if (uopt->presetSlot == 'A') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending1a();
        nameP();
    } else if (uopt->presetSlot == 'B') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending2a();
        nameP();
    } else if (uopt->presetSlot == 'C') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending3a();
        nameP();
    } else if (uopt->presetSlot == 'D') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending4a();
        nameP();
    } else if (uopt->presetSlot == 'E') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending5a();
        nameP();
    } else if (uopt->presetSlot == 'F') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending6a();
        nameP();
    } else if (uopt->presetSlot == 'G') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending7a();
        nameP();
    } else if (uopt->presetSlot == 'H') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending8a();
        nameP();
    } else if (uopt->presetSlot == 'I') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending9a();
        nameP();
    } else if (uopt->presetSlot == 'J') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending10a();
        nameP();
    } else if (uopt->presetSlot == 'K') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending11a();
        nameP();
    } else if (uopt->presetSlot == 'L') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending12a();
        nameP();
    } else if (uopt->presetSlot == 'M') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending13a();
        nameP();
    } else if (uopt->presetSlot == 'N') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending14a();
        nameP();
    } else if (uopt->presetSlot == 'O') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending15a();
        nameP();
    } else if (uopt->presetSlot == 'P') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending16a();
        nameP();
    } else if (uopt->presetSlot == 'Q') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending17a();
        nameP();
    } else if (uopt->presetSlot == 'R') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending18a();
        nameP();
    } else if (uopt->presetSlot == 'S') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending19a();
        nameP();
    } else if (uopt->presetSlot == 'T') {
        colour1 = yellowT;
        number_stroca = stroca3;
        sending20a();
        nameP();
    }

    if (oled_menuItem == OSD_Profile_SelectSlot) {
        colour1 = main0;
        number_stroca = stroca2;
        sending1b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot1) {
        colour1 = main0;
        number_stroca = stroca2;
        sending2b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot2) {
        colour1 = main0;
        number_stroca = stroca2;
        sending3b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot3) {
        colour1 = main0;
        number_stroca = stroca2;
        sending4b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot4) {
        colour1 = main0;
        number_stroca = stroca2;
        sending5b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot5) {
        colour1 = main0;
        number_stroca = stroca2;
        sending6b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Slot6) {
        colour1 = main0;
        number_stroca = stroca2;
        sending7b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_SelectPreset) {
        colour1 = main0;
        number_stroca = stroca2;
        sending8b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Preset1) {
        colour1 = main0;
        number_stroca = stroca2;
        sending9b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Preset2) {
        colour1 = main0;
        number_stroca = stroca2;
        sending10b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Preset3) {
        colour1 = main0;
        number_stroca = stroca2;
        sending11b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Preset4) {
        colour1 = main0;
        number_stroca = stroca2;
        sending12b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Preset5) {
        colour1 = main0;
        number_stroca = stroca2;
        sending13b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Preset6) {
        colour1 = main0;
        number_stroca = stroca2;
        sending14b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Preset7) {
        colour1 = main0;
        number_stroca = stroca2;
        sending15b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Preset8) {
        colour1 = main0;
        number_stroca = stroca2;
        sending16b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Preset9) {
        colour1 = main0;
        number_stroca = stroca2;
        sending17b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Preset10) {
        colour1 = main0;
        number_stroca = stroca2;
        sending18b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Preset11) {
        colour1 = main0;
        number_stroca = stroca2;
        sending19b();
        nameP();
    } else if (oled_menuItem == OSD_Profile_Preset12) {
        colour1 = main0;
        number_stroca = stroca2;
        sending20b();
        nameP();
    }
};
void handle_y(void)
{
    uopt->presetPreference = OutputCustomized;
    saveUserPrefs();
    uopt->presetPreference = OutputCustomized;
    if (rto->videoStandardInput == 14) {
        rto->videoStandardInput = 15;
    } else {
        applyPresets(rto->videoStandardInput);
    }
    saveUserPrefs();
};

void handle_z(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "Contrast");
    // Osd_Display(1, "Saturation");

    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "Saturation");

    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "Default");
}

void handle_A(void)
{


    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);


    colour1 = main0;
    number_stroca = stroca1;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    Typ(contrast);


    OSD_c2(0x3E, P13, main0);
    OSD_c2(0x3E, P14, main0);
    OSD_c2(0x3E, P15, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);


    colour1 = main0;
    number_stroca = stroca2;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    Typ(saturation);
};


void handle_caret(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        if (!lineOption)
            A2_main0 = 0x14;
        else
            A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    // colour1 = blue;
    // number_stroca = stroca1;
    // __(icon5, _27);
    // number_stroca = stroca2;
    // __(I, _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "DoubleLine");
    // Osd_Display(1, "Smooth");
    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "Smooth");
    // Osd_Display(1, "Bright");

    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "Bright");
    // Osd_Display(1, "Contrast");
};
void handle_at(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    // number_stroca = stroca1;
    // __(icon5, _27);
    number_stroca = stroca2;
    __('1', _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "RGBs");
    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "RGsB");
    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "VGA");
};
void handle_exclamation(void)
{
    A1_yellow = main0;
    A2_main0 = main0;
    A3_main0 = main0;

    colour1 = blue;
    // number_stroca = stroca1;
    // __(icon5, _27);
    // number_stroca = stroca2;
    // __('1', _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(0, "Whether to keep the settings");

    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(0, "Restore in ");

    if (keepSettings) {
        colour1 = yellowT;
        OSD_c3(0x15, P2, yellowT);
    } else {
        colour1 = A3_main0;
        OSD_c3(0x15, P2, blue_fill);
    }
    number_stroca = stroca3;
    Osd_Display(3, "Changes");

    if (!keepSettings) {
        colour1 = yellowT;
        OSD_c3(0x15, P13, yellowT);
    } else {
        colour1 = A3_main0;
        OSD_c3(0x15, P13, blue_fill);
    }
    Osd_Display(0xff, "    Recover");

    // OSD_c3(0x15, P2, blue_fill);
};
void handle_hash(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('2', _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "YPBPR");
    colour1 = A2_main0;
    number_stroca = stroca2;
    Osd_Display(1, "SV");
    colour1 = A3_main0;
    number_stroca = stroca3;
    Osd_Display(1, "AV");
};
void handle_dollar(void)
{
    if (oled_menuItem == OSD_Input_SV) {
        OSD_writeString(4, 2, "Format:");
        OSD_writeString(4, 3, "                      ");
    } else if (oled_menuItem == OSD_Input_AV) {
        OSD_writeString(4, 3, "Format:");
        OSD_writeString(4, 2, "                      ");
    } else {
        OSD_writeString(4, 2, "                      ");
        OSD_writeString(4, 3, "                      ");
    }
    switch (SVModeOption) {
        case 0: {
            if (oled_menuItem == OSD_Input_SV)
                OSD_writeString(11, 2, "Auto           ");
        } break;
        case 1: {
            if (oled_menuItem == OSD_Input_SV)
                OSD_writeString(11, 2, "PAL            ");
        } break;
        case 2: {
            if (oled_menuItem == OSD_Input_SV)
                OSD_writeString(11, 2, "NTSC-M         ");
        } break;
        case 3: {
            if (oled_menuItem == OSD_Input_SV)
                OSD_writeString(11, 2, "PAL-60         ");
        } break;
        case 4: {
            if (oled_menuItem == OSD_Input_SV)
                OSD_writeString(11, 2, "NTSC443        ");
        } break;
        case 5: {
            if (oled_menuItem == OSD_Input_SV)
                OSD_writeString(11, 2, "NTSC-J          ");
        } break;
        case 6: {
            if (oled_menuItem == OSD_Input_SV)
                OSD_writeString(11, 2, "PAL-N w/ p      ");
        } break;
        case 7: {
            if (oled_menuItem == OSD_Input_SV)
                OSD_writeString(11, 2, "PAL-M w/o p    ");
        } break;
        case 8: {
            if (oled_menuItem == OSD_Input_SV)
                OSD_writeString(11, 2, "PAL-M          ");
        } break;
        case 9: {
            if (oled_menuItem == OSD_Input_SV)
                OSD_writeString(11, 2, "PAL Cmb -N     ");
        } break;
        case 10: {
            if (oled_menuItem == OSD_Input_SV)
                OSD_writeString(11, 2, "PAL Cmb -N w/ p");
        } break;
        case 11: {
            if (oled_menuItem == OSD_Input_SV)
                OSD_writeString(11, 2, "SECAM          ");
        } break;
        default: {
            if (oled_menuItem == OSD_Input_SV)
                OSD_writeString(11, 2, "               ");
        } break;
    }

    switch (AVModeOption) {
        case 0: {
            if (oled_menuItem == OSD_Input_AV)
                OSD_writeString(11, 3, "Auto           ");
        } break;
        case 1: {
            if (oled_menuItem == OSD_Input_AV)
                OSD_writeString(11, 3, "PAL            ");
        } break;
        case 2: {
            if (oled_menuItem == OSD_Input_AV)
                OSD_writeString(11, 3, "NTSC-M         ");
        } break;
        case 3: {
            if (oled_menuItem == OSD_Input_AV)
                OSD_writeString(11, 3, "PAL-60         ");
        } break;
        case 4: {
            if (oled_menuItem == OSD_Input_AV)
                OSD_writeString(11, 3, "NTSC443        ");
        } break;
        case 5: {
            if (oled_menuItem == OSD_Input_AV)
                OSD_writeString(11, 3, "NTSC-J          ");
        } break;
        case 6: {
            if (oled_menuItem == OSD_Input_AV)
                OSD_writeString(11, 3, "PAL-N w/ p      ");
        } break;
        case 7: {
            if (oled_menuItem == OSD_Input_AV)
                OSD_writeString(11, 3, "PAL-M w/o p    ");
        } break;
        case 8: {
            if (oled_menuItem == OSD_Input_AV)
                OSD_writeString(11, 3, "PAL-M          ");
        } break;
        case 9: {
            if (oled_menuItem == OSD_Input_AV)
                OSD_writeString(11, 3, "PAL Cmb -N     ");
        } break;
        case 10: {
            if (oled_menuItem == OSD_Input_AV)
                OSD_writeString(11, 3, "PAL Cmb -N w/ p");
        } break;
        case 11: {
            if (oled_menuItem == OSD_Input_AV)
                OSD_writeString(11, 3, "SECAM          ");
        } break;
        default: {
            if (oled_menuItem == OSD_Input_AV)
                OSD_writeString(11, 3, "               ");
        } break;
    }
};
void handle_percent(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __(I, _27);
    number_stroca = stroca3;
    __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "Setting");
};
void handle_ampersand(void)
{

    OSD_c1(0x3E, P13, main0);
    OSD_c1(0x3E, P14, main0);
    OSD_c1(0x3E, P15, main0);
    OSD_c1(0x3E, P16, main0);
    OSD_c1(0x3E, P17, main0);
    OSD_c1(0x3E, P18, main0);

    OSD_c2(0x3E, P13, main0);
    OSD_c2(0x3E, P14, main0);
    OSD_c2(0x3E, P15, main0);
    OSD_c2(0x3E, P16, main0);
    OSD_c2(0x3E, P17, main0);
    OSD_c2(0x3E, P18, main0);


    OSD_c3(0x3E, P13, main0);
    OSD_c3(0x3E, P14, main0);
    OSD_c3(0x3E, P15, main0);
    OSD_c3(0x3E, P16, main0);
    OSD_c3(0x3E, P17, main0);
    OSD_c3(0x3E, P18, main0);

    if (lineOption) {
        OSD_c1(n2, P23, main0);
        OSD_c1(X, P24, main0);
    } else {
        OSD_c1(n1, P23, main0);
        OSD_c1(X, P24, main0);
        smoothOption = false;
    }
    if (smoothOption) {
        OSD_c2(O, P23, main0);
        OSD_c2(N, P24, main0);
        OSD_c2(F, P25, blue_fill);
    } else {
        OSD_c2(O, P23, main0);
        OSD_c2(F, P24, main0);
        OSD_c2(F, P25, main0);
    }

    colour1 = main0;
    number_stroca = stroca3;
    sequence_number1 = _25;
    sequence_number2 = _24;
    sequence_number3 = _23;
    Typ(brightness);


    // colour1 = main0;
    // number_stroca = stroca3;
    // sequence_number1 = _25;
    // sequence_number2 = _24;
    // sequence_number3 = _23;
    // Typ(contrast);
};
void handle_asterisk(void)
{
    if (selectedMenuLine == 1) {
        A1_yellow = yellowT;
        A2_main0 = main0;
        A3_main0 = main0;
    } else if (selectedMenuLine == 2) {
        A1_yellow = main0;
        A2_main0 = yellowT;
        A3_main0 = main0;
    } else if (selectedMenuLine == 3) {
        A1_yellow = main0;
        A2_main0 = main0;
        A3_main0 = yellowT;
    }

    colour1 = blue;
    number_stroca = stroca1;
    __(icon5, _27);
    number_stroca = stroca2;
    __('4', _27);
    // number_stroca = stroca3;
    // __(icon6, _27);

    colour1 = A1_yellow;
    number_stroca = stroca1;
    Osd_Display(1, "Matched presets");
    // colour1 = A2_main0;
    // number_stroca = stroca2;
};
