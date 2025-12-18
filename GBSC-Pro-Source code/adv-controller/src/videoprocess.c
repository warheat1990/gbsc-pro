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
 * Global Variables - MUST match original initialization values exactly!
 ******************************************************************************/
uint8_t btn_flag;

static uint8_t status = 0;
uint8_t Adv_7391_sw = 0;
uint8_t adv_input;              /* Uninitialized - loaded from flash */
uint8_t adv_double = true;
uint8_t adv_smooth = false;
uint8_t adv_ace = false;
uint8_t adv_sw = false;         /* CRITICAL: Must be false, not true! */
uint8_t adv_tv = 0xff;          /* CRITICAL: Must be 0xff, not 0x04! */
uint8_t err_flag = 0;
uint8_t c_state = 0;

uint8_t Bright = 0x00;
uint8_t Contrast = 0x80;
uint8_t Saturation = 0x80;

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
    0x42, 0x38, 0x24,   // ADV7280 - NTSC comb control: Disable chroma comb, Uses low-pass/notch filter
    0x42, 0x39, 0x24,   // ADV7280 - PAL comb control: Disable chroma comb, Uses low-pass/notch filter
    0x42, 0x17, 0x49,   // ADV7280 - Shaping Filter Control 1: SH1, SVHS 8
};

/* CVBS (Composite) input configuration */
uint8_t I2C_COMMANDS_CVBS_INPUT[] = {
    /* =============== ADV7280 CVBS =============== */
    0x42, 0x0E, 0x00,   // ADV7280 - ADI Control 1: main register
    0x42, 0x00, 0x00,   // ADV7280 - Input control: CVBS input on A1
    0x42, 0x38, 0x80,   // ADV7280 - NTSC comb control: default
    0x42, 0x39, 0x24,   // ADV7280 - PAL comb control: Disable chroma comb, Uses low-pass/notch filter
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
        printf("525p format configured\n");
    }

    if (!is_525p) {
        (void)I2C_TransmitBatch(I2C_COMMANDS_625p_CONFIG,
                                  sizeof(I2C_COMMANDS_625p_CONFIG) / 3, TIMEOUT);
        printf("625p format configured\n");
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
        printf("No signal lock.\n");
        return;
    }

    if (!interlaced) {
        if (ad_result == 0x40) {
            printf("288p (PAL progressive)\n");
        } else if (ad_result == 0x00 || ad_result == 0x20) {
            printf("240p (NTSC progressive)\n");
        } else {
            printf("Unknown\n");
        }
    } else {
        if (ad_result == 0x40) {
            printf("576i (PAL interlaced)\n");
        } else if (ad_result == 0x00 || ad_result == 0x20) {
            printf("480i (NTSC interlaced)\n");
        } else {
            printf("Unknown\n");
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
    printf(" Init adv_tv: 0x%02x", adv_tv);
    (void)I2C_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT);

    /* Configure encoder and BCSH */
    ADV_ConfigureEncoder();
    ADV_SetBCSH();

    /* Configure I2P based on settings */
    if (adv_double) {
        ADV_SetI2P(adv_double);
        if (adv_smooth)
            ADV_SetSmooth(adv_smooth);
    } else {
        ADV_SetI2P(true);
        ADV_SetSmooth(true);
        adv_smooth = false;
        ADV_SetSmooth(adv_smooth);
        ADV_SetI2P(adv_double);
    }

    DDL_DelayMS(200);

    Adv_7391_sw = 1;
    printf("ModuleOn\n");
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
    printf("ModuleOff\n");
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
        printf("SvSignal\n");
    } else {
        (void)I2C_TransmitBatch(I2C_COMMANDS_CVBS_INPUT,
                                  sizeof(I2C_COMMANDS_CVBS_INPUT) / 3, TIMEOUT);
        printf("AvSignal\n");
    }
}

/**
 * @brief Enable/disable I2P (Interlace to Progressive) conversion
 * @param doubleline true to enable, false to disable
 */
void ADV_SetI2P(uint8_t doubleline)
{
    if (doubleline) {
        (void)I2C_TransmitBatch(I2C_COMMANDS_I2P_ON,
                                  sizeof(I2C_COMMANDS_I2P_ON) / 3, TIMEOUT);
        printf("I2pOn\n");
    } else {
        (void)I2C_TransmitBatch(I2C_COMMANDS_I2P_OFF_p,
                                  sizeof(I2C_COMMANDS_I2P_OFF_p) / 3, TIMEOUT);
        printf("I2pOff\n");
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
        printf("SmoothOn\n");
    } else {
        (void)I2C_TransmitBatch(I2C_COMMANDS_SMOOTH_OFF,
                                  sizeof(I2C_COMMANDS_SMOOTH_OFF) / 3, TIMEOUT);
        printf("SmoothOff\n");
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
    printf("Bright    : 0x%02x\n", Bright);
    printf("Contrast  : 0x%02x\n", Contrast);
    printf("Saturation: 0x%02x\n", Saturation);
}

/**
 * @brief Enable/disable ACE (Adaptive Contrast Enhancement)
 * @param ace true to enable, false to disable
 */
void ADV_SetACE(uint8_t ace)
{
    if (ace) {
        (void)I2C_TransmitBatch(Ace_Code_ON, sizeof(Ace_Code_ON) / 3, TIMEOUT);
        printf("AceOn\n");
    } else {
        (void)I2C_TransmitBatch(Ace_Code_OFF, sizeof(Ace_Code_OFF) / 3, TIMEOUT);
        printf("AceOff\n");
    }
}

/**
 * @brief Output format configuration (unused)
 */
void ADV_SetOutput(uint8_t output)
{
    (void)output;
}

/*******************************************************************************
 * Input Detection and Monitoring
 ******************************************************************************/

/**
 * @brief Read physical key changes for input selection
 */
void ADV_ReadKeyChange(void)
{
    static uint8_t key_state = 0, key_state_last = 0;

    key_state = GPIO_ReadInputPins(GPIO_PORT_B, GPIO_PIN_12);
    if (key_state_last != key_state) {
        adv_input = key_state;
        ADV_SetInput(adv_input);
    }
    key_state_last = key_state;

    static uint8_t key_line_state = 0, key_state_line_last = 0;
    key_line_state = GPIO_ReadInputPins(GPIO_PORT_B, GPIO_PIN_05);
    if (key_state_line_last != key_line_state) {
        ADV_SetI2P(key_line_state);
    }
    key_state_line_last = key_line_state;
}

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
        printf(" Run Free Mode \n");
        c_state = 2;
        err_flag = 0;
        led_state = LED_RED;
    } else if (status && ((uint8_t)(ad_result & 0x05) == 0x05) && (err_flag)) {
        status = 0;
        err_flag = 0;
        printf(" Close Free Mode \n");
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
    if ((sw1) && ((sw1 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW1)) || state))
        ASW1_On();
    else if ((!sw1) && ((sw1 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW1)) || state))
        ASW1_Off();

    if ((sw2) && ((sw2 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW2)) || state))
        ASW2_On();
    else if ((!sw2) && ((sw2 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW2)) || state))
        ASW2_Off();

    if ((sw3) && ((sw3 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW3)) || state))
        ASW3_On();
    else if ((!sw3) && ((sw3 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW3)) || state))
        ASW3_Off();

    if ((sw4) && ((sw4 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW4)) || state))
        ASW4_On();
    else if ((!sw4) && ((sw4 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW4)) || state))
        ASW4_Off();
}

/**
 * @brief Set analog switches except SW2
 */
void ASW_SetSwitchesNot02(uint8_t sw1, uint8_t sw2, uint8_t sw3, uint8_t sw4, uint8_t state)
{
    (void)sw2; /* SW2 not controlled by this function */

    if ((sw1) && ((sw1 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW1)) || state))
        ASW1_On();
    else if ((!sw1) && ((sw1 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW1)) || state))
        ASW1_Off();

    if ((sw3) && ((sw3 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW3)) || state))
        ASW3_On();
    else if ((!sw3) && ((sw3 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW3)) || state))
        ASW3_Off();

    if ((sw4) && ((sw4 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW4)) || state))
        ASW4_On();
    else if ((!sw4) && ((sw4 != GPIO_ReadInputPins(GPIO_PORT_ASW, GPIO_PIN_ASW4)) || state))
        ASW4_Off();
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
        printf("Connecte_on");
        c_state = 1;
    } else {
        stcGpioInit.u16PullUp = PIN_PU_ON;
        stcGpioInit.u16PinState = PIN_STAT_SET;
        printf("Connecte_off");
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

/**
 * @brief Print video info (placeholder)
 */
void ADV_Info(void)
{
    /* Placeholder for info output */
}
