// ====================================================================================
// gbs-control-pro.cpp
// GBSC-Pro Core Implementation
//
// This file contains:
// - Global variable definitions for Pro features
// - ADV controller communication (ADV_PacketSender)
// - ADV7280/ADV7391 processor control
// - Input source switching
// - Color conversion utilities (RGB <-> YUV)
// - Video mode management
// - OSD TV display helper functions
//
// See also:
// - ir-menu-pro.cpp: IR remote handling and menu navigation
// - osd-render-pro.cpp: TV OSD menu rendering and handlers
// - OLEDMenuImplementation-pro.cpp: OLED display menu handlers
// ====================================================================================

#include "gbs-control-pro.h"
#include "ir-menu-pro.h"
#include "osd-render-pro.h"
#include "tv5725.h"
#include "osd.h"
#include "SSD1306Wire.h"
#include "options.h"
#include "OLEDMenuImplementation.h"
#include "ntsc_720x480.h"
#include "src/WebSocketsServer.h"

#include <IRremoteESP8266.h>
#include <IRutils.h>

#include "OSD_TV/remote.h"
#include "OSD_TV/OSD_stv9426.h"
#include "OSD_TV/profile_name.h"
#include "OSD_TV/PT2257.h"

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
extern void setOutModeHdBypass(bool);
extern void saveUserPrefs();
extern void resetSyncProcessor();
extern float getOutputFrameRate();

// ====================================================================================
// Global Variables - OSD TV System (used by OSD_stv9426.h)
// ====================================================================================

// Current color for OSD character rendering
char currentColor;

// Current OSD row (ROW_1=0x00, ROW_2=0x02, ROW_3=0x03)
char currentRow;

// Digit display positions for 3-digit numbers (ones, tens, hundreds)
char digitPos1;  // Ones position
char digitPos2;  // Tens position
char digitPos3;  // Hundreds position

// Profile name characters (9 chars, default '.' = 0x2E)
char profileChars[9] = {0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E};

// ====================================================================================
// Global Variables - IR Remote
// ====================================================================================

static const int kRecvPin = 2;
IRrecv irrecv(kRecvPin);
decode_results results;

// ====================================================================================
// Global Variables - Input/Output State
// ====================================================================================

uint8_t inputSource = 0;
uint8_t inputType = 0;
uint8_t rgbComponentMode = 0;

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
char lastOsdCommand = 0;
boolean irEnabled = true;
int menuLine1Color = 0;
int menuLine2Color = 0;
int menuLine3Color = 0;
uint8_t isInfoDisplayActive = 0;
uint16_t horizontalBlankStart = 0;
uint16_t horizontalBlankStop = 0;

// ====================================================================================
// Global Variables - Picture Settings
// ====================================================================================

unsigned char R_VAL = 128;
unsigned char G_VAL = 128;
unsigned char B_VAL = 128;
uint8_t brightness = 128;
uint8_t contrast = 128;
uint8_t saturation = 128;
uint8_t brightnessOrContrastOption = 0;

// ====================================================================================
// Global Variables - Video Mode Options
// ====================================================================================

uint8_t SVModeOption = 0;
uint8_t AVModeOption = 0;
uint8_t SVModeOptionChanged = 0;
uint8_t AVModeOptionChanged = 0;
uint8_t smoothOption = 0;
uint8_t lineOption = 0;
bool settingLineOptionChanged = false;
bool settingSmoothOptionChanged = false;

// ====================================================================================
// Global Variables - Resolution Settings
// ====================================================================================

uint8_t keepSettings = 0;
uint8_t tentativeResolution = 0;

// ====================================================================================
// Global Variables - Audio
// ====================================================================================

uint8_t volume = 0;
boolean audioMuted = true;

// ====================================================================================
// ADV Communication - Packet Constants
// ====================================================================================

static const unsigned char ADV_InputRGBs[7]  = {0x41, 0x44, 'S', 0x40};
static const unsigned char ADV_InputRGsB[7]  = {0x41, 0x44, 'S', 0x50};
static const unsigned char ADV_InputVGA[7]   = {0x41, 0x44, 'S', 0x60};
static const unsigned char ADV_InputYpbpr[7] = {0x41, 0x44, 'S', 0x70};
static const unsigned char ADV_InputSV[7]    = {0x41, 0x44, 'S', 0x10};
static const unsigned char ADV_InputAV[7]    = {0x41, 0x44, 'S', 0x20};

static unsigned char ADV_TvMode[7]           = {0x41, 0x44, 'T'};
static unsigned char ADV_Line2X[7]           = {0x41, 0x44, 'S', 0x30};
static unsigned char ADV_Line1X[7]           = {0x41, 0x44, 'S', 0x31};
static unsigned char ADV_SmoothOn[7]         = {0x41, 0x44, 'S', 0x90};
static unsigned char ADV_SmoothOff[7]        = {0x41, 0x44, 'S', 0x91};
static unsigned char ADV_CompatibilityOn[7]  = {0x41, 0x44, 'S', 0xA0};
static unsigned char ADV_CompatibilityOff[7] = {0x41, 0x44, 'S', 0xA1};
static unsigned char ADV_BCSH[7]             = {0x41, 0x44, 'N'};

// ====================================================================================
// ADV Communication - Video Format Mapping Table
// ====================================================================================

const uint8_t ADV_VideoFormats[12] = {
    0x04,  // 0: Auto
    0x84,  // 1: Pal
    0x54,  // 2: Ntsc_M
    0x64,  // 3: Pal_60
    0x74,  // 4: Ntsc443
    0x44,  // 5: Ntsc_J
    0x94,  // 6: Pal_N_wp
    0xA4,  // 7: Pal_M_wop
    0xB4,  // 8: Pal_M
    0xC4,  // 9: Pal_Cmb_N
    0xD4,  // 10: Pal_Cmb_N_wp
    0xE4   // 11: Secam
};

// ====================================================================================
// ADV Communication - ADV_PacketSender Class
// ====================================================================================

/**
 * @class ADV_PacketSender
 * @brief Handles UART communication with the ADV controller (HC32F460)
 *
 * Protocol format: [0x41 0x44] [cmd] [data] [0xFE] [checksum]
 * - 0x41 0x44 ('AD'): Header bytes
 * - cmd: Command byte (e.g., 'S', 'T', 'N', 'C')
 * - data: Command-specific payload
 * - 0xFE: End-of-frame marker
 * - checksum: Sum of all preceding bytes (8-bit wrap)
 */
class ADV_PacketSender {
public:
    explicit ADV_PacketSender(HardwareSerial& serial = Serial) : m_serial(serial) {
        randomSeed(analogRead(A0));
    }

    /**
     * @brief Send a standard 4-byte command packet
     * @param buff 4-byte command buffer [header, header, cmd, data]
     *
     * Packet: [buff[0]] [buff[1]] [buff[2]] [buff[3]] [random] [0xFE] [checksum]
     */
    void send(const unsigned char* buff) {
        unsigned char buff_lin[7];
        buff_lin[0] = buff[0];
        buff_lin[1] = buff[1];
        buff_lin[2] = buff[2];
        buff_lin[3] = buff[3];
        buff_lin[4] = random(254);
        buff_lin[5] = 0xfe;
        buff_lin[6] = buff_lin[0] + buff_lin[1] + buff_lin[2] + buff_lin[3] + buff_lin[4] + buff_lin[5];
        m_serial.write(buff_lin, sizeof(buff_lin));
    }

    /**
     * @brief Send command with video mode appended to data byte
     * @param buff Base 4-byte command
     * @param mode Video mode (lower 4 bits merged into buff[3])
     */
    void sendWithMode(const unsigned char* buff, uint8_t mode) {
        unsigned char packet[4];
        memcpy(packet, buff, 4);
        packet[3] |= (mode & 0x0f);
        send(packet);
    }

    /**
     * @brief Send register write command (for BCSH adjustments)
     * @param buff Base command header
     * @param reg I2C register address
     * @param val Value to write
     */
    void writeReg(const unsigned char* buff, unsigned char reg, unsigned char val) {
        unsigned char buff_lin[7];
        for (int i = 0; i < 4; ++i) buff_lin[i] = buff[i];
        buff_lin[3] = reg;
        buff_lin[4] = val;
        buff_lin[5] = 0xFE;
        unsigned char sum = 0;
        for (int i = 0; i < 6; ++i) sum += buff_lin[i];
        buff_lin[6] = sum;
        m_serial.write(buff_lin, sizeof(buff_lin));
    }

    /**
     * @brief Send custom I2C batch command
     * @param data Array of I2C triplets [addr, reg, val, ...]
     * @param size Total bytes (must be multiple of 3)
     *
     * Packet: [0x41 0x44] ['C'] [count] [triplets...] [0xFE] [checksum]
     * Each triplet: [I2C_addr, register, value]
     */
    void sendCustomI2C(const unsigned char* data, size_t size) {
        if (size == 0 || size % 3 != 0) return;

        uint8_t count = size / 3;

        m_serial.write((uint8_t)0x41);
        m_serial.write((uint8_t)0x44);
        m_serial.write((uint8_t)'C');
        m_serial.write(count);

        unsigned char sum = 0x41 + 0x44 + 'C' + count;

        for (size_t i = 0; i < size; ++i) {
            m_serial.write(data[i]);
            sum += data[i];
        }

        m_serial.write((uint8_t)0xFE);
        sum += 0xFE;

        m_serial.write(sum);
    }

private:
    HardwareSerial& m_serial;
};

static ADV_PacketSender advPacketSender;

// ====================================================================================
// ADV Packet Wrappers
// ====================================================================================

void ADV_sendLine1X() { advPacketSender.send(ADV_Line1X); saveUserPrefs(); }
void ADV_sendLine2X() { advPacketSender.send(ADV_Line2X); saveUserPrefs(); }
void ADV_sendSmoothOff() { advPacketSender.send(ADV_SmoothOff); saveUserPrefs(); }
void ADV_sendSmoothOn() { advPacketSender.send(ADV_SmoothOn); saveUserPrefs(); }
void ADV_sendCompatibility(bool mode) {
    if (!mode)
        advPacketSender.send(ADV_CompatibilityOn);
    else
        advPacketSender.send(ADV_CompatibilityOff);
    saveUserPrefs();
}
void ADV_sendVideoFormat(uint8_t mode) {
    ADV_TvMode[3] = mode;
    advPacketSender.send(ADV_TvMode);
    saveUserPrefs();
}
void ADV_sendBCSH(unsigned char reg, unsigned char val) {
    advPacketSender.writeReg(ADV_BCSH, reg, val);
}
void ADV_sendCustomI2C(const unsigned char* data, size_t size) {
    advPacketSender.sendCustomI2C(data, size);
}

void ADV_applyPendingOptions(void)
{
    static char max_index = sizeof(ADV_VideoFormats) / sizeof(ADV_VideoFormats[0]) - 1;
    if (SVModeOptionChanged) {
        SVModeOptionChanged = 0;
        if (SVModeOption >= 0 && SVModeOption <= max_index) {
            ADV_sendVideoFormat(ADV_VideoFormats[SVModeOption]);
        } else {
            ADV_sendVideoFormat(ADV_VideoFormats[1]);
        }
    }
    if (AVModeOptionChanged) {
        AVModeOptionChanged = 0;
        if (AVModeOption >= 0 && AVModeOption <= max_index) {
            ADV_sendVideoFormat(ADV_VideoFormats[AVModeOption]);
        } else {
            ADV_sendVideoFormat(ADV_VideoFormats[1]);
        }
    }
    if (settingLineOptionChanged) {
        settingLineOptionChanged = 0;
        if (lineOption) ADV_sendLine2X(); else ADV_sendLine1X();
    }
    if (settingSmoothOptionChanged) {
        settingSmoothOptionChanged = 0;
        if (smoothOption) ADV_sendSmoothOn(); else ADV_sendSmoothOff();
    }
}

// ====================================================================================
// Input Source Switching - Basic Functions
// ====================================================================================

void InputRGBs(void) {
    advPacketSender.send(ADV_InputRGBs);
    inputSource = InputSourceRGBs;
    inputType = InputTypeRGBs;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(RGB1);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(RGB1);
    brightnessOrContrastOption = 0;
    rto->sourceDisconnected = true;
    saveUserPrefs();
}

void InputRGsB(void) {
    advPacketSender.send(ADV_InputRGsB);
    inputSource = InputSourceRGBs;
    inputType = InputTypeRGsB;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(RGB1);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(RGB1);
    brightnessOrContrastOption = 0;
    rto->sourceDisconnected = true;
    saveUserPrefs();
}

void InputVGA(void) {
    advPacketSender.sendWithMode(ADV_InputVGA, 1);
    inputSource = InputSourceVGA;
    inputType = InputTypeVGA;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(RGB1);
    GBS::SP_EXT_SYNC_SEL::write(HV_Enable);
    GBS::ADC_INPUT_SEL::write(RGB1);
    brightnessOrContrastOption = 0;
    rto->sourceDisconnected = true;
    saveUserPrefs();
}

void InputYUV(void) {
    advPacketSender.send(ADV_InputYpbpr);
    inputSource = InputSourceYUV;
    inputType = InputTypeYUV;
    resetSyncProcessor();
    brightnessOrContrastOption = 1;
    rto->sourceDisconnected = true;
    saveUserPrefs();
}

void InputSV(void) {
    advPacketSender.send(ADV_InputSV);
    inputSource = InputSourceYUV;
    inputType = InputTypeSV;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(YUV0);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(YUV0);
    brightnessOrContrastOption = 2;
    rto->sourceDisconnected = true;
    rto->isInLowPowerMode = false;
    saveUserPrefs();
}

void InputAV(void) {
    advPacketSender.send(ADV_InputAV);
    inputSource = InputSourceYUV;
    inputType = InputTypeAV;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(YUV0);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(YUV0);
    brightnessOrContrastOption = 2;
    rto->sourceDisconnected = true;
    rto->isInLowPowerMode = false;
    saveUserPrefs();
}

// ====================================================================================
// Input Source Switching - Functions with Mode Parameter
// ====================================================================================

void InputRGBs_mode(uint8_t mode) {
    advPacketSender.sendWithMode(ADV_InputRGBs, !mode);
    inputSource = InputSourceRGBs;
    inputType = InputTypeRGBs;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(RGB1);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(RGB1);
    brightnessOrContrastOption = 0;
    rto->sourceDisconnected = true;
    saveUserPrefs();
}

void InputRGsB_mode(uint8_t mode) {
    advPacketSender.sendWithMode(ADV_InputRGsB, !mode);
    inputSource = InputSourceRGBs;
    inputType = InputTypeRGsB;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(RGB1);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(RGB1);
    brightnessOrContrastOption = 0;
    rto->sourceDisconnected = true;
    saveUserPrefs();
}

void InputVGA_mode(uint8_t mode) {
    advPacketSender.sendWithMode(ADV_InputVGA, !mode);
    inputSource = InputSourceVGA;
    inputType = InputTypeVGA;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(RGB1);
    GBS::SP_EXT_SYNC_SEL::write(HV_Enable);
    GBS::ADC_INPUT_SEL::write(RGB1);
    brightnessOrContrastOption = 0;
    rto->sourceDisconnected = true;
    saveUserPrefs();
}

void InputSV_mode(uint8_t mode) {
    advPacketSender.sendWithMode(ADV_InputSV, mode);
    inputSource = InputSourceYUV;
    inputType = InputTypeSV;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(YUV0);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(YUV0);
    brightnessOrContrastOption = 2;
    rto->sourceDisconnected = true;
    rto->isInLowPowerMode = false;
    saveUserPrefs();
}

void InputAV_mode(uint8_t mode) {
    advPacketSender.sendWithMode(ADV_InputAV, mode);
    inputSource = InputSourceYUV;
    inputType = InputTypeAV;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(YUV0);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(YUV0);
    brightnessOrContrastOption = 2;
    rto->sourceDisconnected = true;
    rto->isInLowPowerMode = false;
    saveUserPrefs();
}

// ====================================================================================
// Input Source Switching - Boot/Restore Function
// ====================================================================================

void applySavedInputSource(void) {
    // Apply saved input source configuration to hardware at startup
    switch (inputSource) {
        case InputSourceRGBs:
            // inputType already loaded from preferences (InputTypeRGBs or InputTypeRGsB)
            resetSyncProcessor();
            GBS::ADC_SOGEN::write(RGB1);
            GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
            GBS::ADC_INPUT_SEL::write(RGB1);
            brightnessOrContrastOption = 0;
            rto->sourceDisconnected = true;
            break;

        case InputSourceVGA:
            inputType = InputTypeVGA;
            resetSyncProcessor();
            GBS::ADC_SOGEN::write(RGB1);
            GBS::SP_EXT_SYNC_SEL::write(HV_Enable);
            GBS::ADC_INPUT_SEL::write(RGB1);
            brightnessOrContrastOption = 0;
            rto->sourceDisconnected = true;
            break;

        case InputSourceYUV:
            // inputType already loaded from preferences (InputTypeYUV, InputTypeSV, or InputTypeAV)
            if (inputType == InputTypeSV) {
                advPacketSender.send(ADV_InputSV);  // ADV needed for SV
                resetSyncProcessor();
                GBS::ADC_SOGEN::write(YUV0);
                GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
                GBS::ADC_INPUT_SEL::write(YUV0);
                brightnessOrContrastOption = 2;
            } else if (inputType == InputTypeAV) {
                advPacketSender.send(ADV_InputAV);  // ADV needed for AV
                resetSyncProcessor();
                GBS::ADC_SOGEN::write(YUV0);
                GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
                GBS::ADC_INPUT_SEL::write(YUV0);
                brightnessOrContrastOption = 2;
            } else {
                // InputTypeYUV (YPbPr)
                resetSyncProcessor();
                brightnessOrContrastOption = 1;
            }
            rto->sourceDisconnected = true;
            break;

        default:
            // Default to RGBs if inputSource is invalid
            inputSource = InputSourceRGBs;
            inputType = InputTypeRGBs;
            resetSyncProcessor();
            GBS::ADC_SOGEN::write(RGB1);
            GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
            GBS::ADC_INPUT_SEL::write(RGB1);
            brightnessOrContrastOption = 0;
            rto->sourceDisconnected = true;
            break;
    }
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
    GBS::VDS_Y_OFST::write((signed char)((0.299f * (R_VAL - 128)) + (0.587f * (G_VAL - 128)) + (0.114f * (B_VAL - 128))));
    GBS::VDS_U_OFST::write((signed char)((-0.14713f * (R_VAL - 128)) - (0.28886f * (G_VAL - 128)) + (0.436f * (B_VAL - 128))));
    GBS::VDS_V_OFST::write((signed char)((0.615f * (R_VAL - 128)) - (0.51499f * (G_VAL - 128)) - (0.10001f * (B_VAL - 128))));
}

void readYUVtoRGBConversion(void)
{
    R_VAL = GBS::VDS_Y_OFST::read() + (1.13983f * GBS::VDS_V_OFST::read()) + 128;
    G_VAL = GBS::VDS_Y_OFST::read() - (0.39465f * GBS::VDS_U_OFST::read()) - (0.58060f * GBS::VDS_V_OFST::read()) + 128;
    B_VAL = GBS::VDS_Y_OFST::read() + (2.03211f * GBS::VDS_U_OFST::read()) + 128;
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
    constexpr size_t MESSAGE_LEN = 6;
    char buffer[MESSAGE_LEN];
    buffer[0] = '$';

    uint8_t currentInputType = 1;
    if (inputSource == InputSourceRGBs) {
        currentInputType = (inputType == InputTypeRGsB) ? 2 : 1;
    }
    else if (inputSource == InputSourceVGA) {
        currentInputType = 3;
    }
    else if (inputSource == InputSourceYUV) {
        if (inputType == InputTypeYUV) currentInputType = 4;
        else if (inputType == InputTypeSV) currentInputType = 5;
        else if (inputType == InputTypeAV) currentInputType = 6;
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

    buffer[3] = '0' + (lineOption ? 1 : 0);
    buffer[4] = '0' + (smoothOption ? 1 : 0);
    buffer[5] = isPeakingLocked() ? '1' : '0';

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
        // Refresh OSD TV display
        if (lastOsdCommand != 0) {
            OSD_handleCommand(lastOsdCommand);
        }
    }
}
