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
    MT_SYNCSTRIPPER_OFF,
    MT_SYNCSTRIPPER_ON,
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
    uint8_t advSyncStripper;        /* LM1881 sync stripper (0=off/bypass, 1=on) */ \
    uint8_t osdTheme;               /* TV OSD theme (0-3) */ \
    /* GBS TV5725 Color Balance */ \
    uint8_t gbsColorR;              /* R channel (0-255, default 128) */ \
    uint8_t gbsColorG;              /* G channel (0-255, default 128) */ \
    uint8_t gbsColorB;              /* B channel (0-255, default 128) */ \
    /* ADV7280 Processing Settings */ \
    uint8_t advI2P;                 /* I2P - interlace to progressive (0-1, default 1) */ \
    uint8_t advSmooth;              /* Smooth interpolation (0-1, default 0) */ \
    uint8_t advACE;                 /* ACE - Adaptive Contrast Enhancement (0-1, default 0) */ \
    /* ADV7280 BCSH (Brightness/Contrast/Saturation/Hue) */ \
    uint8_t advBrightness;          /* Brightness (0-254, default 128) */ \
    uint8_t advContrast;            /* Contrast (0-254, default 128) */ \
    uint8_t advSaturation;          /* Saturation (0-254, default 128) */ \
    uint8_t advHue;                 /* Hue (0-254, default 128 = 0°) */ \
    /* ADV7280 ACE Parameters */ \
    uint8_t advACELumaGain;         /* Luma Gain (0-31, default 13) */ \
    uint8_t advACEChromaGain;       /* Chroma Gain (0-15, default 8) */ \
    uint8_t advACEChromaMax;        /* Chroma Max (0-15, default 8) */ \
    uint8_t advACEGammaGain;        /* Gamma Gain (0-15, default 8) */ \
    uint8_t advACEResponseSpeed;    /* Response Speed (0-15, default 15) */ \
    /* ADV7280 Video Filter Parameters */ \
    uint8_t advFilterYShaping;      /* Y Shaping Filter for CVBS (0-30, default 1=Auto Narrow) */ \
    uint8_t advFilterCShaping;      /* C Shaping Filter for CVBS (0-7, default 0=Auto 1.5MHz) */ \
    uint8_t advFilterWYShaping;     /* WY Shaping Filter for S-Video (2-19, default 19=SVHS 18) */ \
    uint8_t advFilterWYOverride;    /* WY Override (0=Auto, 1=Manual, default 1) */ \
    uint8_t advFilterCombNTSC;      /* Comb Filter NTSC bandwidth (0-3, default 0=Narrow) */ \
    uint8_t advFilterCombPAL;       /* Comb Filter PAL bandwidth (0-3, default 1=Medium) */ \
    uint8_t advCombLumaModeNTSC;    /* NTSC Luma Mode (0=Adaptive, 4=Notch, 5-7=Fixed) */ \
    uint8_t advCombChromaModeNTSC;  /* NTSC Chroma Mode (0=Adaptive, 4=Off, 5-7=Fixed) */ \
    uint8_t advCombChromaTapsNTSC;  /* NTSC Chroma Taps (0-3, default 2=5→3 lines) */ \
    uint8_t advCombLumaModePAL;     /* PAL Luma Mode (0=Adaptive, 4=Notch, 5-7=Fixed) */ \
    uint8_t advCombChromaModePAL;   /* PAL Chroma Mode (0=Adaptive, 4=Off, 5-7=Fixed) */ \
    uint8_t advCombChromaTapsPAL;   /* PAL Chroma Taps (0-3, default 3=5→4 lines) */ \
    /* HDMI Limited Range */ \
    uint8_t hdmiLimitedRange;       /* Force Limited Range output (0=Off, 1=HD, 2=SD, 3=All) */ \
    /* No-signal Black Screen Output */ \
    uint8_t keepOutputOnNoSignal;   /* Keep HDMI output active (black screen) when no input (0=Off, 1=On) */ \
    uint8_t lastVideoStandard;      /* Last successfully detected video standard (0=none, 1-9) */ \
    /* Developer menu persistent tweaks (0 / 0xFF = no override, use preset default) */ \
    uint16_t devHTotal;             /* VDS_HSYNC_RST custom (0 = no override) */ \
    uint16_t devPllDiv;             /* PLLAD_MD custom (0 = no override) */ \
    uint8_t  devSdramClock;         /* PLL_MS (0xFF = no override, range 0-7) */ \
    uint8_t  devAdcFilter;          /* ADC_FLTR (0xFF = no override, range 0-3) */ \
    uint8_t  devOsr;                /* OSR (0xFF = no override, valid: 1/2/4) */ \
    uint8_t  devSogLevel;           /* ADC_SOGCTRL (0xFF = no override, range 0-16) */ \
    uint8_t  devSyncInvert;         /* bit0=HS, bit1=VS, bit7=set (0 = no override) */ \
    /* Screen Move / Scale (0 / 0xFFFF = no override) */ \
    uint16_t screenHMove;           /* IF_HBIN_SP (0 = no override) */ \
    uint16_t screenVMoveSt;         /* IF_VB_ST (0xFFFF = no override) */ \
    uint16_t screenVMoveSp;         /* IF_VB_SP (0xFFFF = no override) */ \
    uint16_t screenHScale;          /* VDS_HSCALE (0 = no override) */ \
    uint16_t screenVScale;          /* VDS_VSCALE (0 = no override) */ \
    /* Per-slot SyncWatcher override */ \
    uint8_t  slotSyncwatcherMode;   /* 0=inherit global, 1=force ON, 2=force OFF */

#endif // OPTIONS_PRO_H_
