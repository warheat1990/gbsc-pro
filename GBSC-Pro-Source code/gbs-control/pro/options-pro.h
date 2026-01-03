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
    MT_I2P_OFF,
    MT_I2P_ON,
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

// ====================================================================================
// Pro userOptions Fields
// These fields are included in struct userOptions via options.h
// ====================================================================================

#define USER_OPTIONS_PRO_FIELDS \
    INPUT_PresetPreference INPUT_presetPreference; \
    SETTING_PresetPreference SETTING_presetPreference; \
    TVMODE_PresetPreference TVMODE_presetPreference; \
    uint8_t volume;                 /* PT2257 volume (0-50, 50=max, 0=mute) */ \
    uint8_t audioMuted;             /* PT2257 mute state (0=unmuted, 1=muted) */ \
    uint8_t activeInputType;        /* Input type (1-6: RGBs/RGsB/VGA/YUV/SV/AV) */ \
    uint8_t svVideoFormat;          /* ADV7280 S-Video format (0=Auto, 1-11) */ \
    uint8_t avVideoFormat;          /* ADV7280 Composite format (0=Auto, 1-11) */ \
    uint8_t bcshAdjustMode;         /* BCSH adjustment UI mode */ \
    uint8_t advCompatibility;       /* ADV7280 RGB compatibility mode */ \
    uint8_t osdTheme;               /* TV OSD theme (0-3) */ \
    /* GBS TV5725 Color Balance */ \
    uint8_t gbsColorR;              /* R channel (0-255, default 128) */ \
    uint8_t gbsColorG;              /* G channel (0-255, default 128) */ \
    uint8_t gbsColorB;              /* B channel (0-255, default 128) */ \
    /* ADV7280 Processing Settings */ \
    uint8_t advI2P;                 /* I2P - interlace to progressive (0-1, default 1) */ \
    uint8_t advSmooth;              /* Smooth interpolation (0-1, default 0) */ \
    uint8_t advACE;                 /* ACE - Adaptive Contrast Enhancement (0-1, default 0) */ \
    /* ADV7280 BCSH (Brightness/Contrast/Saturation) */ \
    uint8_t advBrightness;          /* Brightness (0-254, default 128) */ \
    uint8_t advContrast;            /* Contrast (0-254, default 128) */ \
    uint8_t advSaturation;          /* Saturation (0-254, default 128) */ \
    /* ADV7280 ACE Parameters */ \
    uint8_t advACELumaGain;         /* Luma Gain (0-31, default 13) */ \
    uint8_t advACEChromaGain;       /* Chroma Gain (0-15, default 8) */ \
    uint8_t advACEChromaMax;        /* Chroma Max (0-15, default 8) */ \
    uint8_t advACEGammaGain;        /* Gamma Gain (0-15, default 8) */ \
    uint8_t advACEResponseSpeed;    /* Response Speed (0-15, default 15) */

#endif // OPTIONS_PRO_H_
