#ifndef _SLOT_H_
// SLOTS
#define SLOTS_FILE "/slots.bin" // the file where to store slots metadata
#define SLOTS_TOTAL 36          // max number of slots (A-Z, 0-9)
#define EMPTY_SLOT_NAME "Empty                   "

// --- slots.bin file format header ---
// Bumped each time SlotMeta layout changes. Firmware auto-wipes slots.bin
// (and all preset_*.X files) when the on-disk version doesn't match.
#define SLOTS_HEADER_MAGIC      "GBSPS"   // 5 ASCII chars
#define SLOTS_HEADER_MAGIC_LEN  6         // includes the trailing null
#define SLOTS_FORMAT_VERSION    0x01      // bump when SlotMeta layout changes
#define SLOTS_HEADER_SIZE       16

typedef struct
{
    char magic[6];          // "GBSPS\0"
    uint8_t version;        // SLOTS_FORMAT_VERSION
    uint8_t reserved[9];
} __attribute__((packed)) SlotsFileHeader;

static_assert(sizeof(SlotsFileHeader) == SLOTS_HEADER_SIZE, "SlotsFileHeader must be 16 bytes");

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
    // --- PRO: ADV7280 Comb Control Parameters ---
    uint8_t advCombLumaModeNTSC;   // 0,4-7 (default 0=Adaptive)
    uint8_t advCombChromaModeNTSC; // 0,4-7 (default 0=Adaptive)
    uint8_t advCombChromaTapsNTSC; // 0-3 (default 2=5→3 lines)
    uint8_t advCombLumaModePAL;    // 0,4-7 (default 0=Adaptive)
    uint8_t advCombChromaModePAL;  // 0,4-7 (default 0=Adaptive)
    uint8_t advCombChromaTapsPAL;  // 0-3 (default 3=5→4 lines)
    // --- HDMI Limited Range ---
    uint8_t hdmiLimitedRange;    // 0=Off, 1=HD, 2=SD, 3=All (default 1)
    // --- PRO: ADV7280 Hue ---
    uint8_t advHue;              // 0-255 (default 128 = 0°)
    // --- PRO: Developer menu tweaks for NTSC group (videoStandardInput in {1,3,5,6,7,8,9}) ---
    // (0 / 0xFF = no override, use preset default)
    uint16_t devHTotal_ntsc;     // VDS_HSYNC_RST custom (0 = no override)
    uint16_t devPllDiv_ntsc;     // PLLAD_MD custom (0 = no override)
    uint8_t  devSdramClock_ntsc; // PLL_MS (0xFF = no override)
    uint8_t  devAdcFilter_ntsc;  // ADC_FLTR (0xFF = no override)
    uint8_t  devOsr_ntsc;        // OSR (0xFF = no override)
    uint8_t  devSogLevel_ntsc;   // ADC_SOGCTRL custom (0xFF = no override)
    uint8_t  devSyncInvert_ntsc; // bit0=HS, bit1=VS, bit7=set (0 = no override)
    // --- PRO: Screen Move / Scale for NTSC group ---
    uint16_t screenHMove_ntsc;   // IF_HBIN_SP (0 = no override)
    uint16_t screenVMoveSt_ntsc; // IF_VB_ST  (0xFFFF = no override)
    uint16_t screenVMoveSp_ntsc; // IF_VB_SP  (0xFFFF = no override)
    uint16_t screenHScale_ntsc;  // VDS_HSCALE (0 = no override)
    uint16_t screenVScale_ntsc;  // VDS_VSCALE (0 = no override)
    // --- PRO: Developer menu tweaks for PAL group (videoStandardInput in {2,4}) ---
    uint16_t devHTotal_pal;
    uint16_t devPllDiv_pal;
    uint8_t  devSdramClock_pal;
    uint8_t  devAdcFilter_pal;
    uint8_t  devOsr_pal;
    uint8_t  devSogLevel_pal;
    uint8_t  devSyncInvert_pal;
    // --- PRO: Screen Move / Scale for PAL group ---
    uint16_t screenHMove_pal;
    uint16_t screenVMoveSt_pal;
    uint16_t screenVMoveSp_pal;
    uint16_t screenHScale_pal;
    uint16_t screenVScale_pal;
    // --- PRO: Per-slot SyncWatcher override ---
    uint8_t  slotSyncwatcherMode;  // 0=inherit global, 1=force ON, 2=force OFF

    uint8_t activeInputType;    // 1=RGBs, 2=RGsB, 3=VGA, 4=YPbPr, 5=SV, 6=AV (0=unset)
    // --- Reserved for future expansion (do not use directly) ---
    uint8_t reserved[21];        // Padding to make SlotMeta 128 bytes total
} __attribute__((packed)) SlotMeta;

// Ensure SlotMeta is exactly 128 bytes (webapp and firmware must match)
static_assert(sizeof(SlotMeta) == 128, "SlotMeta must be exactly 128 bytes");

typedef struct
{
    SlotMeta slot[SLOTS_TOTAL]; // the max avaliable slots that can be encoded in a the charset[A-Za-z0-9-._~()!*:,;]
} SlotMetaArray;
#endif