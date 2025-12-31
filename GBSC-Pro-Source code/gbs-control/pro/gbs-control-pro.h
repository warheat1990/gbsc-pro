// ====================================================================================
// gbs-control-pro.h
// GBSC-Pro Main Header
//
// This file contains Pro-specific declarations:
// - IR remote control (IRremoteESP8266)
// - TV OSD menu system (STV9426 chip)
// - Audio volume control (PT2257 chip)
// - OLED display integration
// - Extended color/video adjustments
//
// Code organization:
// - gbs-control-pro.cpp: Core functions, global variables
// - menu/menu-core.cpp: Menu navigation, IR dispatch, OLED handlers
// - osd/osd-core.cpp: OSD dispatch table, command handler
// - menu/*.cpp: IR menu handlers per section
// - osd/*.cpp: OSD rendering handlers per section
//
// Types and enums are in:
// - osd/osd-registry.h: OsdCommand, MenuEntry, handler declarations
// - menu/menu-registry.h: OLED_MenuState, MenuItemMapping, IR declarations
// ====================================================================================

#pragma once

#include <Arduino.h>
#include "options-pro.h"
#include "drivers/adv_controller.h"

// ====================================================================================
// Firmware Version Strings
// ====================================================================================

#define GBS_FW_VERSION "2.0.0beta"  // GBS Control firmware version (ESP8266)
#define ADV_FW_VERSION "2.0.0beta"  // ADV Controller firmware version (HC32)

// ====================================================================================
// Registry Headers (types and enums)
// ====================================================================================

#include "osd/osd-registry.h"
#include "menu/menu-registry.h"

// ====================================================================================
// Forward Declarations
// ====================================================================================

class SSD1306Wire;
class WebSocketsServer;
struct runTimeOptions;
struct userOptions;
template <uint8_t> class TV5725;

// ====================================================================================
// Macros
// ====================================================================================

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

// ====================================================================================
// External Variables - IR Remote
// ====================================================================================

#include <IRremoteESP8266.h>
#include <IRrecv.h>

extern IRrecv irrecv;
extern decode_results results;

// ====================================================================================
// Input Source Helper
// ====================================================================================

// Input source routing values (for GBS ADC configuration)
enum InputSource : uint8_t {
    InputSourceRGBs = 0,
    InputSourceVGA = 1,
    InputSourceYUV = 2
};

// Input type values (logical input selection)
enum InputType : uint8_t {
    InputTypeRGBs = 1,
    InputTypeRGsB = 2,
    InputTypeVGA = 3,
    InputTypeYUV = 4,
    InputTypeSV = 5,
    InputTypeAV = 6
};

// Derive InputSource from activeInputType (replaces inputSource variable)
inline uint8_t getInputSourceFromType(uint8_t inputType) {
    switch (inputType) {
        case InputTypeRGBs:
        case InputTypeRGsB:
            return InputSourceRGBs;
        case InputTypeVGA:
            return InputSourceVGA;
        case InputTypeYUV:
        case InputTypeSV:
        case InputTypeAV:
        default:
            return InputSourceYUV;
    }
}

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
extern boolean irEnabled;
extern uint8_t menuLineColors[OSD_MAX_MENU_ROWS];  // Index 0-2 = rows 1-3
extern uint8_t isInfoDisplayActive;
extern uint16_t horizontalBlankStart;
extern uint16_t horizontalBlankStop;

// ====================================================================================
// External Variables - Picture Settings (per-slot, stored in SlotMeta)
// ====================================================================================

extern uint8_t gbsColorR;        // GBS TV5725 color balance R (0-255, default 128)
extern uint8_t gbsColorG;        // GBS TV5725 color balance G (0-255, default 128)
extern uint8_t gbsColorB;        // GBS TV5725 color balance B (0-255, default 128)
extern uint8_t advBrightness;    // ADV7280 brightness (0-255, default 128)
extern uint8_t advContrast;      // ADV7280 contrast (0-255, default 128)
extern uint8_t advSaturation;    // ADV7280 saturation (0-255, default 128)

// ====================================================================================
// External Variables - Video Mode Options
// ====================================================================================

extern uint8_t svVideoFormatChanged;   // Flag: S-Video format changed, needs ADV update
extern uint8_t avVideoFormatChanged;   // Flag: Composite format changed, needs ADV update
extern uint8_t advSmooth;              // ADV7280 smooth interpolation (per-slot)
extern uint8_t advI2P;                 // ADV7280 I2P - interlace to progressive (per-slot)

// ====================================================================================
// External Variables - Resolution Settings
// ====================================================================================

extern uint8_t keepSettings;
extern uint8_t tentativeResolution;

// ====================================================================================
// External Variables - Factory Reset
// ====================================================================================

extern uint8_t factoryResetSelection;  // 0 = No (default), 1 = Yes

// ====================================================================================
// Function Declarations - ADV Communication
// ====================================================================================

// ADV packet wrappers
void ADV_sendI2P(bool enable);
void ADV_sendSmooth(bool enable);
void ADV_sendCompatibility(bool mode);
void ADV_sendVideoFormat(uint8_t format);
void ADV_sendBCSH(unsigned char reg, unsigned char val);
void ADV_sendCustomI2C(const unsigned char* data, size_t size);
void ADV_applyPendingOptions(void);
void ADV_applySlotSettings(void);

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

