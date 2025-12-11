#pragma once

// GBSC-Pro Extensions Header
// This file contains all the additions made in GBSC-Pro fork:
// - IR remote control (IRremoteESP8266)
// - On-screen display menu system (STV9426 OSD chip)
// - Audio volume control (PT2257 chip)
// - OLED display integration
// - Extended color/video adjustments
//
// NOTE: This header is included by gbs-control.ino, which already has all the
// necessary includes. We only forward-declare what we need here.

#include <Arduino.h>

// Forward declarations to avoid circular dependencies
class SSD1306Wire;
struct runTimeOptions;
struct userOptions;
template <uint8_t>
class TV5725;
class IRrecv;
struct decode_results;

// Constants
#define MODEOPTION_MAX 12
#define MODEOPTION_MIN 0
#define STEP 1
#define OSD_CLOSE_TIME 16000            // 16 sec
#define OSD_RESOLUTION_UP_TIME 1000     // 1 sec
#define OSD_RESOLUTION_CLOSE_TIME 20000 // 20 sec

// OSD Character Definitions
#define OSD_CROSS_TOP '7'
#define OSD_CROSS_MID '8'
#define OSD_CROSS_BOTTOM '9'

// Macros
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

// OSD Menu States
typedef enum {
    // Main menu and close
    OSD_None = 0,                                    // No menu/OSD closed

    // Resolution menu
    OSD_Resolution = 62,                             // Resolution menu
    OSD_Resolution_1080 = 68,                        // 1920x1080
    OSD_Resolution_1024 = 69,                        // 1280x1024
    OSD_Resolution_960 = 70,                         // 1280x960
    OSD_Resolution_720 = 71,                         // 1280x720
    OSD_Resolution_480 = 72,                         // 480p/576p
    // OSD_Resolution_Downscale = 73,                // Downscale (UNUSED/COMMENTED)
    OSD_Resolution_pass = 74,                        // Pass-through
    OSD_Resolution_RetainedSettings = 169,           // Retained settings (moved here for logical grouping)

    // Main menu (top level)
    OSD_ScreenSettings = 63,                         // Screen Settings
    OSD_ColorSettings = 64,                          // Color Settings
    OSD_SystemSettings = 65,                         // System Settings
    // OSD_Developer = 66,                           // Developer menu (UNUSED/COMMENTED)
    OSD_ResetDefault = 67,                           // Reset default

    // Screen Settings submenu
    OSD_ScreenSettings_Main = 75,                    // Settings main
    OSD_ScreenSettings_Scale = 76,                   // Screen Scale
    OSD_ScreenSettings_Position = 77,                // Screen Position
    OSD_ScreenSettings_ScaleAdjust = 79,             // Adjust scale (OK on Scale)
    OSD_ScreenSettings_PositionAdjust = 80,          // Adjust position (OK on Position)
    OSD_ScreenSettings_Advanced = 81,                // Advanced settings

    // Color Settings submenu
    OSD_ColorSettings_Bright = 82,                   // Brightness
    OSD_ColorSettings_Contrast = 83,                 // Contrast
    OSD_ColorSettings_Saturation = 84,               // Saturation
    OSD_ColorSettings_Sharpness = 85,                // Sharpness
    OSD_ColorSettings_ColorTemp = 86,                // Color Temperature
    OSD_ColorSettings_Advanced = 87,                 // Advanced color
    OSD_ColorSettings_RGB_R = 88,                    // RGB Red channel
    OSD_ColorSettings_RGB_G = 89,                    // RGB Green channel
    OSD_ColorSettings_RGB_B = 90,                    // RGB Blue channel
    OSD_ColorSettings_RGB_Preset = 91,               // RGB Preset
    OSD_ColorSettings_RGB_Save = 92,                 // Save RGB preset
    OSD_ColorSettings_RGB_Load = 93,                 // Load RGB preset

    // System Settings submenu
    OSD_Volume_Adjust = 1,                           // Volume adjustment screen
    OSD_SystemSettings_SV = 94,                      // S-Video settings
    OSD_SystemSettings_AV = 95,                      // AV settings
    OSD_SystemSettings_Upscaling = 96,               // Use upscaling option
    // OSD_SystemSettings_ComponentVGA = 97,         // Component/VGA settings (UNUSED/COMMENTED)
    OSD_SystemSettings_SVAVInput_Line = 98,          // Line option
    OSD_SystemSettings_SVAVInput_Smooth_Main = 99,   // Smooth option (main menu)
    OSD_SystemSettings_SVAVInput_Bright_Main = 100,  // Brightness SV/AV (main menu)
    OSD_SystemSettings_SVAVInput_Contrast_Main = 101,// Contrast SV/AV (main menu)
    OSD_SystemSettings_SVAVInput_Saturation_Main = 102, // Saturation SV/AV (main menu)
    OSD_SystemSettings_SVAVInput_Mode = 103,         // Mode SV/AV
    OSD_SystemSettings_Volume = 109,                 // Volume control
    OSD_SystemSettings_Restart = 110,                // Restart menu
    OSD_SystemSettings_Info = 111,                   // System info

    // Advanced/Debug Settings (UNUSED/COMMENTED)
    // OSD_Advanced_MemoryAdjust = 104,              // Memory adjustment (MEM left/right)
    // OSD_Advanced_HSyncAdjust = 105,               // HSync adjustment (HS left/right)
    // OSD_Advanced_HTotalAdjust = 106,              // Horizontal total adjustment (HTotal -/+)
    // OSD_Advanced_DebugView = 107,                 // Debug view
    // OSD_Advanced_ADCFilter = 108,                 // ADC filter settings

    // Profile/Slot Management
    OSD_Profile_Main = 112,                          // Profile main menu
    OSD_Profile_SaveConfirm = 113,                   // Save confirmation
    OSD_Profile_Save = 114,                          // Save profile
    OSD_Profile_Load = 115,                          // Load profile
    OSD_Profile_Operation1 = 116,                    // Profile operation 1
    OSD_Profile_Operation2 = 117,                    // Profile operation 2
    OSD_Profile_Operation3 = 118,                    // Profile operation 3
    OSD_Profile_SelectSlot = 119,                    // Slot selection menu
    OSD_Profile_Slot1 = 120,                         // Profile slot 1
    OSD_Profile_Slot2 = 121,                         // Profile slot 2
    OSD_Profile_Slot3 = 122,                         // Profile slot 3
    OSD_Profile_Slot4 = 123,                         // Profile slot 4
    OSD_Profile_Slot5 = 124,                         // Profile slot 5
    OSD_Profile_Slot6 = 125,                         // Profile slot 6
    OSD_Profile_Slot7 = 126,                         // Profile slot 7
    OSD_Profile_Slot8 = 127,                         // Profile slot 8
    OSD_Profile_Slot9 = 128,                         // Profile slot 9
    OSD_Profile_Slot10 = 129,                        // Profile slot 10
    OSD_Profile_Slot11 = 130,                        // Profile slot 11
    OSD_Profile_Slot12 = 131,                        // Profile slot 12
    OSD_Profile_Slot13 = 132,                        // Profile slot 13
    OSD_Profile_Slot14 = 133,                        // Profile slot 14
    OSD_Profile_Slot15 = 134,                        // Profile slot 15
    OSD_Profile_Slot16 = 135,                        // Profile slot 16
    OSD_Profile_Slot17 = 136,                        // Profile slot 17
    OSD_Profile_Slot18 = 137,                        // Profile slot 18
    OSD_Profile_Slot19 = 138,                        // Profile slot 19
    OSD_Profile_SelectPreset = 139,                  // Preset selection menu
    OSD_Profile_Preset1 = 140,                       // Profile preset 1
    OSD_Profile_Preset2 = 141,                       // Profile preset 2
    OSD_Profile_Preset3 = 142,                       // Profile preset 3
    OSD_Profile_Preset4 = 143,                       // Profile preset 4
    OSD_Profile_Preset5 = 144,                       // Profile preset 5
    OSD_Profile_Preset6 = 145,                       // Profile preset 6
    OSD_Profile_Preset7 = 146,                       // Profile preset 7
    OSD_Profile_Preset8 = 147,                       // Profile preset 8
    OSD_Profile_Preset9 = 148,                       // Profile preset 9
    OSD_Profile_Preset10 = 149,                      // Profile preset 10
    OSD_Profile_Preset11 = 150,                      // Profile preset 11
    OSD_Profile_Preset12 = 151,                      // Profile preset 12

    // Info display
    OSD_Info_Display = 152,                          // Info display screen
    // OSD_Advanced_FreezeCapture = 153,             // Freeze capture (UNUSED/COMMENTED)

    // Input menu
    OSD_Input = 154,                                 // Input menu
    OSD_Input_RGBs,                                  // 155 Input RGBs
    OSD_Input_RGsB,                                  // 156 Input RGsB (FIX TYPO: was RBsB)
    OSD_Input_VGA,                                   // 157 Input VGA
    OSD_Input_YPBPR,                                 // 158 Input YPbPr
    OSD_Input_SV,                                    // 159 Input S-Video
    OSD_Input_AV,                                    // 160 Input AV

    // SV/AV Input settings (continue from 161)
    OSD_SystemSettings_SVAVInput,                    // 161 SV/AV Input settings
    OSD_SystemSettings_SVAVInput_DoubleLine,         // 162 Double line
    OSD_SystemSettings_SVAVInput_Smooth,             // 163 Smooth (submenu)
    OSD_SystemSettings_SVAVInput_Bright,             // 164 Brightness (submenu)
    OSD_SystemSettings_SVAVInput_contrast,           // 165 Contrast (submenu)
    OSD_SystemSettings_SVAVInput_saturation,         // 166 Saturation (submenu)
    OSD_SystemSettings_SVAVInput_default,            // 167 Default
    OSD_SystemSettings_SVAVInput_Compatibility,      // 168 Compatibility
} OSD_Menu;

// Menu Handler Function Pointer Type
typedef struct
{
    int key;
    void (*handler)(void);
} MenuEntry;

// ============================================================================
// GLOBAL VARIABLES (extern declarations)
// ============================================================================

// IR Remote
extern const int kRecvPin;
extern IRrecv irrecv;
extern decode_results results;

// Status and Control Variables
extern uint8_t brightnessOrContrastOption;
extern uint8_t Info;
extern uint8_t selectedInputSource;

// Timing Variables
extern unsigned long OledUpdataTime;
extern uint8_t infoState;
extern bool irDecodedFlag;
extern unsigned long lastSignalTime;
extern unsigned long lastSystemTime;
extern unsigned long lastWebUpdateTime;
extern unsigned long lastMenuItemTime;
extern unsigned long lastResolutionTime;
extern unsigned long resolutionStartTime;

// Position Variables
extern uint16_t horizontalBlankStart;
extern uint16_t horizontalBlankStop;

// RGB Color Values
extern unsigned char R_VAL;
extern unsigned char G_VAL;
extern unsigned char B_VAL;

// Audio and Display Control
extern char osdDisplayValue;
extern boolean irEnabled;
extern int selectedMenuLine;
extern uint8_t Volume;
extern boolean audioMuted;
extern int oled_menuItem;
extern int lastOledMenuItem;
extern uint8_t oledClearFlag;
extern boolean NEW_OLED_MENU;

// Video Mode Options
extern uint8_t SVModeOption;
extern uint8_t AVModeOption;
extern uint8_t SVModeOptionChanged;
extern uint8_t AVModeOptionChanged;
extern uint8_t smoothOption;
extern uint8_t lineOption;
extern bool settingLineOptionChanged;
extern bool settingSmoothOptionChanged;

// Picture Adjustment
extern uint8_t brightness;
extern uint8_t contrast;
extern uint8_t saturation;

// Miscellaneous
extern uint8_t rgbComponentMode;
extern uint8_t keepSettings;
extern uint8_t tentativeResolution;

// Color Display Variables
extern int A1_yellow;
extern int A2_main0;
extern int A3_main0;

// Menu Table
extern const MenuEntry menuTable[];

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

// Core GBSC-Pro Functions - Color Conversion
void applyRGBtoYUVConversion(void);  // Convert R_VAL, G_VAL, B_VAL → YUV registers (ITU-R BT.601)
void readYUVtoRGBConversion(void);   // Read YUV registers → R_VAL, G_VAL, B_VAL (ITU-R BT.601)
void UpDisplay(void);
void Mode_Option(void);
boolean CheckInputFrequency(void);
void OSD_DISPLAY(const int T, const char C);
void ChangeSVModeOption(uint8_t num);
void ChangeAVModeOption(uint8_t num);
void Osd_Display(uint8_t start, const char str[]);
void OSD_writeString(int startPos, int row, const char *str);

// Main IR and Menu Handlers
void OSD_selectOption(void);
void OSD_menu_F(char incomingByte);
void OSD_IR(void);

// Menu Handler Functions (handle_0 through handle_z, handle_A, and special characters)
void handle_0(void);
void handle_1(void);
void handle_2(void);
void handle_3(void);
void handle_4(void);
void handle_5(void);
void handle_6(void);
void handle_7(void);
void handle_8(void);
void handle_9(void);
void handle_a(void);
void handle_b(void);
void handle_c(void);
void handle_d(void);
void handle_e(void);
void handle_f(void);
void handle_g(void);
void handle_h(void);
void handle_i(void);
void handle_j(void);
void handle_k(void);
void handle_l(void);
void handle_m(void);
void handle_n(void);
void handle_o(void);
void handle_p(void);
void handle_q(void);
void handle_r(void);
void handle_s(void);
void handle_t(void);
void handle_u(void);
void handle_v(void);
void handle_w(void);
void handle_x(void);
void handle_y(void);
void handle_z(void);
void handle_A(void);
void handle_caret(void);
void handle_at(void);
void handle_exclamation(void);
void handle_hash(void);
void handle_dollar(void);
void handle_percent(void);
void handle_ampersand(void);
void handle_asterisk(void);
