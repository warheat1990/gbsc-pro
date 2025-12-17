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
    OLED_None,                                        // No menu/OSD closed

    // Main menu (top level)
    OLED_Input,                                       // Input menu
    OLED_OutputResolution,                            // Output Resolution menu
    OLED_ScreenSettings,                              // Screen Settings
    OLED_ColorSettings,                               // Color Settings
    OLED_SystemSettings,                              // System Settings
    OLED_ResetSettings,                               // Reset default

    // Input menu
    OLED_Input_RGBs,                                  // Input RGBs
    OLED_Input_RGsB,                                  // Input RGsB
    OLED_Input_VGA,                                   // Input VGA
    OLED_Input_YPBPR,                                 // Input YPbPr
    OLED_Input_SV,                                    // Input S-Video
    OLED_Input_AV,                                    // Input AV

    // Resolution menu
    OLED_OutputResolution_1080,                       // 1920x1080
    OLED_OutputResolution_1024,                       // 1280x1024
    OLED_OutputResolution_960,                        // 1280x960
    OLED_OutputResolution_720,                        // 1280x720
    OLED_OutputResolution_480,                        // 480p/576p
    // OLED_OutputResolution_Downscale,               // Downscale
    OLED_OutputResolution_PassThrough,                // Pass-through

    // Screen Settings submenu
    OLED_ScreenSettings_Move,                         // Move
    OLED_ScreenSettings_Scale,                        // Scale
    OLED_ScreenSettings_Borders,                      // Borders
    OLED_ScreenSettings_MoveAdjust,                   // Move adjust
    OLED_ScreenSettings_ScaleAdjust,                  // Scale adjust
    OLED_ScreenSettings_BordersAdjust,                // Borders adjust

    // Color Settings submenu
    OLED_ColorSettings_ADCGain,                       // ADC Gain
    OLED_ColorSettings_Scanlines,                     // Scanlines
    OLED_ColorSettings_LineFilter,                    // Line Filter
    OLED_ColorSettings_Sharpness,                     // Sharpness
    OLED_ColorSettings_Peaking,                       // Peaking
    OLED_ColorSettings_StepResponse,                  // Step Response
    OLED_ColorSettings_RGB_R,                         // RGB Red channel
    OLED_ColorSettings_RGB_G,                         // RGB Green channel
    OLED_ColorSettings_RGB_B,                         // RGB Blue channel
    OLED_ColorSettings_Y_Gain,                        // Y Gain
    OLED_ColorSettings_Color,                         // Color
    OLED_ColorSettings_DefaultColor,                  // Default Color

    // System Settings submenu
    OLED_SystemSettings_MatchedPresets,               // Matched Presets
    OLED_ScreenSettings_FullHeight,                   // Full Height
    OLED_SystemSettings_UseUpscaling,                 // Use upscaling
    // OLED_SystemSettings_ComponentVGA,              // Component/VGA settings
    OLED_SystemSettings_Force5060Hz,                  // Force 50/60Hz
    OLED_SystemSettings_ClockGenerator,               // Clock Generator
    OLED_SystemSettings_ADCCalibration,               // ADC Calibration
    OLED_SystemSettings_FrameTimeLock,                // Frame Time Lock
    OLED_SystemSettings_LockMethod,                   // Lock Method
    OLED_SystemSettings_Deinterlace,                  // Deinterlace
    OLED_SystemSettings_Compatibility,                // Compatibility

    // SV/AV Input settings
    OLED_SystemSettings_SVAVInputSettings,            // SV/AV Input settings
    OLED_SystemSettings_SVAVInput_DoubleLine,         // Double line
    OLED_SystemSettings_SVAVInput_Smooth,             // Smooth
    OLED_SystemSettings_SVAVInput_Bright,             // Brightness
    OLED_SystemSettings_SVAVInput_Contrast,           // Contrast
    OLED_SystemSettings_SVAVInput_Saturation,         // Saturation
    OLED_SystemSettings_SVAVInput_Default,            // Default

    OLED_Volume_Adjust,                               // Volume adjustment screen
    OLED_RetainedSettings,                            // Retained settings
    OLED_Info_Display,                                // Info display screen
    // OLED_FreezeCapture,                            // Freeze capture
    OLED_EnableOTA,                                   // Enable OTA update
    OLED_Restart,                                     // Restart menu
    OLED_ResetDefaults,                               // Reset to factory defaults

    // Disabled
    // OLED_Developer,                                // Developer menu
    // OLED_MemoryAdjust,                             // Memory adjustment (MEM left/right)
    // OLED_HSyncAdjust,                              // HSync adjustment (HS left/right)
    // OLED_HTotalAdjust,                             // Horizontal total adjustment (HTotal -/+)
    // OLED_DebugView,                                // Debug view
    // OLED_ADCFilter,                                // ADC filter settings

    // Profile/Slot Management
    OLED_Profile,                                     // Profile main menu
    OLED_Profile_SaveConfirm,                         // Save confirmation
    OLED_Profile_Save,                                // Save profile
    OLED_Profile_Load,                                // Load profile
    OLED_Profile_Operation1,                          // Profile operation 1
    OLED_Profile_Operation2,                          // Profile operation 2
    OLED_Profile_Operation3,                          // Profile operation 3
    OLED_Profile_SelectSlot,                          // Slot selection menu
    OLED_Profile_Slot1,                               // Profile slot 1
    OLED_Profile_Slot2,                               // Profile slot 2
    OLED_Profile_Slot3,                               // Profile slot 3
    OLED_Profile_Slot4,                               // Profile slot 4
    OLED_Profile_Slot5,                               // Profile slot 5
    OLED_Profile_Slot6,                               // Profile slot 6
    OLED_Profile_Slot7,                               // Profile slot 7
    OLED_Profile_Slot8,                               // Profile slot 8
    OLED_Profile_Slot9,                               // Profile slot 9
    OLED_Profile_Slot10,                              // Profile slot 10
    OLED_Profile_Slot11,                              // Profile slot 11
    OLED_Profile_Slot12,                              // Profile slot 12
    OLED_Profile_Slot13,                              // Profile slot 13
    OLED_Profile_Slot14,                              // Profile slot 14
    OLED_Profile_Slot15,                              // Profile slot 15
    OLED_Profile_Slot16,                              // Profile slot 16
    OLED_Profile_Slot17,                              // Profile slot 17
    OLED_Profile_Slot18,                              // Profile slot 18
    OLED_Profile_Slot19,                              // Profile slot 19
    OLED_Profile_SelectPreset,                        // Preset selection menu
    OLED_Profile_Preset1,                             // Profile preset 1
    OLED_Profile_Preset2,                             // Profile preset 2
    OLED_Profile_Preset3,                             // Profile preset 3
    OLED_Profile_Preset4,                             // Profile preset 4
    OLED_Profile_Preset5,                             // Profile preset 5
    OLED_Profile_Preset6,                             // Profile preset 6
    OLED_Profile_Preset7,                             // Profile preset 7
    OLED_Profile_Preset8,                             // Profile preset 8
    OLED_Profile_Preset9,                             // Profile preset 9
    OLED_Profile_Preset10,                            // Profile preset 10
    OLED_Profile_Preset11,                            // Profile preset 11
    OLED_Profile_Preset12,                            // Profile preset 12
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

// Status Packet
extern const size_t PRO_STATUS_MESSAGE_LEN;

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
const char* proStatusPacket();
bool isPeakingLocked(void);
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
