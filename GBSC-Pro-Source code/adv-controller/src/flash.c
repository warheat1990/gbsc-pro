/**
 *******************************************************************************
 * @file  flash.c
 * @brief Flash driver for storing/loading video settings
 *******************************************************************************
 */

#include "flash.h"
#include "videoprocess.h"

// Deferred save: wait 30 ticks (3 seconds) after last change
// FLASH_Task() called every 100ms, so 30 ticks = 3 seconds
#define FLASH_SAVE_DELAY_TICKS  30

static uint8_t  flash_pending = 0;
static uint16_t flash_delay_counter = 0;
static uint8_t  u8_buf[64];

/**
 * @brief  Load all settings from flash
 */
void FLASH_LoadSettings(void)
{
    // Read directly from flash memory
    uint32_t *src = (uint32_t *)(EFM_BASE + FLASH_LEAF_ADDR(USER));
    uint32_t *dst = (uint32_t *)u8_buf;
    for (int i = 0; i < 16; i++)
        dst[i] = src[i];

    // Check header
    if (u8_buf[0] != 0xbc)
    {
        printf("[FLASH] No valid settings, using defaults\n");
        return;
    }

    // Load settings with validation
    adv_input  = (u8_buf[1] < 2)  ? u8_buf[1] : 0;
    adv_i2p    = (u8_buf[2] < 2)  ? u8_buf[2] : 0;
    adv_smooth = (u8_buf[3] < 2)  ? u8_buf[3] : 0;
    adv_ace    = (u8_buf[4] < 2)  ? u8_buf[4] : 0;
    adv_sw     = (u8_buf[5] < 2)  ? u8_buf[5] : 0;
    adv_tv     = (u8_buf[6] < 0xf5) ? u8_buf[6] : 0x04;

    asw_01 = (u8_buf[7] < 2)  ? u8_buf[7] : 0;
    asw_02 = (u8_buf[8] < 2)  ? u8_buf[8] : 0;
    asw_03 = (u8_buf[9] < 2)  ? u8_buf[9] : 0;
    asw_04 = (u8_buf[10] < 2) ? u8_buf[10] : 0;
    AVsw   = (u8_buf[11] < 2) ? u8_buf[11] : 0;

    Bright     = (u8_buf[12] != 0xff) ? u8_buf[12] : 0x00;
    Contrast   = (u8_buf[13] != 0xff) ? u8_buf[13] : 0x80;
    Saturation = (u8_buf[14] != 0xff) ? u8_buf[14] : 0x80;

    /* ACE parameters (offsets 15-19) */
    AceLumaGain      = (u8_buf[15] <= 31) ? u8_buf[15] : 13;
    AceChromaGain    = (u8_buf[16] <= 15) ? u8_buf[16] : 8;
    AceChromaMax     = (u8_buf[17] <= 15) ? u8_buf[17] : 8;
    AceGammaGain     = (u8_buf[18] <= 15) ? u8_buf[18] : 8;
    AceResponseSpeed = (u8_buf[19] <= 15) ? u8_buf[19] : 15;

    /* Video Filter parameters (offsets 20-25) */
    FilterYShaping     = (u8_buf[20] <= 31) ? u8_buf[20] : 0;
    FilterCShaping     = (u8_buf[21] <= 7)  ? u8_buf[21] : 0;
    FilterWYShaping    = (u8_buf[22] <= 31) ? u8_buf[22] : 0;
    FilterWYShapingOvr = (u8_buf[23] <= 1)  ? u8_buf[23] : 0;
    FilterCombNTSC     = (u8_buf[24] <= 3)  ? u8_buf[24] : 0;
    FilterCombPAL      = (u8_buf[25] <= 3)  ? u8_buf[25] : 0;

    Input_signal = u8_buf[48];

    printf("[FLASH] Loaded: adv_sw=%d adv_tv=0x%02x\n", adv_sw, adv_tv);
}

/**
 * @brief  Mark settings for deferred save
 */
void FLASH_SaveSettings(void)
{
    flash_pending = 1;
    flash_delay_counter = 0;
}

/**
 * @brief  Save settings to flash (called by FLASH_Task after delay)
 */
static void FLASH_SaveSettingsNow(void)
{
    // Erase sector
    EFM_SectorErase(EFM_BASE + FLASH_LEAF_ADDR(USER));

    // Prepare buffer
    u8_buf[0]  = 0xbc;
    u8_buf[1]  = adv_input;
    u8_buf[2]  = adv_i2p;
    u8_buf[3]  = adv_smooth;
    u8_buf[4]  = adv_ace;
    u8_buf[5]  = adv_sw;
    u8_buf[6]  = adv_tv;
    u8_buf[7]  = asw_01;
    u8_buf[8]  = asw_02;
    u8_buf[9]  = asw_03;
    u8_buf[10] = asw_04;
    u8_buf[11] = AVsw;
    u8_buf[12] = Bright;
    u8_buf[13] = Contrast;
    u8_buf[14] = Saturation;

    /* ACE parameters (offsets 15-19) */
    u8_buf[15] = AceLumaGain;
    u8_buf[16] = AceChromaGain;
    u8_buf[17] = AceChromaMax;
    u8_buf[18] = AceGammaGain;
    u8_buf[19] = AceResponseSpeed;

    /* Video Filter parameters (offsets 20-25) */
    u8_buf[20] = FilterYShaping;
    u8_buf[21] = FilterCShaping;
    u8_buf[22] = FilterWYShaping;
    u8_buf[23] = FilterWYShapingOvr;
    u8_buf[24] = FilterCombNTSC;
    u8_buf[25] = FilterCombPAL;

    u8_buf[48] = Input_signal;

    // Write to flash
    EFM_Program(EFM_BASE + FLASH_LEAF_ADDR(USER), u8_buf, 64);

    flash_pending = 0;
    printf("[FLASH] Settings saved\n");
}

/**
 * @brief  Call from main loop - saves to flash after delay
 */
void FLASH_Task(void)
{
    if (flash_pending && ++flash_delay_counter >= FLASH_SAVE_DELAY_TICKS)
    {
        FLASH_SaveSettingsNow();
    }
}
