// ====================================================================================
// gbs-control-pro.cpp
// GBSC-Pro Core Implementation
//
// This file contains:
// - Global variable definitions for Pro features
// - ADV controller communication (ADVController)
// - ADV7280/ADV7391 processor control
// - Input source switching
// - Color conversion utilities (RGB <-> YUV)
// - Video mode management
// - OSD TV display helper functions
//
// See also:
// - menu/menu-core.cpp: Menu navigation, IR dispatch, OLED handlers
// - osd/osd-core.cpp: OSD dispatch table, command handler
// - menu/*.cpp: IR menu handlers per section
// - osd/*.cpp: OSD rendering handlers per section
// ====================================================================================

#include "gbs-control-pro.h"
#include "menu/menu-core.h"
#include "osd/osd-core.h"
#include "../tv5725.h"
#include "../osd.h"
#include "../OLEDMenuImplementation.h"
#include "../ntsc_720x480.h"
#include "../src/WebSocketsServer.h"

#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <SSD1306Wire.h>

#include "drivers/ir_remote.h"
#include "drivers/stv9426.h"
#include "drivers/pt2257.h"
#include "drivers/adv_controller.h"

// ====================================================================================
// External References - gbs-control.ino
// ====================================================================================

typedef TV5725<GBS_ADDR> GBS;
extern struct runTimeOptions *rto;
extern struct userOptions *uopt;
extern char serialCommand;
extern char userCommand;
extern OLEDMenuManager oledMenu;
extern SSD1306Wire display;

extern uint8_t getVideoMode();
extern void applyPresets(uint8_t videoMode);
extern bool loadSlotSettings();
extern void setOutModeHdBypass(bool);
extern void saveUserPrefs();
extern void resetSyncProcessor();
extern float getOutputFrameRate();

// ====================================================================================
// Global Variables - IR Remote
// ====================================================================================

static const int kRecvPin = 2;
IRrecv irrecv(kRecvPin);
decode_results results;

// ====================================================================================
// Global Variables - Timing
// ====================================================================================

bool irDecodedFlag = false;
unsigned long lastSignalTime = 0;
unsigned long lastSystemTime = 0;
unsigned long lastWebUpdateTime = 0;
unsigned long lastMenuItemTime = 0;
unsigned long lastResolutionTime = 0;
unsigned long resolutionStartTime = 0;

// ====================================================================================
// Global Variables - OLED Menu State
// ====================================================================================

int oled_menuItem = OLED_None;
int lastOledMenuItem = 0;
uint8_t oledClearFlag = 0;
boolean NEW_OLED_MENU = false;
int selectedMenuLine = 0;

// ====================================================================================
// Global Variables - TV OSD State
// ====================================================================================

char osdDisplayValue = 0;
boolean irEnabled = true;
uint8_t menuLineColors[OSD_MAX_MENU_ROWS] = {OSD_TEXT_NORMAL, OSD_TEXT_NORMAL, OSD_TEXT_NORMAL};
uint8_t isInfoDisplayActive = 0;
uint16_t horizontalBlankStart = 0;
uint16_t horizontalBlankStop = 0;

// ====================================================================================
// Global Variables - Picture Settings (per-slot, stored in SlotMeta)
// ====================================================================================

uint8_t gbsColorR = 128;       // GBS TV5725 color balance R
uint8_t gbsColorG = 128;       // GBS TV5725 color balance G
uint8_t gbsColorB = 128;       // GBS TV5725 color balance B
uint8_t advBrightness = 128;   // ADV7280 brightness
uint8_t advContrast = 128;     // ADV7280 contrast
uint8_t advSaturation = 128;   // ADV7280 saturation

// ====================================================================================
// Global Variables - Video Mode Options
// ====================================================================================

uint8_t svVideoFormatChanged = 0;       // Flag: S-Video format changed, needs ADV update
uint8_t avVideoFormatChanged = 0;       // Flag: Composite format changed, needs ADV update
uint8_t advSmooth = 0;                  // ADV7280 smooth interpolation (per-slot)
uint8_t advI2P = 0;                     // ADV7280 I2P - interlace to progressive (per-slot)
uint8_t advACE = 0;                     // ADV7280 ACE - Adaptive Contrast Enhancement (per-slot)

// ====================================================================================
// Global Variables - Resolution Settings
// ====================================================================================

uint8_t keepSettings = 0;
uint8_t tentativeResolution = 0;

// ====================================================================================
// Global Variables - Factory Reset
// ====================================================================================

uint8_t factoryResetSelection = 0;  // 0 = No (default), 1 = Yes

// ====================================================================================
// ADV Communication - Packet Sender Instance
// ====================================================================================

static ADVController advController;

// ====================================================================================
// ADV Packet Wrappers
// ====================================================================================

// Send packet only (no save)
static void ADV_send(const unsigned char* packet) {
    advController.send(packet);
}

// Send packet and save preferences
static void ADV_sendAndSave(const unsigned char* packet) {
    advController.send(packet);
    saveUserPrefs();
}

void ADV_sendI2P(bool enable) {
    ADV_sendAndSave(enable ? ADV_I2P_On : ADV_I2P_Off);
}

void ADV_sendSmooth(bool enable) {
    ADV_sendAndSave(enable ? ADV_Smooth_On : ADV_Smooth_Off);
}

void ADV_sendCompatibility(bool mode) {
    ADV_sendAndSave(mode ? ADV_Compatibility_Off : ADV_Compatibility_On);
}

void ADV_sendACE(bool enable) {
    ADV_sendAndSave(enable ? ADV_ACE_On : ADV_ACE_Off);
}

void ADV_sendVideoFormat(uint8_t format) {
    unsigned char packet[4] = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_TVMODE, format};
    ADV_sendAndSave(packet);
}

void ADV_sendBCSH(unsigned char reg, unsigned char val) {
    advController.writeReg(ADV_BCSH, reg, val);
}

void ADV_sendCustomI2C(const unsigned char* data, size_t size) {
    advController.sendCustomI2C(data, size);
}

// Apply video format option with bounds checking
// Returns true if format was changed (triggers picture settings re-apply)
static bool ADV_applyVideoFormatOption(uint8_t* changed, uint8_t option) {
    static const uint8_t max_index = sizeof(ADV_VideoFormats) / sizeof(ADV_VideoFormats[0]) - 1;
    if (*changed) {
        *changed = 0;
        uint8_t idx = (option <= max_index) ? option : 1;  // Default to PAL if invalid
        ADV_sendVideoFormat(ADV_VideoFormats[idx]);
        return true;
    }
    return false;
}

// Apply per-slot ADV settings (line double, smooth, BCSH)
// These are loaded from SlotMeta and stored in global variables
// Only applies when input is SV or AV (ADV7280 is only used for these inputs)
void ADV_applySlotSettings(void) {
    if (uopt->activeInputType != InputTypeSV && uopt->activeInputType != InputTypeAV) {
        return;  // ADV7280 not in use for this input type
    }
    ADV_send(advI2P ? ADV_I2P_On : ADV_I2P_Off);
    ADV_send(advSmooth ? ADV_Smooth_On : ADV_Smooth_Off);
    ADV_send(advACE ? ADV_ACE_On : ADV_ACE_Off);
    advController.writeReg(ADV_BCSH, 0x0A, advBrightness - 128);
    advController.writeReg(ADV_BCSH, 0x08, advContrast);
    advController.writeReg(ADV_BCSH, 0xE3, advSaturation);
}

void ADV_applyPendingOptions(void)
{
    // Check if video format changed (ADV reinitializes, need to re-apply picture settings)
    bool formatChanged = false;
    formatChanged |= ADV_applyVideoFormatOption(&svVideoFormatChanged, uopt->svVideoFormat);
    formatChanged |= ADV_applyVideoFormatOption(&avVideoFormatChanged, uopt->avVideoFormat);

    // After format change, re-apply per-slot ADV settings
    if (formatChanged) {
        ADV_applySlotSettings();
    }
}

// ====================================================================================
// Input Source Switching - Configuration Table
// ====================================================================================

// Flags for InputConfig
#define INPUT_FLAG_NONE         0x00
#define INPUT_FLAG_CONFIG_ADC   0x01  // Configure ADC registers
#define INPUT_FLAG_HV_ENABLE    0x02  // Use HV_Enable instead of HV_Disable
#define INPUT_FLAG_LOW_POWER    0x04  // Clear isInLowPowerMode

struct InputConfig {
    const unsigned char* packet;
    uint8_t type;           // InputType value (stored in uopt->activeInputType)
    uint8_t bcshMode;       // bcshAdjustMode value (stored in uopt->bcshAdjustMode)
    uint8_t adcValue;       // ADC_SOGEN/ADC_INPUT_SEL value (RGB1 or YUV0)
    uint8_t flags;
};

// Input configuration lookup table (indexed by InputType - 1)
static const InputConfig inputConfigs[] = {
    // packet,         type,          bcshMode, adc,  flags
    {ADV_InputRGBs,  InputTypeRGBs, 0, RGB1, INPUT_FLAG_CONFIG_ADC},                              // 0: RGBs
    {ADV_InputRGsB,  InputTypeRGsB, 0, RGB1, INPUT_FLAG_CONFIG_ADC},                              // 1: RGsB
    {ADV_InputVGA,   InputTypeVGA,  0, RGB1, INPUT_FLAG_CONFIG_ADC | INPUT_FLAG_HV_ENABLE},       // 2: VGA
    {ADV_InputYpbpr, InputTypeYUV,  1, 0,    INPUT_FLAG_NONE},                                    // 3: YPbPr
    {ADV_InputSV,    InputTypeSV,   2, YUV0, INPUT_FLAG_CONFIG_ADC | INPUT_FLAG_LOW_POWER},       // 4: SV
    {ADV_InputAV,    InputTypeAV,   2, YUV0, INPUT_FLAG_CONFIG_ADC | INPUT_FLAG_LOW_POWER},       // 5: AV
};

// Config indices for clarity
#define INPUT_CFG_RGBS  0
#define INPUT_CFG_RGSB  1
#define INPUT_CFG_VGA   2
#define INPUT_CFG_YUV   3
#define INPUT_CFG_SV    4
#define INPUT_CFG_AV    5

// Unified input switching function
// mode: -1 = use send(), >= 0 = use sendWithMode(mode)
static void switchInput(const InputConfig& cfg, int8_t mode = -1) {
    // Send packet to ADV
    if (mode < 0) {
        advController.send(cfg.packet);
    } else {
        advController.sendWithMode(cfg.packet, mode);
    }

    // Set input state (using uopt->activeInputType)
    uopt->activeInputType = cfg.type;
    resetSyncProcessor();

    // Configure ADC if needed
    if (cfg.flags & INPUT_FLAG_CONFIG_ADC) {
        GBS::ADC_SOGEN::write(cfg.adcValue);
        GBS::SP_EXT_SYNC_SEL::write((cfg.flags & INPUT_FLAG_HV_ENABLE) ? HV_Enable : HV_Disable);
        GBS::ADC_INPUT_SEL::write(cfg.adcValue);
    }

    // Set BCSH mode and disconnect state
    uopt->bcshAdjustMode = cfg.bcshMode;
    rto->sourceDisconnected = true;

    // Clear low power mode if needed
    if (cfg.flags & INPUT_FLAG_LOW_POWER) {
        rto->isInLowPowerMode = false;
    }

    // Reset colors to defaults for input type (RGB vs YUV/Component)
    if (cfg.bcshMode == 0) {
        gbsColorR = gbsColorG = gbsColorB = 128;
    } else {
        gbsColorR = 146;
        gbsColorG = 138;
        gbsColorB = 148;
    }
    applyRGBtoYUVConversion();

    saveUserPrefs();
}

// ====================================================================================
// Input Source Switching - Public Functions
// ====================================================================================

// Basic functions (no mode parameter)
void InputRGBs(void)            { switchInput(inputConfigs[INPUT_CFG_RGBS]); }
void InputRGsB(void)            { switchInput(inputConfigs[INPUT_CFG_RGSB]); }
void InputVGA(void)             { switchInput(inputConfigs[INPUT_CFG_VGA], 1); }
void InputYUV(void)             { switchInput(inputConfigs[INPUT_CFG_YUV]); }
void InputSV(void)              { switchInput(inputConfigs[INPUT_CFG_SV]); }
void InputAV(void)              { switchInput(inputConfigs[INPUT_CFG_AV]); }

// Functions with mode parameter (note: InputYUV_mode doesn't exist)
void InputRGBs_mode(uint8_t m)  { switchInput(inputConfigs[INPUT_CFG_RGBS], !m); }
void InputRGsB_mode(uint8_t m)  { switchInput(inputConfigs[INPUT_CFG_RGSB], !m); }
void InputVGA_mode(uint8_t m)   { switchInput(inputConfigs[INPUT_CFG_VGA], !m); }
void InputSV_mode(uint8_t m)    { switchInput(inputConfigs[INPUT_CFG_SV], m); }
void InputAV_mode(uint8_t m)    { switchInput(inputConfigs[INPUT_CFG_AV], m); }

// ====================================================================================
// Input Source Switching - Boot/Restore Function
// ====================================================================================

// Apply hardware config from InputConfig (no packet send, no saveUserPrefs)
static void applyInputHardwareConfig(const InputConfig& cfg) {
    resetSyncProcessor();
    if (cfg.flags & INPUT_FLAG_CONFIG_ADC) {
        GBS::ADC_SOGEN::write(cfg.adcValue);
        GBS::SP_EXT_SYNC_SEL::write((cfg.flags & INPUT_FLAG_HV_ENABLE) ? HV_Enable : HV_Disable);
        GBS::ADC_INPUT_SEL::write(cfg.adcValue);
    }
    uopt->bcshAdjustMode = cfg.bcshMode;
    rto->sourceDisconnected = true;
}

void applySavedInputSource(void) {
    // Apply saved input source configuration to hardware at startup
    // uopt->activeInputType is already loaded from preferences

    // Validate activeInputType and get config index
    uint8_t cfgIndex;
    switch (uopt->activeInputType) {
        case InputTypeRGBs: cfgIndex = INPUT_CFG_RGBS; break;
        case InputTypeRGsB: cfgIndex = INPUT_CFG_RGSB; break;
        case InputTypeVGA:  cfgIndex = INPUT_CFG_VGA;  break;
        case InputTypeYUV:  cfgIndex = INPUT_CFG_YUV;  break;
        case InputTypeSV:   cfgIndex = INPUT_CFG_SV;   break;
        case InputTypeAV:   cfgIndex = INPUT_CFG_AV;   break;
        default:
            // Invalid activeInputType, default to RGBs
            uopt->activeInputType = InputTypeRGBs;
            cfgIndex = INPUT_CFG_RGBS;
            break;
    }

    const InputConfig& cfg = inputConfigs[cfgIndex];

    // SV/AV need ADV packet at boot (ADV controller needs to know input type)
    if (uopt->activeInputType == InputTypeSV || uopt->activeInputType == InputTypeAV) {
        advController.send(cfg.packet);
    }

    applyInputHardwareConfig(cfg);
}

// ====================================================================================
// OLED Menu Helper Functions
// ====================================================================================

void resetOLEDScreenSaverTimer() {
    oledMenu.resetScreenSaverTimer();
}

// ====================================================================================
// Color Conversion Functions (ITU-R BT.601)
// ====================================================================================

void applyRGBtoYUVConversion(void)
{
    int r = gbsColorR - 128;
    int g = gbsColorG - 128;
    int b = gbsColorB - 128;

    GBS::VDS_Y_OFST::write(constrain(0.299f * r + 0.587f * g + 0.114f * b, -128, 127));
    GBS::VDS_U_OFST::write(constrain(-0.14713f * r - 0.28886f * g + 0.436f * b, -128, 127));
    GBS::VDS_V_OFST::write(constrain(0.615f * r - 0.51499f * g - 0.10001f * b, -128, 127));
}

void readYUVtoRGBConversion(void)
{
    int8_t y = (int8_t)GBS::VDS_Y_OFST::read();
    int8_t u = (int8_t)GBS::VDS_U_OFST::read();
    int8_t v = (int8_t)GBS::VDS_V_OFST::read();

    gbsColorR = constrain(y + 1.13983f * v + 128, 0, 255);
    gbsColorG = constrain(y - 0.39465f * u - 0.58060f * v + 128, 0, 255);
    gbsColorB = constrain(y + 2.03211f * u + 128, 0, 255);
}

// ====================================================================================
// Video Mode Functions
// ====================================================================================

void applyVideoModePreset(void)
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
        setOutModeHdBypass(false);
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

boolean hasOutputFrequencyChanged()
{
    unsigned char freq = 0;
    static unsigned char freq_last;
    freq = getOutputFrameRate();
    if ((abs(freq_last - freq) < 9) || (freq_last == 0)) {
        freq_last = freq;
        return 0;
    }
    freq_last = freq;
    return 1;
}

// ====================================================================================
// Status Functions
// ====================================================================================

bool isPeakingLocked(void) {
    return (GBS::VDS_PK_LB_GAIN::read() != 0x16);
}

void broadcastProStatus(WebSocketsServer& ws)
{
    constexpr size_t MESSAGE_LEN = 7;
    char buffer[MESSAGE_LEN];
    buffer[0] = '$';

    // activeInputType is already 1-6, use it directly
    uint8_t currentInputType = uopt->activeInputType;
    if (currentInputType < 1 || currentInputType > 6) {
        currentInputType = 1;  // Default to RGBs if invalid
    }

    buffer[1] = '0' + currentInputType;

    uint8_t format = (uint8_t)uopt->TVMODE_presetPreference;
    if (format > 11) {
        format = 0;
    }
    if (format <= 9) {
        buffer[2] = '0' + format;
    } else if (format == 10) {
        buffer[2] = 'A';
    } else {
        buffer[2] = 'B';
    }

    buffer[3] = '0' + (advI2P ? 1 : 0);
    buffer[4] = '0' + (advSmooth ? 1 : 0);
    buffer[5] = isPeakingLocked() ? '1' : '0';
    buffer[6] = '0' + (advACE ? 1 : 0);

    ws.broadcastTXT(buffer, MESSAGE_LEN);
}

// ====================================================================================
// Menu Refresh on Signal Change
// ====================================================================================

void refreshMenusOnSignalChange()
{
    static uint8_t prevVideoStandardInput = 0;
    static uint16_t prevVPERIOD_IF = 0;
    static boolean prevOutModeHdBypass = false;
    static uint8_t prevDeintMode = 0;

    uint16_t currentVPERIOD_IF = GBS::VPERIOD_IF::read();
    boolean signalChanged = (rto->videoStandardInput != prevVideoStandardInput ||
                             currentVPERIOD_IF != prevVPERIOD_IF ||
                             rto->outModeHdBypass != prevOutModeHdBypass ||
                             uopt->deintMode != prevDeintMode);

    if (signalChanged) {
        prevVideoStandardInput = rto->videoStandardInput;
        prevVPERIOD_IF = currentVPERIOD_IF;
        prevOutModeHdBypass = rto->outModeHdBypass;
        prevDeintMode = uopt->deintMode;
        // Refresh OLED display
        oledClearFlag = ~0;
        if (oled_menuItem != OLED_None) {
            OLED_MenuState current = (OLED_MenuState)oled_menuItem;
            oled_menuItem = OLED_None;
            Menu_navigateTo(current);
        }
    }
}
