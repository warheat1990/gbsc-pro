// ====================================================================================
// adv_controller.h
// ADV Controller UART Communication Driver (Header-Only)
//
// Generic driver for UART communication with HC32F460 ADV controller.
// This module is project-independent and contains only:
// - Protocol constants
// - Packet definitions for ADV7280/ADV7391 control
// - ADVController class for packet transmission
//
// Protocol format: [0x41 0x44] [cmd] [data] [random] [0xFE] [checksum]
// - 0x41 0x44 ('AD'): Header bytes identifying ADV protocol
// - cmd: Command byte ('S' = Source, 'T' = TvMode, 'N' = BCSH, 'C' = Custom)
// - data: Command-specific payload
// - random: Random byte (for anti-replay / packet uniqueness)
// - 0xFE: End-of-frame marker
// - checksum: 8-bit sum of all preceding bytes
// ====================================================================================

#ifndef ADV_CONTROLLER_H_
#define ADV_CONTROLLER_H_

#include <Arduino.h>

// ====================================================================================
// Protocol Constants
// ====================================================================================

#define ADV_HEADER_0        0x41  // 'A'
#define ADV_HEADER_1        0x44  // 'D'
#define ADV_END_MARKER      0xFE
#define ADV_PACKET_SIZE     7

// Delay after packet transmission for HC32 DMA processing
// HC32 uses USART_TIMEOUT_BITS = 250 (~6.7ms timeout) to detect frame end
// 35ms provides margin for timeout + command processing + I2C operations
#define ADV_PACKET_DELAY_MS 35

// Command bytes
#define ADV_CMD_SOURCE      'S'   // Input source / line mode / smooth / compatibility
#define ADV_CMD_TVMODE      'T'   // TV mode (video format)
#define ADV_CMD_BCSH        'N'   // Brightness/Contrast/Saturation/Hue register write
#define ADV_CMD_CUSTOM      'C'   // Custom I2C batch command

// ====================================================================================
// Packet Constants - Input Sources
// Data byte format for 'S' command: 0xXY where X=source type, Y=mode bits
// ====================================================================================

static const unsigned char ADV_InputRGBs[4]  = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0x40};
static const unsigned char ADV_InputRGsB[4]  = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0x50};
static const unsigned char ADV_InputVGA[4]   = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0x60};
static const unsigned char ADV_InputYpbpr[4] = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0x70};
static const unsigned char ADV_InputSV[4]    = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0x10};
static const unsigned char ADV_InputAV[4]    = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0x20};

// ====================================================================================
// Packet Constants - Video Options
// ====================================================================================

static const unsigned char ADV_TvMode[4]            = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_TVMODE, 0x00};
static const unsigned char ADV_I2P_On[4]            = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0x30};
static const unsigned char ADV_I2P_Off[4]           = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0x31};
static const unsigned char ADV_Smooth_On[4]         = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0x90};
static const unsigned char ADV_Smooth_Off[4]        = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0x91};
static const unsigned char ADV_Compatibility_On[4]  = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0xA0};
static const unsigned char ADV_Compatibility_Off[4] = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0xA1};
static const unsigned char ADV_ACE_On[4]            = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0x80};
static const unsigned char ADV_ACE_Off[4]           = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0x81};
static const unsigned char ADV_BCSH[4]              = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_BCSH, 0x00};

// ====================================================================================
// ACE (Adaptive Contrast Enhancement) Parameter Commands
// Uses 'S' command with sub-commands 0x82-0x87
// Data byte in packet[4] contains the parameter value
// ====================================================================================

#define ADV_ACE_LUMA_GAIN       0x82  // Set Luma Gain (0-31), value in packet[4]
#define ADV_ACE_CHROMA_GAIN     0x83  // Set Chroma Gain (0-15), value in packet[4]
#define ADV_ACE_CHROMA_MAX      0x84  // Set Chroma Max (0-15), value in packet[4]
#define ADV_ACE_GAMMA_GAIN      0x85  // Set Gamma Gain (0-15), value in packet[4]
#define ADV_ACE_RESPONSE_SPEED  0x86  // Set Response Speed (0-15), value in packet[4]
#define ADV_ACE_DEFAULTS        0x87  // Reset ACE parameters to defaults

// ACE parameter default values (from ADV7280 User Sub Map 2)
#define ADV_ACE_LUMA_GAIN_DEFAULT       13  // 0x0D
#define ADV_ACE_CHROMA_GAIN_DEFAULT     8
#define ADV_ACE_CHROMA_MAX_DEFAULT      8
#define ADV_ACE_GAMMA_GAIN_DEFAULT      8
#define ADV_ACE_RESPONSE_SPEED_DEFAULT  15  // 0x0F

// Packet templates for ACE parameters (use writeReg to fill value)
static const unsigned char ADV_ACE_Param[4] = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0x00};
static const unsigned char ADV_ACE_Defaults[4] = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, ADV_ACE_DEFAULTS};

// ====================================================================================
// Video Filter Parameter Commands
// Uses 'S' command with sub-commands 0xB0-0xB7
// Data byte in packet[4] contains the parameter value
// ====================================================================================

#define ADV_FILTER_Y_SHAPING     0xB0  // Set Y Shaping Filter for CVBS (0-31), value in packet[4]
#define ADV_FILTER_C_SHAPING     0xB1  // Set C Shaping Filter for CVBS (0-7), value in packet[4]
#define ADV_FILTER_WY_SHAPING    0xB2  // Set WY Shaping Filter for S-Video (0-31), value in packet[4]
#define ADV_FILTER_WY_OVERRIDE   0xB3  // Set WY Override (0=Auto, 1=Manual), value in packet[4]
#define ADV_FILTER_COMB_NTSC     0xB4  // Set Comb Filter NTSC bandwidth (0-3), value in packet[4]
#define ADV_FILTER_COMB_PAL      0xB5  // Set Comb Filter PAL bandwidth (0-3), value in packet[4]
#define ADV_FILTER_DEFAULTS      0xB7  // Reset Video Filter parameters to defaults

// Video Filter parameter default values (from ADV7280 Main Register Map)
#define ADV_FILTER_Y_SHAPING_DEFAULT     1   // Auto Narrow
#define ADV_FILTER_C_SHAPING_DEFAULT     0   // Auto 1.5MHz
#define ADV_FILTER_WY_SHAPING_DEFAULT    19  // SVHS 18 (CCIR 601)
#define ADV_FILTER_WY_OVERRIDE_DEFAULT   1   // Manual
#define ADV_FILTER_COMB_NTSC_DEFAULT     0   // Narrow
#define ADV_FILTER_COMB_PAL_DEFAULT      1   // Medium

// Packet templates for Video Filter parameters (use writeReg to fill value)
static const unsigned char ADV_Filter_Param[4] = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, 0x00};
static const unsigned char ADV_Filter_Defaults[4] = {ADV_HEADER_0, ADV_HEADER_1, ADV_CMD_SOURCE, ADV_FILTER_DEFAULTS};

// ====================================================================================
// Video Format Mapping Table
// ====================================================================================

// Index to ADV7280 video format register value mapping
// Bits [7:4] = format select, Bits [3:0] = 0x4 (enable auto detection within format)
//
// 0 = Auto-detect all
// 1 = PAL B/G/H/I/D
// 2 = NTSC-M
// 3 = PAL-60
// 4 = NTSC-4.43
// 5 = NTSC-J
// 6 = PAL-N (with pedestal)
// 7 = PAL-M (without pedestal)
// 8 = PAL-M (with pedestal)
// 9 = PAL-Combination N
// 10 = PAL-Combination N (with pedestal)
// 11 = SECAM
static const uint8_t ADV_VideoFormats[12] = {
    0x04,  // 0: Auto
    0x84,  // 1: PAL
    0x54,  // 2: NTSC-M
    0x64,  // 3: PAL-60
    0x74,  // 4: NTSC-4.43
    0x44,  // 5: NTSC-J
    0x94,  // 6: PAL-N (wp)
    0xA4,  // 7: PAL-M (wop)
    0xB4,  // 8: PAL-M
    0xC4,  // 9: PAL-Cn
    0xD4,  // 10: PAL-Cn (wp)
    0xE4   // 11: SECAM
};

#define ADV_VIDEO_FORMAT_COUNT  (sizeof(ADV_VideoFormats) / sizeof(ADV_VideoFormats[0]))

// ====================================================================================
// ADVController Class
// ====================================================================================

/**
 * @class ADVController
 * @brief Handles UART packet transmission to the ADV controller
 *
 * Provides methods for various packet types:
 * - send(): Standard 4-byte command packets
 * - sendWithMode(): Commands with mode bits merged into data byte
 * - writeReg(): Register write commands (for BCSH)
 * - sendCustomI2C(): Custom I2C batch commands
 *
 * All methods handle random byte injection, end marker, and checksum automatically.
 */
class ADVController {
public:
    /**
     * @brief Construct packet sender with serial port
     * @param serial Reference to HardwareSerial (default: Serial)
     */
    explicit ADVController(HardwareSerial& serial = Serial) : m_serial(serial) {
        randomSeed(analogRead(A0));
    }

    /**
     * @brief Send a standard 4-byte command packet
     * @param buff 4-byte base packet [header0, header1, cmd, data]
     * @param delayMs Post-transmit delay in ms (default: ADV_PACKET_DELAY_MS)
     *
     * Output: [buff[0..3]] [random] [0xFE] [checksum]
     */
    void send(const unsigned char* buff, uint16_t delayMs = ADV_PACKET_DELAY_MS) {
        unsigned char packet[ADV_PACKET_SIZE];
        packet[0] = buff[0];
        packet[1] = buff[1];
        packet[2] = buff[2];
        packet[3] = buff[3];
        packet[4] = random(254);
        packet[5] = ADV_END_MARKER;
        packet[6] = packet[0] + packet[1] + packet[2] + packet[3] + packet[4] + packet[5];
        m_serial.write(packet, ADV_PACKET_SIZE);
        delay(delayMs);
    }

    /**
     * @brief Send command with mode bits merged into data byte
     * @param buff 4-byte base packet
     * @param mode Mode value (lower 4 bits OR'd into buff[3])
     * @param delayMs Post-transmit delay in ms (default: ADV_PACKET_DELAY_MS)
     */
    void sendWithMode(const unsigned char* buff, uint8_t mode, uint16_t delayMs = ADV_PACKET_DELAY_MS) {
        unsigned char packet[4];
        memcpy(packet, buff, 4);
        packet[3] |= (mode & 0x0F);
        send(packet, delayMs);
    }

    /**
     * @brief Send register write command (for BCSH adjustments)
     * @param buff 4-byte base packet (buff[2] = command, buff[3] ignored)
     * @param reg Target register address
     * @param val Value to write
     * @param delayMs Post-transmit delay in ms (default: ADV_PACKET_DELAY_MS)
     *
     * Output: [header0, header1, cmd, reg, val, 0xFE, checksum]
     */
    void writeReg(const unsigned char* buff, unsigned char reg, unsigned char val, uint16_t delayMs = ADV_PACKET_DELAY_MS) {
        unsigned char packet[ADV_PACKET_SIZE];
        packet[0] = buff[0];
        packet[1] = buff[1];
        packet[2] = buff[2];
        packet[3] = reg;
        packet[4] = val;
        packet[5] = ADV_END_MARKER;
        unsigned char sum = 0;
        for (int i = 0; i < 6; ++i) sum += packet[i];
        packet[6] = sum;
        m_serial.write(packet, ADV_PACKET_SIZE);
        delay(delayMs);
    }

    /**
     * @brief Send custom I2C batch command
     * @param data Array of I2C triplets [addr, reg, val, addr, reg, val, ...]
     * @param size Total bytes (must be multiple of 3)
     * @param delayMs Post-transmit delay in ms (default: ADV_PACKET_DELAY_MS)
     *
     * Output: [header0, header1, 'C', count, triplets..., 0xFE, checksum]
     * Each triplet: [I2C_addr, register, value]
     */
    void sendCustomI2C(const unsigned char* data, size_t size, uint16_t delayMs = ADV_PACKET_DELAY_MS) {
        if (size == 0 || size % 3 != 0) return;

        uint8_t count = size / 3;

        m_serial.write((uint8_t)ADV_HEADER_0);
        m_serial.write((uint8_t)ADV_HEADER_1);
        m_serial.write((uint8_t)ADV_CMD_CUSTOM);
        m_serial.write(count);

        unsigned char sum = ADV_HEADER_0 + ADV_HEADER_1 + ADV_CMD_CUSTOM + count;

        for (size_t i = 0; i < size; ++i) {
            m_serial.write(data[i]);
            sum += data[i];
        }

        m_serial.write((uint8_t)ADV_END_MARKER);
        sum += ADV_END_MARKER;

        m_serial.write(sum);
        delay(delayMs);
    }

private:
    HardwareSerial& m_serial;
};

#endif // ADV_CONTROLLER_H_
