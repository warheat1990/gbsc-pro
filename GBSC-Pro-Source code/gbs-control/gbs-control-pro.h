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
// - oled-menu-pro.cpp: OLED menu navigation and IR remote handling
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
// Types - TV OSD Menu (STV9426 chip)
// ====================================================================================

// TV OSD command enum for type-safe menu dispatch (used by osd-render-pro.cpp)
// Naming convention:
//   - Labels handlers: show menu text on the left (e.g., OSD_CMD_COLOR_PAGE1)
//   - Values handlers: show ON/OFF or numeric values on the right (e.g., OSD_CMD_COLOR_PAGE1_VALUES)
typedef enum : uint8_t {
    OSD_CMD_NONE = 0,

    // Cursor Positioning
    OSD_CMD_CURSOR_ROW1,
    OSD_CMD_CURSOR_ROW2,
    OSD_CMD_CURSOR_ROW3,

    // Main Menu
    OSD_CMD_MAIN_PAGE1,
    OSD_CMD_MAIN_PAGE1_UPDATE,
    OSD_CMD_MAIN_PAGE2,

    // Output Resolution
    OSD_CMD_OUTPUT_1080_1024_960,
    OSD_CMD_OUTPUT_720_480,
    OSD_CMD_OUTPUT_PASSTHROUGH,

    // Screen Settings
    OSD_CMD_SCREEN_SETTINGS,
    OSD_CMD_SCREEN_FULLHEIGHT,
    OSD_CMD_SCREEN_FULLHEIGHT_VALUES,

    // Color Settings
    OSD_CMD_COLOR_PAGE1,
    OSD_CMD_COLOR_PAGE1_VALUES,
    OSD_CMD_COLOR_PAGE2,
    OSD_CMD_COLOR_PAGE2_VALUES,
    OSD_CMD_COLOR_PAGE3,
    OSD_CMD_COLOR_RGB_LABELS,
    OSD_CMD_COLOR_RGB_VALUES,

    // System Settings - SV/AV Input
    OSD_CMD_SYS_SVINPUT_VALUES,

    // System Settings - General
    OSD_CMD_SYS_PAGE1,
    OSD_CMD_SYS_PAGE1_VALUES,
    OSD_CMD_SYS_PAGE2,
    OSD_CMD_SYS_PAGE2_VALUES,
    OSD_CMD_SYS_PAGE4,
    OSD_CMD_SYS_PAGE4_VALUES,
    OSD_CMD_SYS_PAGE5,
    OSD_CMD_SYS_PAGE5_VALUES,

    // Developer
    OSD_CMD_DEV_MEMORY,
    OSD_CMD_DEV_MEMORY_VALUES,
    OSD_CMD_DEV_DEBUG,
    OSD_CMD_DEV_DEBUG_VALUES,

    // Restart
    OSD_CMD_RESTART,

    // Profile
    OSD_CMD_PROFILE_SAVELOAD,
    OSD_CMD_PROFILE_SLOTDISPLAY,
    OSD_CMD_PROFILE_SLOTROW1,
    OSD_CMD_PROFILE_SLOTROW2,
    OSD_CMD_PROFILE_SLOTROW3,

    // Input Menu
    OSD_CMD_INPUT_PAGE1,
    OSD_CMD_INPUT_PAGE2,
    OSD_CMD_INPUT_INFO,
    OSD_CMD_INPUT_FORMAT,
    OSD_CMD_INPUT_SOURCE,

    // Calibration
    OSD_CMD_ADCCALIB_RUNNING,
    OSD_CMD_ADCCALIB_DISPLAY,

    OSD_CMD_COUNT
} OsdCommand;

typedef struct {
    OsdCommand cmd;
    void (*handler)(void);
    bool saveable;  // If true, saved to lastOsdCommand for refresh on signal change
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

// Video format names lookup table (for SV/AV mode display)
static const char* const videoFormatNames[] = {
    "Auto", "PAL", "NTSC-M", "PAL-60", "NTSC443", "NTSC-J",
    "PAL-N w/ p", "PAL-M w/o p", "PAL-M", "PAL Cmb -N",
    "PAL Cmb -N w/ p", "SECAM"
};

// Get video format name by index (0-11), returns "Unknown" if out of range
inline const char* getVideoFormatName(uint8_t index) {
    if (index < MODEOPTION_MAX) return videoFormatNames[index];
    return "Unknown";
}

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
// Macros
// ====================================================================================

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

// ====================================================================================
// Types - OLED Menu States (SSD1306 display, used by ir-menu-pro.cpp)
// ====================================================================================

typedef enum {
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
    // OLED_OutputResolution_Downscale,  // disabled

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
    // OLED_SystemSettings_ComponentVGA,  // disabled

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

    // Developer submenu (disabled)
    // OLED_Developer,
    // OLED_Developer_MemoryAdjust,
    // OLED_Developer_HSyncAdjust,
    // OLED_Developer_HTotalAdjust,
    // OLED_Developer_DebugView,
    // OLED_Developer_ADCFilter,
    // OLED_Developer_FreezeCapture,

    // Profile/Slot Management
    // Row 1: Load profile slots (1-20 = A-T)
    OLED_Profile_Load1,
    OLED_Profile_Load2,
    OLED_Profile_Load3,
    OLED_Profile_Load4,
    OLED_Profile_Load5,
    OLED_Profile_Load6,
    OLED_Profile_Load7,
    OLED_Profile_Load8,
    OLED_Profile_Load9,
    OLED_Profile_Load10,
    OLED_Profile_Load11,
    OLED_Profile_Load12,
    OLED_Profile_Load13,
    OLED_Profile_Load14,
    OLED_Profile_Load15,
    OLED_Profile_Load16,
    OLED_Profile_Load17,
    OLED_Profile_Load18,
    OLED_Profile_Load19,
    OLED_Profile_Load20,
    // Row 2: Save profile slots (1-20 = A-T)
    OLED_Profile_Save1,
    OLED_Profile_Save2,
    OLED_Profile_Save3,
    OLED_Profile_Save4,
    OLED_Profile_Save5,
    OLED_Profile_Save6,
    OLED_Profile_Save7,
    OLED_Profile_Save8,
    OLED_Profile_Save9,
    OLED_Profile_Save10,
    OLED_Profile_Save11,
    OLED_Profile_Save12,
    OLED_Profile_Save13,
    OLED_Profile_Save14,
    OLED_Profile_Save15,
    OLED_Profile_Save16,
    OLED_Profile_Save17,
    OLED_Profile_Save18,
    OLED_Profile_Save19,
    OLED_Profile_Save20,
} OLED_MenuState;

// ====================================================================================
// Sub-module Headers (after MenuEntry and OLED_MenuState are defined)
// ====================================================================================

#include "oled-menu-pro.h"
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

// Maximum menu rows supported (can expand 3→6 in future)
// Note: Also defined in OSD_stv9426.h for the OSD library
#ifndef OSD_MAX_MENU_ROWS
#define OSD_MAX_MENU_ROWS 3
#endif

extern char osdDisplayValue;
extern OsdCommand lastOsdCommand;
extern boolean irEnabled;
extern uint8_t menuLineColors[OSD_MAX_MENU_ROWS];  // Index 0-2 = rows 1-3
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
