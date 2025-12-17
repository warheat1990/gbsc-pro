// ====================================================================================
// options-pro.h
// GBSC-Pro Extensions for options.h
//
// This file contains Pro-specific enum definitions that extend the original
// gbs-control options. These enums are used in the Pro-specific fields added
// to userOptions struct for persistent settings storage.
// ====================================================================================

#ifndef OPTIONS_PRO_H_
#define OPTIONS_PRO_H_

#include <stdint.h>

// ====================================================================================
// Input Source Selection
// ====================================================================================

enum INPUT_PresetPreference : uint8_t {
    MT_RGBs,
    MT_RGsB,
    MT_VGA,
    MT_YPBPR,
    MT_SV,
    MT_AV,
};

// ====================================================================================
// Video Processing Settings
// ====================================================================================

enum SETTING_PresetPreference : uint8_t {
    MT_7391_1X,
    MT_7391_2X,
    MT_SMOOTH_OFF,
    MT_SMOOTH_ON,
    MT_COMPATIBILITY_OFF,
    MT_COMPATIBILITY_ON,
    MT_ACE_OFF,
    MT_ACE_ON,
};

// ====================================================================================
// TV Mode / Video Standard Selection (ADV7280 Decoder)
// ====================================================================================

enum TVMODE_PresetPreference : uint8_t {
    MT_MODE_AUTO = 0,
    MT_MODE_PAL,
    MT_MODE_NTSCM,
    MT_MODE_PAL60,
    MT_MODE_NTSC443,
    MT_MODE_NTSCJ,
    MT_MODE_PALNwp,
    MT_MODE_PALMwop,
    MT_MODE_PALM,
    MT_MODE_PALCmbN,
    MT_MODE_PALCmbNwp,
    MT_MODE_SECAM,
};

#endif // OPTIONS_PRO_H_
