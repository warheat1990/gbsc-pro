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
extern bool areScanLinesAllowed();

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
// Global Variables - Video Mode Flags (runtime only, not persisted)
// ====================================================================================

uint8_t svVideoFormatChanged = 0;       // Flag: S-Video format changed, needs ADV update
uint8_t avVideoFormatChanged = 0;       // Flag: Composite format changed, needs ADV update

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
// ADV Command Functions
// ====================================================================================

void ADV_sendI2P(bool enable) {
    advController.send(enable ? ADV_I2P_On : ADV_I2P_Off);
}

void ADV_sendSmooth(bool enable) {
    advController.send(enable ? ADV_Smooth_On : ADV_Smooth_Off);
}

void ADV_sendSyncStripper(bool mode) {
    advController.send(mode ? ADV_SyncStripper_On : ADV_SyncStripper_Off);
}

void ADV_sendACE(bool enable) {
    advController.send(enable ? ADV_ACE_On : ADV_ACE_Off);
}

void ADV_sendVideoFormat(uint8_t format) {
    unsigned char packet[4] = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_TVMODE, format};
    advController.send(packet);
}

void ADV_sendBCSH(unsigned char reg, unsigned char val) {
    advController.writeReg(ADV_BCSH, reg, val);
}

void ADV_sendCustomI2C(const unsigned char* data, size_t size) {
    advController.sendCustomI2C(data, size);
}

// ====================================================================================
// ACE Parameter Functions
// ====================================================================================

void ADV_sendACELumaGain(uint8_t gain) {
    advController.writeReg(ADV_ACE_Param, ADV_ACE_LUMA_GAIN, gain);
}

void ADV_sendACEChromaGain(uint8_t gain) {
    advController.writeReg(ADV_ACE_Param, ADV_ACE_CHROMA_GAIN, gain);
}

void ADV_sendACEChromaMax(uint8_t max) {
    advController.writeReg(ADV_ACE_Param, ADV_ACE_CHROMA_MAX, max);
}

void ADV_sendACEGammaGain(uint8_t gain) {
    advController.writeReg(ADV_ACE_Param, ADV_ACE_GAMMA_GAIN, gain);
}

void ADV_sendACEResponseSpeed(uint8_t speed) {
    advController.writeReg(ADV_ACE_Param, ADV_ACE_RESPONSE_SPEED, speed);
}

void ADV_sendACEDefaults(void) {
    advController.send(ADV_ACE_Defaults);
    uopt->advACELumaGain = ADV_ACE_LUMA_GAIN_DEFAULT;
    uopt->advACEChromaGain = ADV_ACE_CHROMA_GAIN_DEFAULT;
    uopt->advACEChromaMax = ADV_ACE_CHROMA_MAX_DEFAULT;
    uopt->advACEGammaGain = ADV_ACE_GAMMA_GAIN_DEFAULT;
    uopt->advACEResponseSpeed = ADV_ACE_RESPONSE_SPEED_DEFAULT;
}

void ADV_sendACEParams(void) {
    ADV_sendACELumaGain(uopt->advACELumaGain);
    ADV_sendACEChromaGain(uopt->advACEChromaGain);
    ADV_sendACEChromaMax(uopt->advACEChromaMax);
    ADV_sendACEGammaGain(uopt->advACEGammaGain);
    ADV_sendACEResponseSpeed(uopt->advACEResponseSpeed);
}

// ====================================================================================
// Video Filter Parameter Functions
// ====================================================================================

void ADV_sendFilterYShaping(uint8_t filter) {
    advController.writeReg(ADV_VideoFilter_Param, ADV_FILTER_Y_SHAPING, filter);
}

void ADV_sendFilterCShaping(uint8_t filter) {
    advController.writeReg(ADV_VideoFilter_Param, ADV_FILTER_C_SHAPING, filter);
}

void ADV_sendFilterWYShaping(uint8_t filter) {
    advController.writeReg(ADV_VideoFilter_Param, ADV_FILTER_WY_SHAPING, filter);
}

void ADV_sendFilterWYOverride(uint8_t ovr) {
    advController.writeReg(ADV_VideoFilter_Param, ADV_FILTER_WY_OVERRIDE, ovr);
}

void ADV_sendFilterCombNTSC(uint8_t bw) {
    advController.writeReg(ADV_VideoFilter_Param, ADV_FILTER_COMB_NTSC, bw);
}

void ADV_sendFilterCombPAL(uint8_t bw) {
    advController.writeReg(ADV_VideoFilter_Param, ADV_FILTER_COMB_PAL, bw);
}

void ADV_sendVideoFiltersDefaults(void) {
    // Send Video Filters Defaults command to ADV controller
    advController.send(ADV_VideoFilter_Defaults);
    // Reset filter settings
    uopt->advFilterYShaping = ADV_FILTER_Y_SHAPING_DEFAULT;
    uopt->advFilterCShaping = ADV_FILTER_C_SHAPING_DEFAULT;
    uopt->advFilterWYShaping = ADV_FILTER_WY_SHAPING_DEFAULT;
    uopt->advFilterWYOverride = ADV_FILTER_WY_OVERRIDE_DEFAULT;
    uopt->advFilterCombNTSC = ADV_FILTER_COMB_NTSC_DEFAULT;
    uopt->advFilterCombPAL = ADV_FILTER_COMB_PAL_DEFAULT;
    uopt->advCombLumaModeNTSC = ADV_COMB_LUMA_MODE_NTSC_DEFAULT;
    uopt->advCombChromaModeNTSC = ADV_COMB_CHROMA_MODE_NTSC_DEFAULT;
    uopt->advCombChromaTapsNTSC = ADV_COMB_CHROMA_TAPS_NTSC_DEFAULT;
    uopt->advCombLumaModePAL = ADV_COMB_LUMA_MODE_PAL_DEFAULT;
    uopt->advCombChromaModePAL = ADV_COMB_CHROMA_MODE_PAL_DEFAULT;
    uopt->advCombChromaTapsPAL = ADV_COMB_CHROMA_TAPS_PAL_DEFAULT;
}

// ====================================================================================
// Comb Control Parameter Functions (individual setters)
// ====================================================================================

void ADV_sendCombLumaModeNTSC(uint8_t mode) {
    advController.writeReg(ADV_VideoFilter_Param, ADV_COMB_LUMA_MODE_NTSC, mode);
}

void ADV_sendCombChromaModeNTSC(uint8_t mode) {
    advController.writeReg(ADV_VideoFilter_Param, ADV_COMB_CHROMA_MODE_NTSC, mode);
}

void ADV_sendCombChromaTapsNTSC(uint8_t taps) {
    advController.writeReg(ADV_VideoFilter_Param, ADV_COMB_CHROMA_TAPS_NTSC, taps);
}

void ADV_sendCombLumaModePAL(uint8_t mode) {
    advController.writeReg(ADV_VideoFilter_Param, ADV_COMB_LUMA_MODE_PAL, mode);
}

void ADV_sendCombChromaModePAL(uint8_t mode) {
    advController.writeReg(ADV_VideoFilter_Param, ADV_COMB_CHROMA_MODE_PAL, mode);
}

void ADV_sendCombChromaTapsPAL(uint8_t taps) {
    advController.writeReg(ADV_VideoFilter_Param, ADV_COMB_CHROMA_TAPS_PAL, taps);
}

// ====================================================================================
// Video Filters Function
// ====================================================================================

void ADV_sendVideoFilters(void) {
    ADV_sendFilterYShaping(uopt->advFilterYShaping);
    ADV_sendFilterCShaping(uopt->advFilterCShaping);
    ADV_sendFilterWYShaping(uopt->advFilterWYShaping);
    ADV_sendFilterWYOverride(uopt->advFilterWYOverride);
    ADV_sendFilterCombNTSC(uopt->advFilterCombNTSC);
    ADV_sendFilterCombPAL(uopt->advFilterCombPAL);
    ADV_sendCombLumaModeNTSC(uopt->advCombLumaModeNTSC);
    ADV_sendCombChromaModeNTSC(uopt->advCombChromaModeNTSC);
    ADV_sendCombChromaTapsNTSC(uopt->advCombChromaTapsNTSC);
    ADV_sendCombLumaModePAL(uopt->advCombLumaModePAL);
    ADV_sendCombChromaModePAL(uopt->advCombChromaModePAL);
    ADV_sendCombChromaTapsPAL(uopt->advCombChromaTapsPAL);
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

// Apply per-slot ADV settings (I2P, Smooth, ACE, BCSH, Video Filters)
// These are loaded from SlotMeta and stored in uopt
// Only applies when input is SV or AV (ADV7280 is only used for these inputs)
void ADV_applySlotSettings(void) {
    if (uopt->activeInputType != InputTypeSV && uopt->activeInputType != InputTypeAV) {
        return;  // ADV7280 not in use for this input type
    }
    ADV_sendI2P(uopt->advI2P);
    ADV_sendSmooth(uopt->advSmooth);
    ADV_sendACE(uopt->advACE);
    advController.writeReg(ADV_BCSH, 0x0A, uopt->advBrightness - 128);
    advController.writeReg(ADV_BCSH, 0x08, uopt->advContrast);
    advController.writeReg(ADV_BCSH, 0xE3, uopt->advSaturation);  // SD_SAT_Cb
    advController.writeReg(ADV_BCSH, 0xE4, uopt->advSaturation);  // SD_SAT_Cr
    advController.writeReg(ADV_BCSH, 0x0B, uopt->advHue - 128);
    // Apply ACE parameters
    ADV_sendACEParams();
    // Apply Video Filter parameters
    ADV_sendVideoFilters();
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
    // YUV/Component defaults (129,123,132) produce Y=-2, U=3, V=3 matching applyYuvPatches()
    if (cfg.bcshMode == 0) {
        uopt->gbsColorR = uopt->gbsColorG = uopt->gbsColorB = 128;
    } else {
        uopt->gbsColorR = 129;
        uopt->gbsColorG = 123;
        uopt->gbsColorB = 132;
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
    int r = uopt->gbsColorR - 128;
    int g = uopt->gbsColorG - 128;
    int b = uopt->gbsColorB - 128;

    GBS::VDS_Y_OFST::write(constrain(0.299f * r + 0.587f * g + 0.114f * b, -128, 127));
    GBS::VDS_U_OFST::write(constrain(-0.14713f * r - 0.28886f * g + 0.436f * b, -128, 127));
    GBS::VDS_V_OFST::write(constrain(0.615f * r - 0.51499f * g - 0.10001f * b, -128, 127));
}

void readYUVtoRGBConversion(void)
{
    int8_t y = (int8_t)GBS::VDS_Y_OFST::read();
    int8_t u = (int8_t)GBS::VDS_U_OFST::read();
    int8_t v = (int8_t)GBS::VDS_V_OFST::read();

    uopt->gbsColorR = constrain(y + 1.13983f * v + 128, 0, 255);
    uopt->gbsColorG = constrain(y - 0.39465f * u - 0.58060f * v + 128, 0, 255);
    uopt->gbsColorB = constrain(y + 2.03211f * u + 128, 0, 255);
}

// ====================================================================================
// Video Mode Functions
// ====================================================================================

void applyVideoModePreset(void)
{
    uint8_t videoMode = getVideoMode();
    if (videoMode == 0 && (GBS::STATUS_SYNC_PROC_HSACT::read() || rto->noSignalBlackScreenMode)) {
        videoMode = rto->videoStandardInput; // in no-signal mode this is lastVideoStandard
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

// Helper to convert value 0-31 to hex char (0-9, A-V)
static char toHexChar32(uint8_t val) {
    if (val < 10) return '0' + val;
    return 'A' + (val - 10);
}

// Helper to convert value 0-15 to hex char (0-9, A-F)
static char toHexChar16(uint8_t val) {
    if (val < 10) return '0' + val;
    return 'A' + (val - 10);
}

void broadcastProStatus(WebSocketsServer& ws)
{
    // Extended message format:
    // $[input][format][i2p][smooth][sharpness][ace][lumaGain][chromaGain][chromaMax][gammaGain][responseSpeed]
    //  [yFilter][cFilter][wyFilter][wyOverride][comb][hdmiLimitedRange][syncStripper]
    //  [combLumaN][combChromaN][combTapsN][combLumaP][combChromaP][combTapsP][hue][scanLines]
    // Positions: 0=$ 1=input 2=format 3=i2p 4=smooth 5=sharpness 6=ace 7=luma 8=chroma 9=chromamax 10=gamma 11=response
    //            12=yFilter 13=cFilter 14=wyFilter 15=wyOverride 16=comb 17=hdmiLimitedRange 18=syncStripper
    //            19=combLumaN 20=combChromaN 21=combTapsN 22=combLumaP 23=combChromaP 24=combTapsP 25=hue 26=scanLines
    constexpr size_t MESSAGE_LEN = 27;
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

    buffer[3] = '0' + (uopt->advI2P ? 1 : 0);
    buffer[4] = '0' + (uopt->advSmooth ? 1 : 0);
    buffer[5] = isPeakingLocked() ? '1' : '0';
    buffer[6] = '0' + (uopt->advACE ? 1 : 0);

    // ACE parameters (hex encoded)
    buffer[7] = toHexChar32(uopt->advACELumaGain);       // 0-31 -> '0'-'9','A'-'V'
    buffer[8] = toHexChar16(uopt->advACEChromaGain);     // 0-15 -> '0'-'9','A'-'F'
    buffer[9] = toHexChar16(uopt->advACEChromaMax);      // 0-15 -> '0'-'9','A'-'F'
    buffer[10] = toHexChar16(uopt->advACEGammaGain);     // 0-15 -> '0'-'9','A'-'F'
    buffer[11] = toHexChar16(uopt->advACEResponseSpeed); // 0-15 -> '0'-'9','A'-'F'

    // Video Filter parameters (hex encoded)
    buffer[12] = toHexChar32(uopt->advFilterYShaping);    // 0-30 for AV Y filter
    buffer[13] = toHexChar16(uopt->advFilterCShaping);    // 0-7 for AV C filter
    buffer[14] = toHexChar16(uopt->advFilterWYShaping);   // 2-19 for SV WY filter (raw value)
    buffer[15] = '0' + (uopt->advFilterWYOverride ? 1 : 0); // 0=Auto, 1=Manual
    buffer[16] = toHexChar16(uopt->advFilterCombPAL);     // 0-3 unified comb filter
    buffer[17] = '0' + uopt->hdmiLimitedRange;            // 0-3 HDMI Limited Range
    buffer[18] = '0' + (uopt->advSyncStripper ? 1 : 0);   // 0=Off, 1=On
    buffer[19] = toHexChar16(uopt->advCombLumaModeNTSC);    // 0,4-7 Luma mode NTSC
    buffer[20] = toHexChar16(uopt->advCombChromaModeNTSC);  // 0,4-7 Chroma mode NTSC
    buffer[21] = toHexChar16(uopt->advCombChromaTapsNTSC);  // 0-3 Chroma taps NTSC
    buffer[22] = toHexChar16(uopt->advCombLumaModePAL);     // 0,4-7 Luma mode PAL
    buffer[23] = toHexChar16(uopt->advCombChromaModePAL);   // 0,4-7 Chroma mode PAL
    buffer[24] = toHexChar16(uopt->advCombChromaTapsPAL);   // 0-3 Chroma taps PAL
    buffer[25] = toHexChar32(uopt->advHue >> 3);            // 0-254 -> 0-31 -> hex char
    buffer[26] = areScanLinesAllowed() ? '1' : '0';

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
