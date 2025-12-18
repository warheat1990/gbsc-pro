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
    adv_double = (u8_buf[2] < 2)  ? u8_buf[2] : 0;
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
    u8_buf[2]  = adv_double;
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
