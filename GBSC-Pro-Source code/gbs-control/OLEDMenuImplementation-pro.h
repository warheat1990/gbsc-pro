// ====================================================================================
// OLEDMenuImplementation-pro.h
// GBSC-Pro Extensions for OLEDMenuImplementation
//
// This file contains Pro-specific declarations for:
// - HC32 controller communication
// - ADV7280/ADV7391 processor control
// - Input source switching
// - Pro-specific OLED menu items
// ====================================================================================

#ifndef OLED_MENU_IMPLEMENTATION_PRO_H_
#define OLED_MENU_IMPLEMENTATION_PRO_H_

#include "OLEDMenuManager.h"

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
// Constants - Input Source Selection
// ====================================================================================

// Input source types
#define S_RGBs 1
#define S_VGA  2
#define S_YUV  3

// Input info identifiers (for display/status)
#define InfoRGBs 1
#define InfoRGsB 2
#define InfoVGA  3
#define InfoYUV  4
#define InfoSV   5
#define InfoAV   6

// ====================================================================================
// External Variables
// ====================================================================================

// TV mode mapping table (defined in .cpp)
extern const uint8_t modes[12];

// ====================================================================================
// Menu Initialization Functions
// ====================================================================================

void initOLEDMenuProInput(OLEDMenuItem *root);
void initOLEDMenuProSettings(OLEDMenuItem *root);

// ====================================================================================
// Menu Handler Functions
// ====================================================================================

bool InputSwHandler(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav nav, bool isFirstTime);
bool SettingHandler(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav nav, bool isFirstTime);
bool Adv7391TvModeSwHandler(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav nav, bool isFirstTime);

// ====================================================================================
// HC32 Communication Functions
// ====================================================================================

void SetReg(unsigned char reg, unsigned char val);

// ====================================================================================
// ADV Processor Control Functions
// ====================================================================================

void Send_TvMode(uint8_t Mode);
void Send_Line(bool line);
void Send_Smooth(bool Smooth);
void Send_Compatibility(bool Com);

// ====================================================================================
// Input Source Switching Functions
// ====================================================================================

// Basic input switching (no mode parameter)
void InputRGBs(void);
void InputRGsB(void);
void InputVGA(void);
void InputYUV(void);
void InputSV(void);
void InputAV(void);

// Input switching with mode parameter
void InputRGBs_mode(uint8_t mode);
void InputRGsB_mode(uint8_t mode);
void InputVGA_mode(uint8_t mode);
void InputSV_mode(uint8_t mode);
void InputAV_mode(uint8_t mode);

// Restore saved input configuration at boot
void applySavedInputSource(void);

#endif // OLED_MENU_IMPLEMENTATION_PRO_H_
