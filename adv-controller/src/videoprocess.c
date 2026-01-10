/**
 *******************************************************************************
 * @file  videoprocess.c
 * @brief ADV7280/ADV7391 video processing control
 *        - ADV7280: Video decoder (CVBS/S-Video input)
 *        - ADV7391: Video encoder (Component/RGB output)
 *******************************************************************************
 */

#include "videoprocess.h"
#include "ev_hc32f460_lqfp100_v2_bsp.h"
#include "main.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
uint8_t btn_flag;

static uint8_t status = 0;
uint8_t Adv_7391_sw = 0;
uint8_t adv_input;
uint8_t adv_i2p = true;
uint8_t adv_smooth = false;
uint8_t adv_ace = false;
uint8_t adv_sw = false;
uint8_t adv_tv = 0xff;
uint8_t err_flag = 0;
uint8_t c_state = 0;

uint8_t Bright = 0x00;
uint8_t Contrast = 0x80;
uint8_t Saturation = 0x80;

/* ACE (Adaptive Contrast Enhancement) parameters - User Sub Map 2 */
uint8_t AceLumaGain = 13;      /* 0x83 bits 4:0, default 0x0D */
uint8_t AceChromaGain = 8;     /* 0x84 bits 3:0, default 0x08 */
uint8_t AceChromaMax = 8;      /* 0x84 bits 7:4, default 0x08 */
uint8_t AceGammaGain = 8;      /* 0x85 bits 3:0, default 0x08 */
uint8_t AceResponseSpeed = 15; /* 0x85 bits 7:4, default 0x0F */

/* Video Filter parameters - Main Register Map (0x17, 0x18, 0x19) */
uint8_t FilterYShaping = 1;     /* 0x17 bits 4:0, YSFM for CVBS, default 1 (Auto Narrow) */
uint8_t FilterCShaping = 0;     /* 0x17 bits 7:5, CSFM for CVBS, default 0 (Auto 1.5MHz) */
uint8_t FilterWYShaping = 19;   /* 0x18 bits 4:0, WYSFM for S-Video/YPrPb, default 19 (SVHS 18 CCIR 601) */
uint8_t FilterWYShapingOvr = 1; /* 0x18 bit 7, WYSFMOVR default 1 (Manual) */
uint8_t FilterCombNTSC = 0;     /* 0x19 bits 3:2, NSFSEL, default 0 (Narrow) */
uint8_t FilterCombPAL = 1;      /* 0x19 bits 1:0, PSFSEL, default 1 (Medium) */

bool asw_01, asw_02, asw_03, asw_04;
bool AVsw;
uint8_t Input_signal = 0;
uint8_t buff_send[APP_FRAME_LEN_MAX];

/*******************************************************************************
 * I2C Command Arrays - ADV7280/ADV7391 Register Configuration
 * Format: [I2C_ADDR, REG, VALUE] triplets
 ******************************************************************************/

/* I2P (Interlace-to-Progressive) Enable */
uint8_t I2C_COMMANDS_I2P_ON[] = {
    0x84, 0x55, 0x80,   // ADV7280 - I2C_DEINT_ENABLE: Enable I2P Converter
    0x84, 0x5A, 0x02,   // ADV7280 - Not documented on page 99 - Configure I2P Parameters Smooth 1A (?)
};

/* I2P Disable for progressive input */
const uint8_t I2C_COMMANDS_I2P_OFF_p[] = {
    0x84, 0x55, 0x00,   // ADV7280 - I2C_DEINT_ENABLE: Disable I2P Converter
    0x84, 0x5A, 0x02,   // ADV7280 - Not documented on page 99 - Configure I2P Parameters Smooth 1A (?)
};

/* Smooth filter OFF */
uint8_t I2C_COMMANDS_SMOOTH_OFF[] = {
    0x84, 0x55, 0x80,   // ADV7280 - I2C_DEINT_ENABLE: Enable I2P Converter
    0x84, 0x5A, 0x02,   // ADV7280 - Disable I2P Smooth 1A
    0x42, 0x0E, 0x00    // ADV7280 - ADI Control 1: main register
};

/* Smooth filter ON */
uint8_t I2C_COMMANDS_SMOOTH_ON[] = {
    0x84, 0x55, 0x80,   // ADV7280 - I2C_DEINT_ENABLE: Enable I2P Converter
    0x84, 0x5A, 0x1A,   // ADV7280 - Enable I2P Smooth 1A
    0x42, 0x0E, 0x00    // ADV7280 - ADI Control 1: main register
};

/* Brightness/Contrast/Saturation/Hue control */
uint8_t I2C_COMMANDS_BCSH[] = {
    0x42, 0x0E, 0x00,   // ADV7280 - ADI Control 1: main register
    0x42, 0x0A, 0x00,   // ADV7280 - Brightness adjust: 0 IRE
    0x42, 0x08, 0x00,   // ADV7280 - Contrast: 0 gain [0x80 (default) for 1 gain]
    0x42, 0xE3, 0x00,   // ADV7280 - SD saturation Cb channel: -42dB [0x80 (default) for 0dB]
    0x42, 0x0B, 0x00,   // ADV7280 - Hue adjust: 0 default
};

/* S-Video (Y/C) input configuration */
uint8_t I2C_COMMANDS_YC_INPUT[] = {
    /* =============== ADV7280 S-Video =============== */
    0x42, 0x0E, 0x00,   // ADV7280 - ADI Control 1: main register
    0x42, 0x00, 0x09,   // ADV7280 - Input control: Y input on A3, C input on A4
    0x42, 0x38, 0x80,   // ADV7280 - NTSC comb control: default
    0x42, 0x39, 0xC0,   // ADV7280 - PAL comb control: default
    0x42, 0x17, 0x49,   // ADV7280 - Shaping Filter Control 1: SH1, SVHS 8
};

/* CVBS (Composite) input configuration */
uint8_t I2C_COMMANDS_CVBS_INPUT[] = {
    /* =============== ADV7280 CVBS =============== */
    0x42, 0x0E, 0x00,   // ADV7280 - ADI Control 1: main register
    0x42, 0x00, 0x00,   // ADV7280 - Input control: CVBS input on A1
    0x42, 0x38, 0x80,   // ADV7280 - NTSC comb control: default
    0x42, 0x39, 0xC0,   // ADV7280 - PAL comb control: default
    0x42, 0x17, 0x47,   // ADV7280 - Shaping Filter Control 1: SH1, SVHS 6
};

/* Full initialization sequence for ADV7280 + ADV7391 */
uint8_t I2C_AUTO_COMMANDS[] = {
    /* Initial reset */
    0x42, 0x0F, 0x80,   // ADV7280 - Power management: Start reset sequence
    0x56, 0x17, 0x02,   // ADV7391 - Software reset: Software reset
    0xFF, 0x0A, 0x00,   // Unknown - Delay 10ms
    0x42, 0x0F, 0x00,   // ADV7280 - Power management: Normal operation

    /* Configure input */
    0x42, 0x05, 0x00,   // ADV7280 - Not documented on page 71
    0x42, 0x02, 0x04,   // ADV7280 - Video Selection 2: Autodetect, set to default
    0x42, 0x07, 0xFF,   // ADV7280 - Autodetect enable: All enabled
    0x42, 0x14, 0x15,   // ADV7280 - Analog clamp control: Current sources enabled, Sets to default, Boundary box
    0x42, 0x00, 0x00,   // ADV7280 - Input control: CVBS input on A1

    /* ADI Required write */
    0x42, 0x0E, 0x80,   // ADV7280 - ADI Required Write; Reset Current Clamp Circuitry (step1)
    0x42, 0x9C, 0x00,   // ADV7280 - ADI Required Write; Reset Current Clamp Circuitry (step2)
    0x42, 0x9C, 0xFF,   // ADV7280 - ADI Required Write; Reset Current Clamp Circuitry (step3)
    0x42, 0x0E, 0x00,   // ADV7280 - ADI Required Write; Reset Current Clamp Circuitry (step4)

    /* Power Up Digital Output Pads */
    0x42, 0x03, 0x0C,   // ADV7280 - Enable Pixel & Sync output drivers
    0x42, 0x04, 0x07,   // ADV7280 - Power-up INTRQ, HS & VS pads
    0x42, 0x13, 0x00,   // ADV7280 - Enable ADV7182 for 28_63636MHz crystal

    /* Configuration */
    0x42, 0x17, 0x40,   // ADV7280 - Shaping Filter Control 1: SH1, Autowide notch for poor quality sources or wideband filter with comb for good quality input
    0x42, 0x1D, 0x40,   // ADV7280 - Enable LLC output driver

    /* ADI Required Write for Fast Switch */
    0x42, 0x52, 0xCD,   // ADV7280 - Single Ended CVBS - Set optimized IBIAS for the AFE
    0x42, 0x80, 0x51,   // ADV7280 - ADI Required Write
    0x42, 0x81, 0x51,   // ADV7280 - ADI Required Write
    0x42, 0x82, 0x00,   // 0x68 ? // ADV7280 - ADI Required Write
    0x42, 0x0E, 0x80,   // ADV7280 - ADI Required Write
    0x42, 0xD9, 0x44,   // ADV7280 - ADI Required Write
    0x42, 0x0E, 0x00,   // ADV7280 - ADI Control 1: main register

    /* Enable I2P */
    0x42, 0xFD, 0x84,   // ADV7280 - set the VPP address to 0x84
    0x84, 0xA3, 0x00,   // ADV7280 - ADI Required Write - VPP writes begin
    0x84, 0x5B, 0x00,   // ADV7280 - ADV_TIMING_MODE_EN: Enable advanced timing mode
    0x84, 0x55, 0x80,   // ADV7280 - I2C_DEINT_ENABLE - Enable I2P Converter
    0x84, 0x5A, 0x02,   // ADV7280 - Disable I2P Smooth 1A
    0x42, 0x6B, 0x11,   // ADV7280 - Output Sync Select 2: VSYNC

    /* Reset ADV7391 */
    0x56, 0x17, 0x02,   // ADV7391 - Software reset: Software reset
    0xFF, 0x0A, 0x00,   // Unknown - Delay 10ms

    /* Encoder configuration */
    0x56, 0x00, 0x9C,   // 0x1C ? // ADV7391 - Power up DACs and PLL [Encoder writes begin]
    0x56, 0x01, 0x70,   // ADV7391 - Mode select: ED (at 54MHz) input, Chrome rising, luma falling
    0x56, 0x30, 0x1C,   // 0x04 NTSC // ADV7391 - ED/HD Mode Register 1: SMPTE 296M-4, SMPTE 274M-5 720p at 30 Hz/29.97 Hz, EIA-770.2 output EIA-770.3 output
    0x56, 0x31, 0x01,   // ADV7391 - ED/HD Mode Register 2: Pixel data valid on
    0x42, 0x0E, 0x00,   // ADV7280 - ADI Control 1: main register
};

/* ACE (Adaptive Contrast Enhancement) ON */
uint8_t Ace_Code_ON[] = {
    0x42, 0x0E, 0x40,   // ADV7280 - ADI Control 1: User Sub Map 2
    0x42, 0x80, 0x80,   // ADV7280 - ACE Control 1: Enable ACE
    0x42, 0x0E, 0x00    // ADV7280 - ADI Control 1: main register
};

/* ACE (Adaptive Contrast Enhancement) OFF */
uint8_t Ace_Code_OFF[] = {
    0x42, 0x0E, 0x40,   // ADV7280 - ADI Control 1: User Sub Map 2
    0x42, 0x80, 0x00,   // ADV7280 - ACE Control 1: Disable ACE
    0x42, 0x0E, 0x00    // ADV7280 - ADI Control 1: main register
};

/* 525p (NTSC) Encoder Configuration */
uint8_t I2C_COMMANDS_525p_CONFIG[] = {
    0x56, 0x30, 0x04,
    0x56, 0x31, 0x11,
};

/* 625p (PAL) Encoder Configuration */
uint8_t I2C_COMMANDS_625p_CONFIG[] = {
    0x56, 0x30, 0x1C,
    0x56, 0x31, 0x11,
};

/*******************************************************************************
 * Video Format Detection and Configuration
 ******************************************************************************/

/**
 * @brief Configure video encoder based on detected input format
 *        Reads AD_RESULT register to determine 525p vs 625p
 */
static void ADV_ConfigureEncoder(void)
{
    uint8_t ad_result;
    uint8_t buff[1] = { 0x10 };

    I2C_Receive(DEVICE_ADDR, &buff[0], &ad_result, 1, TIMEOUT);

    /* Extract bits [6:5:4] for AD_RESULT */
    uint8_t ad_standard = (ad_result & 0x70) >> 4;
    uint8_t is_525p = 0;

    /*
     * AD_RESULT values:
     * 0: NTSC M/NTSC J (525p)
     * 1: NTSC 4.43 (525p)
     * 2: PAL M (525p)
     * 3: PAL 60 (525p)
     * 4: PAL B/PAL G/PAL H/PAL I/PAL D (625p)
     * 5: SECAM (625p)
     * 6: PAL Combination N (625p)
     * 7: SECAM 525 (525p)
     */
    if (ad_standard == 0 || ad_standard == 1 || ad_standard == 2 ||
        ad_standard == 3 || ad_standard == 7) {
        is_525p = 1;
    } else {
        is_525p = 0;
    }

    if (is_525p) {
        (void)I2C_TransmitBatch(I2C_COMMANDS_525p_CONFIG,
                                  sizeof(I2C_COMMANDS_525p_CONFIG) / 3, TIMEOUT);
        printf("[ADV] 525p format configured\n");
    }

    if (!is_525p) {
        (void)I2C_TransmitBatch(I2C_COMMANDS_625p_CONFIG,
                                  sizeof(I2C_COMMANDS_625p_CONFIG) / 3, TIMEOUT);
        printf("[ADV] 625p format configured\n");
    }
}

/**
 * @brief Detect and print current video format
 * @param btn_flag Enable flag for detection printout
 */
static void ADV_DetectFormat(uint8_t btn_flag)
{
    if (!btn_flag)
        return;

    uint8_t reg13 = 0, reg12 = 0, reg10 = 0;
    uint8_t buff[2];

    buff[0] = 0x13;
    I2C_Receive(DEVICE_ADDR, &buff[0], &reg13, 1, TIMEOUT);

    buff[0] = 0x12;
    I2C_Receive(DEVICE_ADDR, &buff[0], &reg12, 1, TIMEOUT);

    buff[0] = 0x10;
    I2C_Receive(DEVICE_ADDR, &buff[0], &reg10, 1, TIMEOUT);

    uint8_t interlaced = (reg13 & 0x40) >> 6;
    uint8_t in_lock    = (reg13 & 0x01);
    uint8_t ll_nstd    = (reg12 & 0x10) >> 4;
    uint8_t ad_result  =  reg10 & 0x70;

    (void)ll_nstd; /* Unused but kept for reference */

    if (!in_lock) {
        printf("[ADV] No signal lock\n");
        return;
    }

    if (!interlaced) {
        if (ad_result == 0x40) {
            printf("[ADV] 288p (PAL progressive)\n");
        } else if (ad_result == 0x00 || ad_result == 0x20) {
            printf("[ADV] 240p (NTSC progressive)\n");
        } else {
            printf("[ADV] Unknown format\n");
        }
    } else {
        if (ad_result == 0x40) {
            printf("[ADV] 576i (PAL interlaced)\n");
        } else if (ad_result == 0x00 || ad_result == 0x20) {
            printf("[ADV] 480i (NTSC interlaced)\n");
        } else {
            printf("[ADV] Unknown format\n");
        }
    }
}

/*******************************************************************************
 * Video Module Init/Deinit
 ******************************************************************************/

/**
 * @brief Initialize ADV7280/ADV7391 video processing module
 */
void ADV_Init(void)
{
    uint8_t count = 0;
    uint8_t buff[2] = {0xe0, 0x00};

    (void)count; /* Unused */

    /* Power sequence */
    GPIO_SetPins(POWER_DOWN_PORT, GPIO_PIN_POWER_DOWN);
    DDL_DelayMS(5);
    GPIO_SetPins(INPUT_RESET_PORT, GPIO_PIN_INPUT_RESET);
    DDL_DelayMS(10);
    GPIO_SetPins(OUTPUT_PORT, GPIO_PIN_OUTPUT_EN);
    DDL_DelayMS(10);

    (void)I2C_Receive(DEVICE_ADDR, buff, NULL, 1, TIMEOUT);
    DDL_DelayMS(15);

    /* Send full initialization sequence */
    (void)I2C_TransmitBatch(I2C_AUTO_COMMANDS, sizeof(I2C_AUTO_COMMANDS) / 3, TIMEOUT);
    ADV_SetInput(adv_input);

    /* Set video mode */
    buff[0] = VID_SEL_REG;
    buff[1] = adv_tv;
    printf("[ADV] Init adv_tv: 0x%02x\n", adv_tv);
    (void)I2C_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT);

    /* Configure encoder and BCSH */
    ADV_ConfigureEncoder();
    ADV_SetBCSH();
    ADV_SetACE(adv_ace);
    ADV_SetACEParams();     /* Apply ACE parameters loaded from flash */
    ADV_SetFilterParams();  /* Apply Video Filter parameters loaded from flash */

    /* Configure I2P based on settings */
    if (adv_i2p) {
        ADV_SetI2P(adv_i2p);
        if (adv_smooth)
            ADV_SetSmooth(adv_smooth);
    } else {
        ADV_SetI2P(true);
        ADV_SetSmooth(true);
        adv_smooth = false;
        ADV_SetSmooth(adv_smooth);
        ADV_SetI2P(adv_i2p);
    }

    Adv_7391_sw = 1;
    printf("[ADV] ModuleOn\n");
}

/**
 * @brief Deinitialize video processing module
 */
void ADV_Deinit(void)
{
    GPIO_ResetPins(POWER_DOWN_PORT, GPIO_PIN_POWER_DOWN);
    GPIO_ResetPins(INPUT_RESET_PORT, GPIO_PIN_INPUT_RESET);
    GPIO_ResetPins(OUTPUT_PORT, GPIO_PIN_OUTPUT_EN);
    Adv_7391_sw = 0;
    printf("[ADV] ModuleOff\n");
}

/**
 * @brief Enable/disable video module
 * @param sw true to enable, false to disable
 */
void ADV_Enable(uint8_t sw)
{
    if (sw == true) {
        ADV_Init();
    }
}

/*******************************************************************************
 * Video Settings Control
 ******************************************************************************/

/**
 * @brief Set input source (CVBS or S-Video)
 * @param input 0=CVBS, 1=S-Video
 */
void ADV_SetInput(uint8_t input)
{
    if (input) {
        (void)I2C_TransmitBatch(I2C_COMMANDS_YC_INPUT,
                                  sizeof(I2C_COMMANDS_YC_INPUT) / 3, TIMEOUT);
        printf("[ADV] SvSignal\n");
    } else {
        (void)I2C_TransmitBatch(I2C_COMMANDS_CVBS_INPUT,
                                  sizeof(I2C_COMMANDS_CVBS_INPUT) / 3, TIMEOUT);
        printf("[ADV] AvSignal\n");
    }
}

/**
 * @brief Enable/disable I2P (Interlace to Progressive) conversion
 * @param enable true to enable, false to disable
 */
void ADV_SetI2P(uint8_t enable)
{
    if (enable) {
        (void)I2C_TransmitBatch(I2C_COMMANDS_I2P_ON,
                                  sizeof(I2C_COMMANDS_I2P_ON) / 3, TIMEOUT);
        printf("[ADV] I2pOn\n");
    } else {
        (void)I2C_TransmitBatch(I2C_COMMANDS_I2P_OFF_p,
                                  sizeof(I2C_COMMANDS_I2P_OFF_p) / 3, TIMEOUT);
        printf("[ADV] I2pOff\n");
    }
}

/**
 * @brief Enable/disable smooth filter
 * @param smooth true to enable, false to disable
 */
void ADV_SetSmooth(uint8_t smooth)
{
    if (smooth) {
        (void)I2C_TransmitBatch(I2C_COMMANDS_SMOOTH_ON,
                                  sizeof(I2C_COMMANDS_SMOOTH_ON) / 3, TIMEOUT);
        printf("[ADV] SmoothOn\n");
    } else {
        (void)I2C_TransmitBatch(I2C_COMMANDS_SMOOTH_OFF,
                                  sizeof(I2C_COMMANDS_SMOOTH_OFF) / 3, TIMEOUT);
        printf("[ADV] SmoothOff\n");
    }
}

/**
 * @brief Apply brightness/contrast/saturation settings
 */
void ADV_SetBCSH(void)
{
    I2C_COMMANDS_BCSH[5] = Bright;
    I2C_COMMANDS_BCSH[8] = Contrast;
    I2C_COMMANDS_BCSH[11] = Saturation;
    (void)I2C_TransmitBatch(I2C_COMMANDS_BCSH, sizeof(I2C_COMMANDS_BCSH) / 3, TIMEOUT);
    printf("[ADV] Bright: 0x%02x Contrast: 0x%02x Sat: 0x%02x\n", Bright, Contrast, Saturation);
}

/**
 * @brief Enable/disable ACE (Adaptive Contrast Enhancement)
 * @param ace true to enable, false to disable
 */
void ADV_SetACE(uint8_t ace)
{
    if (ace) {
        (void)I2C_TransmitBatch(Ace_Code_ON, sizeof(Ace_Code_ON) / 3, TIMEOUT);
        printf("[ADV] AceOn\n");
    } else {
        (void)I2C_TransmitBatch(Ace_Code_OFF, sizeof(Ace_Code_OFF) / 3, TIMEOUT);
        printf("[ADV] AceOff\n");
    }
}

/*******************************************************************************
 * ACE (Adaptive Contrast Enhancement) Parameter Controls
 * User Sub Map 2 (0x0E = 0x40): Registers 0x80-0x85
 ******************************************************************************/

/**
 * @brief Set ACE Luma Gain (auto-contrast level)
 * @param gain 0-31 (5 bits), default 13 (0x0D)
 * Register 0x83, bits [4:0]
 */
void ADV_SetACELumaGain(uint8_t gain)
{
    if (gain > 31) gain = 31;
    AceLumaGain = gain;

    /* Batch: [addr, reg, val] x 3 commands */
    const uint8_t batch[] = {
        DEVICE_ADDR, 0x0E, 0x40,              /* Switch to User Sub Map 2 */
        DEVICE_ADDR, 0x83, gain & 0x1F,       /* ACE Control 4: Luma Gain [4:0] */
        DEVICE_ADDR, 0x0E, 0x00               /* Return to main register map */
    };
    (void)I2C_TransmitBatch(batch, 3, TIMEOUT);

    printf("[ADV] ACE LumaGain: %d\n", gain);
}

/**
 * @brief Set ACE Chroma Gain (auto-saturation level)
 * @param gain 0-15 (4 bits), default 8
 * Register 0x84, bits [3:0]
 */
void ADV_SetACEChromaGain(uint8_t gain)
{
    if (gain > 15) gain = 15;
    AceChromaGain = gain;

    /* Batch: [addr, reg, val] x 3 commands */
    const uint8_t batch[] = {
        DEVICE_ADDR, 0x0E, 0x40,                              /* Switch to User Sub Map 2 */
        DEVICE_ADDR, 0x84, (AceChromaMax << 4) | (gain & 0x0F), /* ACE Control 5: ChromaMax[7:4] | ChromaGain[3:0] */
        DEVICE_ADDR, 0x0E, 0x00                                /* Return to main register map */
    };
    (void)I2C_TransmitBatch(batch, 3, TIMEOUT);

    printf("[ADV] ACE ChromaGain: %d\n", gain);
}

/**
 * @brief Set ACE Chroma Max (maximum saturation threshold)
 * @param max 0-15 (4 bits), default 8
 * Register 0x84, bits [7:4]
 */
void ADV_SetACEChromaMax(uint8_t max)
{
    if (max > 15) max = 15;
    AceChromaMax = max;

    /* Batch: [addr, reg, val] x 3 commands */
    const uint8_t batch[] = {
        DEVICE_ADDR, 0x0E, 0x40,                              /* Switch to User Sub Map 2 */
        DEVICE_ADDR, 0x84, (max << 4) | (AceChromaGain & 0x0F), /* ACE Control 5: ChromaMax[7:4] | ChromaGain[3:0] */
        DEVICE_ADDR, 0x0E, 0x00                                /* Return to main register map */
    };
    (void)I2C_TransmitBatch(batch, 3, TIMEOUT);

    printf("[ADV] ACE ChromaMax: %d\n", max);
}

/**
 * @brief Set ACE Gamma Gain (further contrast enhancement)
 * @param gain 0-15 (4 bits), default 8
 * Register 0x85, bits [3:0]
 */
void ADV_SetACEGammaGain(uint8_t gain)
{
    if (gain > 15) gain = 15;
    AceGammaGain = gain;

    /* Batch: [addr, reg, val] x 3 commands */
    const uint8_t batch[] = {
        DEVICE_ADDR, 0x0E, 0x40,                                   /* Switch to User Sub Map 2 */
        DEVICE_ADDR, 0x85, (AceResponseSpeed << 4) | (gain & 0x0F), /* ACE Control 6: RespSpeed[7:4] | GammaGain[3:0] */
        DEVICE_ADDR, 0x0E, 0x00                                    /* Return to main register map */
    };
    (void)I2C_TransmitBatch(batch, 3, TIMEOUT);

    printf("[ADV] ACE GammaGain: %d\n", gain);
}

/**
 * @brief Set ACE Response Speed
 * @param speed 0-15 (4 bits), default 15 (fastest)
 * Register 0x85, bits [7:4]
 */
void ADV_SetACEResponseSpeed(uint8_t speed)
{
    if (speed > 15) speed = 15;
    AceResponseSpeed = speed;

    /* Batch: [addr, reg, val] x 3 commands */
    const uint8_t batch[] = {
        DEVICE_ADDR, 0x0E, 0x40,                                   /* Switch to User Sub Map 2 */
        DEVICE_ADDR, 0x85, (speed << 4) | (AceGammaGain & 0x0F),    /* ACE Control 6: RespSpeed[7:4] | GammaGain[3:0] */
        DEVICE_ADDR, 0x0E, 0x00                                    /* Return to main register map */
    };
    (void)I2C_TransmitBatch(batch, 3, TIMEOUT);

    printf("[ADV] ACE ResponseSpeed: %d\n", speed);
}

/**
 * @brief Apply all ACE parameters to hardware
 * Call this after loading settings from flash or changing multiple params
 */
void ADV_SetACEParams(void)
{
    /* Batch: [addr, reg, val] x 5 commands - write all ACE regs in one batch */
    const uint8_t batch[] = {
        DEVICE_ADDR, 0x0E, 0x40,                                       /* Switch to User Sub Map 2 */
        DEVICE_ADDR, 0x83, AceLumaGain & 0x1F,                         /* ACE Control 4: Luma Gain [4:0] */
        DEVICE_ADDR, 0x84, (AceChromaMax << 4) | (AceChromaGain & 0x0F), /* ACE Control 5: ChromaMax[7:4] | ChromaGain[3:0] */
        DEVICE_ADDR, 0x85, (AceResponseSpeed << 4) | (AceGammaGain & 0x0F), /* ACE Control 6: RespSpeed[7:4] | GammaGain[3:0] */
        DEVICE_ADDR, 0x0E, 0x00                                        /* Return to main register map */
    };
    (void)I2C_TransmitBatch(batch, 5, TIMEOUT);

    printf("[ADV] ACE Params: Luma=%d Chroma=%d/%d Gamma=%d Speed=%d\n",
           AceLumaGain, AceChromaGain, AceChromaMax, AceGammaGain, AceResponseSpeed);
}

/**
 * @brief Reset all ACE parameters to defaults
 */
void ADV_SetACEDefaults(void)
{
    AceLumaGain = 13;      /* 0x0D */
    AceChromaGain = 8;
    AceChromaMax = 8;
    AceGammaGain = 8;
    AceResponseSpeed = 15; /* 0x0F */

    ADV_SetACEParams();
    printf("[ADV] ACE Defaults restored\n");
}

/*******************************************************************************
 * Video Filter Parameter Controls
 * Main Register Map: Registers 0x17 (Shaping Filter 1), 0x18 (Shaping Filter 2),
 * 0x19 (Comb Filter Control)
 ******************************************************************************/

/**
 * @brief Set Y Shaping Filter for CVBS input
 * @param filter 0-31 (YSFM), default 0 (Auto Wide)
 * Register 0x17, bits [4:0]
 */
void ADV_SetFilterYShaping(uint8_t filter)
{
    if (filter > 31) filter = 31;
    FilterYShaping = filter;

    /* Register 0x17 = (CSFM << 5) | YSFM */
    uint8_t reg_val = ((FilterCShaping & 0x07) << 5) | (filter & 0x1F);

    const uint8_t batch[] = {
        DEVICE_ADDR, 0x17, reg_val
    };
    (void)I2C_TransmitBatch(batch, 1, TIMEOUT);

    printf("[ADV] Filter Y Shaping: %d\n", filter);
}

/**
 * @brief Set C Shaping Filter for CVBS input
 * @param filter 0-7 (CSFM), default 0 (Auto 1.5MHz)
 * Register 0x17, bits [7:5]
 */
void ADV_SetFilterCShaping(uint8_t filter)
{
    if (filter > 7) filter = 7;
    FilterCShaping = filter;

    /* Register 0x17 = (CSFM << 5) | YSFM */
    uint8_t reg_val = ((filter & 0x07) << 5) | (FilterYShaping & 0x1F);

    const uint8_t batch[] = {
        DEVICE_ADDR, 0x17, reg_val
    };
    (void)I2C_TransmitBatch(batch, 1, TIMEOUT);

    printf("[ADV] Filter C Shaping: %d\n", filter);
}

/**
 * @brief Set Wideband Y Shaping Filter for S-Video/YPrPb input
 * @param filter 0-31 (WYSFM), default 0 (Auto)
 * Register 0x18, bits [4:0]
 */
void ADV_SetFilterWYShaping(uint8_t filter)
{
    if (filter > 31) filter = 31;
    FilterWYShaping = filter;

    /* Register 0x18 = (WYSFMOVR << 7) | WYSFM - bits 6 and 5 are reserved */
    uint8_t reg_val = ((FilterWYShapingOvr & 0x01) << 7) | (filter & 0x1F);

    const uint8_t batch[] = {
        DEVICE_ADDR, 0x18, reg_val
    };
    (void)I2C_TransmitBatch(batch, 1, TIMEOUT);

    printf("[ADV] Filter WY Shaping: %d\n", filter);
}

/**
 * @brief Set Wideband Y Shaping Filter Override mode
 * @param ovr 0=Auto (decoder selects), 1=Manual (use WYSFM value)
 * Register 0x18, bit [7]
 */
void ADV_SetFilterWYShapingOvr(uint8_t ovr)
{
    FilterWYShapingOvr = ovr ? 1 : 0;

    /* Register 0x18 = (WYSFMOVR << 7) | WYSFM - bits 6 and 5 are reserved */
    uint8_t reg_val = ((FilterWYShapingOvr & 0x01) << 7) | (FilterWYShaping & 0x1F);

    const uint8_t batch[] = {
        DEVICE_ADDR, 0x18, reg_val
    };
    (void)I2C_TransmitBatch(batch, 1, TIMEOUT);

    printf("[ADV] Filter WY Override: %s\n", FilterWYShapingOvr ? "Manual" : "Auto");
}

/**
 * @brief Set Comb Filter bandwidth for NTSC
 * @param bw 0=Narrow, 1=Medium, 2=Medium, 3=Wide
 * Register 0x19, bits [3:2]
 */
void ADV_SetFilterCombNTSC(uint8_t bw)
{
    if (bw > 3) bw = 3;
    FilterCombNTSC = bw;

    /* Register 0x19 = 0xF0 | (NSFSEL << 2) | PSFSEL */
    uint8_t reg_val = 0xF0 | ((bw & 0x03) << 2) | (FilterCombPAL & 0x03);

    const uint8_t batch[] = {
        DEVICE_ADDR, 0x19, reg_val
    };
    (void)I2C_TransmitBatch(batch, 1, TIMEOUT);

    printf("[ADV] Filter Comb NTSC: %d\n", bw);
}

/**
 * @brief Set Comb Filter bandwidth for PAL
 * @param bw 0=Narrow, 1=Medium, 2=Wide, 3=Widest
 * Register 0x19, bits [1:0]
 */
void ADV_SetFilterCombPAL(uint8_t bw)
{
    if (bw > 3) bw = 3;
    FilterCombPAL = bw;

    /* Register 0x19 = 0xF0 | (NSFSEL << 2) | PSFSEL */
    uint8_t reg_val = 0xF0 | ((FilterCombNTSC & 0x03) << 2) | (bw & 0x03);

    const uint8_t batch[] = {
        DEVICE_ADDR, 0x19, reg_val
    };
    (void)I2C_TransmitBatch(batch, 1, TIMEOUT);

    printf("[ADV] Filter Comb PAL: %d\n", bw);
}

/**
 * @brief Apply all filter parameters to hardware
 * Call this after loading settings from flash or changing multiple params
 */
void ADV_SetFilterParams(void)
{
    /* Write all three filter registers in one batch */
    uint8_t reg17 = ((FilterCShaping & 0x07) << 5) | (FilterYShaping & 0x1F);
    uint8_t reg18 = ((FilterWYShapingOvr & 0x01) << 7) | (FilterWYShaping & 0x1F);  /* bit 7 for WYSFMOVR */
    uint8_t reg19 = 0xF0 | ((FilterCombNTSC & 0x03) << 2) | (FilterCombPAL & 0x03);

    const uint8_t batch[] = {
        DEVICE_ADDR, 0x17, reg17,
        DEVICE_ADDR, 0x18, reg18,
        DEVICE_ADDR, 0x19, reg19
    };
    (void)I2C_TransmitBatch(batch, 3, TIMEOUT);

    printf("[ADV] Filter Params: Y=%d C=%d WY=%d OVR=%d NTSC=%d PAL=%d\n",
           FilterYShaping, FilterCShaping, FilterWYShaping,
           FilterWYShapingOvr, FilterCombNTSC, FilterCombPAL);
}

/**
 * @brief Reset all filter parameters to defaults
 */
void ADV_SetFilterDefaults(void)
{
    FilterYShaping = 0;     /* Auto Wide */
    FilterCShaping = 0;     /* Auto 1.5MHz */
    FilterWYShaping = 0;    /* Auto */
    FilterWYShapingOvr = 0; /* Auto */
    FilterCombNTSC = 0;     /* Narrow */
    FilterCombPAL = 0;      /* Narrow */

    ADV_SetFilterParams();
    printf("[ADV] Filter Defaults restored\n");
}

/*******************************************************************************
 * Detection and Monitoring
 ******************************************************************************/

/**
 * @brief Main detection loop - monitors signal status
 */
void ADV_DetectLoop(void)
{
    if (Adv_7391_sw != 1)
        return;

    uint8_t detect_result, ad_result;
    uint8_t buff[2];

    buff[0] = 0x0E;
    buff[1] = 0x00;
    (void)I2C_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT);
    DDL_DelayMS(10);

    buff[0] = 0x0E;
    buff[1] = 0x00;
    (void)I2C_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT);

    buff[0] = 0x13;
    buff[1] = 0x10;
    I2C_Receive(DEVICE_ADDR, &buff[0], &detect_result, 1, TIMEOUT);
    I2C_Receive(DEVICE_ADDR, &buff[1], &ad_result, 1, TIMEOUT);

    ADV_DetectFormat(btn_flag);

    if (((uint8_t)(ad_result & 0x02) == 0x02) && !status && (err_flag)) {
        status = 1;
        printf("[ADV] Run Free Mode\n");
        c_state = 2;
        err_flag = 0;
        led_state = LED_RED;
    } else if (status && ((uint8_t)(ad_result & 0x05) == 0x05) && (err_flag)) {
        status = 0;
        err_flag = 0;
        printf("[ADV] Close Free Mode\n");
        LED_SetSignal(Input_signal);
    }
}

/*******************************************************************************
 * Analog Switch Control
 ******************************************************************************/

/**
 * @brief Set all analog switches
 */
void ASW_SetSwitches(uint8_t sw1, uint8_t sw2, uint8_t sw3, uint8_t sw4, uint8_t state)
{
    (void)state; /* No longer used - always apply */

    if (sw1) ASW1_On(); else ASW1_Off();
    if (sw2) ASW2_On(); else ASW2_Off();
    if (sw3) ASW3_On(); else ASW3_Off();
    if (sw4) ASW4_On(); else ASW4_Off();
}

/**
 * @brief Set AV connector state
 * @param sw true=connected, false=disconnected
 */
void ASW_SetAVConnect(uint8_t sw)
{
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    stcGpioInit.u16PinAttr = PIN_ATTR_DIGITAL;
    stcGpioInit.u16PinDrv = PIN_HIGH_DRV;
    stcGpioInit.u16PinOutputType = PIN_OUT_TYPE_CMOS;

    if (sw) {
        stcGpioInit.u16PullUp = PIN_PU_OFF;
        stcGpioInit.u16PinState = PIN_STAT_RST;
        printf("[ADV] Connecte_on\n");
        c_state = 1;
    } else {
        stcGpioInit.u16PullUp = PIN_PU_ON;
        stcGpioInit.u16PinState = PIN_STAT_SET;
        printf("[ADV] Connecte_off\n");
        c_state = 2;
    }

    (void)GPIO_Init(GPIO_PORT_A, GPIO_PIN_08, &stcGpioInit);
}

/*******************************************************************************
 * LED and Status
 ******************************************************************************/

/**
 * @brief Set LED color based on signal type
 * @param signal Signal type (1=RGBs, 2=RGsB, 3=VGA, 4=YUV, 5=SV, 6=AV)
 */
void LED_SetSignal(uint8_t signal)
{
    switch (signal) {
    case 1: /* RGBs */
        led_state = LED_RED | LED_GREEN | LED_BLUE;
        break;
    case 2: /* RGsB */
        led_state = LED_GREEN;
        break;
    case 3: /* VGA */
        led_state = LED_BLUE;
        break;
    case 4: /* YUV */
        led_state = LED_RED | LED_GREEN;
        break;
    case 5: /* SV */
        led_state = LED_RED | LED_BLUE;
        break;
    case 6: /* AV */
        led_state = LED_GREEN | LED_BLUE;
        break;
    }
}

/**
 * @brief Reset status flag
 */
void ADV_ResetStatus(void)
{
    status = 0;
}
