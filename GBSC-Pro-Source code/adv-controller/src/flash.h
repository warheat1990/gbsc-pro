/**
 *******************************************************************************
 * @file  flash.h
 * @brief Flash driver for storing/loading video settings
 *******************************************************************************
 */
#ifndef __FLASH_H__
#define __FLASH_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "hc32_ll_efm.h"
#include "main.h"

/* Flash definitions */
#define FLASH_BASE                      (EFM_START_ADDR)
#define FLASH_SIZE                      (EFM_END_ADDR + 1U)
#define FLASH_SECTOR_SIZE               (SECTOR_SIZE)
#define FLASH_SECTOR_NUM                (64U)

/* SRAM definitions */
#define SRAM_SIZE                       (0x020000UL)
/* Vector table */
#define VECT_TAB_STEP                   (0x400UL)

/* Settings functions */
void FLASH_SaveSettings(void);      // Deferred save - actual write in FLASH_Task()
void FLASH_LoadSettings(void);      // Load all settings from flash
void FLASH_Task(void);              // Call from main loop to handle deferred saves

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_H__ */
