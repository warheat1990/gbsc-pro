#ifndef _SLOT_H_
// SLOTS
#define SLOTS_FILE "/slots.bin" // the file where to store slots metadata
#define SLOTS_TOTAL 36          // max number of slots (A-Z, 0-9)
#define EMPTY_SLOT_NAME "Empty                   "
typedef struct
{
    // --- ORIGINAL GBS-CONTROL ---
    char name[25];
    uint8_t presetID;
    uint8_t scanlines;
    uint8_t scanlinesStrength;
    uint8_t slot;
    uint8_t wantVdsLineFilter;
    uint8_t wantStepResponse;
    uint8_t wantPeaking;
    // --- PRO: GBS Processing options ---
    uint8_t wantFullHeight;      // 0=off, 1=on (default 1)
    uint8_t deintMode;           // 0=Adaptive, 1=Bob (default 0)
    uint8_t enableFrameTimeLock; // 0=off, 1=on (default 0)
    uint8_t frameTimeLockMethod; // 0 or 1 (default 0)
    uint8_t PalForce60;          // 0=off, 1=on (default 0)
    uint8_t adcGain;             // ADC_RGCTRL value (default 0x7B)
    uint8_t wantSharpness;       // 0=off, 1=on (default 0)
    // --- PRO: GBS Color balance ---
    uint8_t gbsColorR;           // 0-255 (default 128)
    uint8_t gbsColorG;           // 0-255 (default 128)
    uint8_t gbsColorB;           // 0-255 (default 128)
    // --- PRO: ADV7280 settings (directly expandable) ---
    uint8_t advSmooth;           // 0=off, 1=on
    uint8_t advI2P;              // 0=off, 1=on (interlace to progressive)
    uint8_t advBrightness;       // 0-255 (default 128)
    uint8_t advContrast;         // 0-255 (default 128)
    uint8_t advSaturation;       // 0-255 (default 128)
    uint8_t advACE;              // 0=off, 1=on (Adaptive Contrast Enhancement)
    // --- PRO: ADV7280 ACE Parameters ---
    uint8_t advACELumaGain;      // 0-31 (default 13)
    uint8_t advACEChromaGain;    // 0-15 (default 8)
    uint8_t advACEChromaMax;     // 0-15 (default 8)
    uint8_t advACEGammaGain;     // 0-15 (default 8)
    uint8_t advACEResponseSpeed; // 0-15 (default 15)
    // --- PRO: ADV7280 Video Filter Parameters ---
    uint8_t advFilterYShaping;   // 0-30 (default 1=Auto Narrow)
    uint8_t advFilterCShaping;   // 0-7 (default 0=Auto 1.5MHz)
    uint8_t advFilterWYShaping;  // 2-19 (default 19=SVHS 18)
    uint8_t advFilterWYOverride; // 0=Auto, 1=Manual (default 1=Manual)
    uint8_t advFilterCombNTSC;   // 0-3 (default 0=Narrow)
    uint8_t advFilterCombPAL;    // 0-3 (default 1=Medium)
    // --- HDMI Limited Range ---
    uint8_t hdmiLimitedRange;    // 0=Off, 1=HD, 2=SD, 3=All (default 1)
    // --- Reserved for future expansion (do not use directly) ---
    uint8_t reserved[68];        // Padding to make SlotMeta 128 bytes total
} SlotMeta;

// Ensure SlotMeta is exactly 128 bytes (webapp and firmware must match)
static_assert(sizeof(SlotMeta) == 128, "SlotMeta must be exactly 128 bytes");

typedef struct
{
    SlotMeta slot[SLOTS_TOTAL]; // the max avaliable slots that can be encoded in a the charset[A-Za-z0-9-._~()!*:,;]
} SlotMetaArray;
#endif