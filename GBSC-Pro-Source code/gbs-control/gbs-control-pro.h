// ====================================================================================
// gbs-control-pro.h
// GBSC-Pro Main Header
//
// This file contains all Pro-specific declarations:
// - IR remote control (IRremoteESP8266)
// - TV OSD menu system (STV9426 chip)
// - Audio volume control (PT2257 chip)
// - OLED display integration
// - Extended color/video adjustments
//
// Code organization:
// - gbs-control-pro.cpp: Core functions, global variables
// - ir-menu-pro.cpp: IR remote handling and menu navigation
// - osd-render-pro.cpp: TV OSD menu rendering
// - OLEDMenuImplementation-pro.cpp: OLED display and ADV communication
// ====================================================================================

#pragma once

#include <Arduino.h>
#include "options-pro.h"

// ====================================================================================
// Forward Declarations
// ====================================================================================

class SSD1306Wire;
class WebSocketsServer;
struct runTimeOptions;
struct userOptions;
template <uint8_t> class TV5725;

// ====================================================================================
// Types - Menu System
// ====================================================================================

typedef struct {
    int key;
    void (*handler)(void);
} MenuEntry;

// ====================================================================================
// Constants - Timing
// ====================================================================================

#define OSD_CLOSE_TIME 16000            // 16 sec
#define OSD_RESOLUTION_UP_TIME 1000     // 1 sec
#define OSD_RESOLUTION_CLOSE_TIME 20000 // 20 sec

// ====================================================================================
// Constants - Mode Options
// ====================================================================================

#define MODEOPTION_MAX 12
#define MODEOPTION_MIN 0
#define STEP 1

// ====================================================================================
// Constants - Video Format
// ====================================================================================

#define RGB1 0x01
#define YUV0 0x00

// ====================================================================================
// Constants - Sync Selection
// ====================================================================================

#define HV_Enable  0x00
#define HV_Disable 0x01

// ====================================================================================
// Constants - Compatibility Mode
// ====================================================================================

#define COMPATIBILITY_OFF 0
#define COMPATIBILITY_ON  1

// ====================================================================================
// Constants - Input Source Category
// ====================================================================================

#define InputSourceRGBs 1
#define InputSourceVGA  2
#define InputSourceYUV  3

// ====================================================================================
// Constants - Input Type (specific input within category)
// ====================================================================================

#define InputTypeRGBs 1
#define InputTypeRGsB 2
#define InputTypeVGA  3
#define InputTypeYUV  4
#define InputTypeSV   5
#define InputTypeAV   6

// ====================================================================================
// Constants - OSD Characters
// ====================================================================================

#define OSD_CROSS_TOP '7'
#define OSD_CROSS_MID '8'
#define OSD_CROSS_BOTTOM '9'

// ====================================================================================
// Macros
// ====================================================================================

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

// ====================================================================================
// Enums - OLED Menu States
// ====================================================================================

typedef enum {
    // No menu / OSD closed
    OLED_None,

    // Main menu (top level)
    OLED_Input,
    OLED_OutputResolution,
    OLED_ScreenSettings,
    OLED_ColorSettings,
    OLED_SystemSettings,
    OLED_ResetSettings,

    // Input submenu
    OLED_Input_RGBs,
    OLED_Input_RGsB,
    OLED_Input_VGA,
    OLED_Input_YPBPR,
    OLED_Input_SV,
    OLED_Input_AV,

    // Output Resolution submenu
    OLED_OutputResolution_1080,
    OLED_OutputResolution_1024,
    OLED_OutputResolution_960,
    OLED_OutputResolution_720,
    OLED_OutputResolution_480,
    OLED_OutputResolution_PassThrough,

    // Screen Settings submenu
    OLED_ScreenSettings_Move,
    OLED_ScreenSettings_Scale,
    OLED_ScreenSettings_Borders,
    OLED_ScreenSettings_MoveAdjust,
    OLED_ScreenSettings_ScaleAdjust,
    OLED_ScreenSettings_BordersAdjust,

    // Color Settings submenu
    OLED_ColorSettings_ADCGain,
    OLED_ColorSettings_Scanlines,
    OLED_ColorSettings_LineFilter,
    OLED_ColorSettings_Sharpness,
    OLED_ColorSettings_Peaking,
    OLED_ColorSettings_StepResponse,
    OLED_ColorSettings_RGB_R,
    OLED_ColorSettings_RGB_G,
    OLED_ColorSettings_RGB_B,
    OLED_ColorSettings_Y_Gain,
    OLED_ColorSettings_Color,
    OLED_ColorSettings_DefaultColor,

    // System Settings submenu
    OLED_SystemSettings_MatchedPresets,
    OLED_ScreenSettings_FullHeight,
    OLED_SystemSettings_UseUpscaling,
    OLED_SystemSettings_Force5060Hz,
    OLED_SystemSettings_ClockGenerator,
    OLED_SystemSettings_ADCCalibration,
    OLED_SystemSettings_FrameTimeLock,
    OLED_SystemSettings_LockMethod,
    OLED_SystemSettings_Deinterlace,
    OLED_SystemSettings_Compatibility,

    // SV/AV Input Settings submenu
    OLED_SystemSettings_SVAVInputSettings,
    OLED_SystemSettings_SVAVInput_DoubleLine,
    OLED_SystemSettings_SVAVInput_Smooth,
    OLED_SystemSettings_SVAVInput_Bright,
    OLED_SystemSettings_SVAVInput_Contrast,
    OLED_SystemSettings_SVAVInput_Saturation,
    OLED_SystemSettings_SVAVInput_Default,

    // Special screens
    OLED_Volume_Adjust,
    OLED_RetainedSettings,
    OLED_Info_Display,
    OLED_EnableOTA,
    OLED_Restart,
    OLED_ResetDefaults,

    // Profile/Slot Management
    OLED_Profile,
    OLED_Profile_SaveConfirm,
    OLED_Profile_Save,
    OLED_Profile_Load,
    OLED_Profile_Operation1,
    OLED_Profile_Operation2,
    OLED_Profile_Operation3,
    OLED_Profile_SelectSlot,
    OLED_Profile_Slot1,
    OLED_Profile_Slot2,
    OLED_Profile_Slot3,
    OLED_Profile_Slot4,
    OLED_Profile_Slot5,
    OLED_Profile_Slot6,
    OLED_Profile_Slot7,
    OLED_Profile_Slot8,
    OLED_Profile_Slot9,
    OLED_Profile_Slot10,
    OLED_Profile_Slot11,
    OLED_Profile_Slot12,
    OLED_Profile_Slot13,
    OLED_Profile_Slot14,
    OLED_Profile_Slot15,
    OLED_Profile_Slot16,
    OLED_Profile_Slot17,
    OLED_Profile_Slot18,
    OLED_Profile_Slot19,
    OLED_Profile_SelectPreset,
    OLED_Profile_Preset1,
    OLED_Profile_Preset2,
    OLED_Profile_Preset3,
    OLED_Profile_Preset4,
    OLED_Profile_Preset5,
    OLED_Profile_Preset6,
    OLED_Profile_Preset7,
    OLED_Profile_Preset8,
    OLED_Profile_Preset9,
    OLED_Profile_Preset10,
    OLED_Profile_Preset11,
    OLED_Profile_Preset12,
} OSD_Menu;

// ====================================================================================
// Sub-module Headers (after MenuEntry and OSD_Menu are defined)
// ====================================================================================

#include "ir-menu-pro.h"
#include "osd-render-pro.h"

// ====================================================================================
// External Variables - IR Remote
// ====================================================================================

#include <IRremoteESP8266.h>
#include <IRrecv.h>

extern IRrecv irrecv;
extern decode_results results;

// ====================================================================================
// External Variables - Input/Output State
// ====================================================================================

extern uint8_t inputSource;
extern uint8_t inputType;
extern uint8_t rgbComponentMode;

// ====================================================================================
// External Variables - Timing
// ====================================================================================

extern bool irDecodedFlag;
extern unsigned long lastSignalTime;
extern unsigned long lastSystemTime;
extern unsigned long lastWebUpdateTime;
extern unsigned long lastMenuItemTime;
extern unsigned long lastResolutionTime;
extern unsigned long resolutionStartTime;

// ====================================================================================
// External Variables - OLED Menu State
// ====================================================================================

extern int oled_menuItem;
extern int lastOledMenuItem;
extern uint8_t oledClearFlag;
extern boolean NEW_OLED_MENU;
extern int selectedMenuLine;

// ====================================================================================
// External Variables - TV OSD State
// ====================================================================================

extern char osdDisplayValue;
extern char lastOsdCommand;
extern boolean irEnabled;
extern int A1_yellow;
extern int A2_main0;
extern int A3_main0;
extern uint8_t isInfoDisplayActive;
extern uint16_t horizontalBlankStart;
extern uint16_t horizontalBlankStop;
extern const MenuEntry menuTable[];

// ====================================================================================
// External Variables - Picture Settings
// ====================================================================================

extern unsigned char R_VAL;
extern unsigned char G_VAL;
extern unsigned char B_VAL;
extern uint8_t brightness;
extern uint8_t contrast;
extern uint8_t saturation;
extern uint8_t brightnessOrContrastOption;

// ====================================================================================
// External Variables - Video Mode Options
// ====================================================================================

extern uint8_t SVModeOption;
extern uint8_t AVModeOption;
extern uint8_t SVModeOptionChanged;
extern uint8_t AVModeOptionChanged;
extern uint8_t smoothOption;
extern uint8_t lineOption;
extern bool settingLineOptionChanged;
extern bool settingSmoothOptionChanged;

// ====================================================================================
// External Variables - Resolution Settings
// ====================================================================================

extern uint8_t keepSettings;
extern uint8_t tentativeResolution;

// ====================================================================================
// External Variables - Audio
// ====================================================================================

extern uint8_t volume;
extern boolean audioMuted;

// ====================================================================================
// External Variables - Video Format Table
// ====================================================================================

extern const uint8_t ADV_VideoFormats[12];

// ====================================================================================
// Function Declarations - ADV Communication
// ====================================================================================

// ADV packet wrappers
void ADV_sendLine1X();
void ADV_sendLine2X();
void ADV_sendSmoothOff();
void ADV_sendSmoothOn();
void ADV_sendCompatibility(bool mode);
void ADV_sendVideoFormat(uint8_t format);
void ADV_sendBCSH(unsigned char reg, unsigned char val);
void ADV_sendCustomI2C(const unsigned char* data, size_t size);
void ADV_applyPendingOptions(void);

// ====================================================================================
// Function Declarations - Input Source Switching
// ====================================================================================

void InputRGBs(void);
void InputRGsB(void);
void InputVGA(void);
void InputYUV(void);
void InputSV(void);
void InputAV(void);

void InputRGBs_mode(uint8_t mode);
void InputRGsB_mode(uint8_t mode);
void InputVGA_mode(uint8_t mode);
void InputSV_mode(uint8_t mode);
void InputAV_mode(uint8_t mode);

void applySavedInputSource(void);

// ====================================================================================
// Function Declarations - OLED Menu
// ====================================================================================

void resetOLEDScreenSaverTimer();

// ====================================================================================
// Function Declarations - Color Conversion
// ====================================================================================

void applyRGBtoYUVConversion(void);
void readYUVtoRGBConversion(void);

// ====================================================================================
// Function Declarations - Video Mode
// ====================================================================================

void applyVideoModePreset(void);
boolean hasOutputFrequencyChanged(void);

// ====================================================================================
// Function Declarations - Menu Refresh
// ====================================================================================

void refreshMenusOnSignalChange(void);

// ====================================================================================
// Function Declarations - Status
// ====================================================================================

bool isPeakingLocked(void);
void broadcastProStatus(WebSocketsServer& ws);
