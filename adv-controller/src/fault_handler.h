/**
 *******************************************************************************
 * @file  fault_handler.h
 * @brief Fault handlers and watchdog for crash detection/recovery
 *******************************************************************************
 */

#ifndef FAULT_HANDLER_H
#define FAULT_HANDLER_H

#include "main.h"

/**
 * @brief Initialize watchdog timer
 *        Timeout ~2.6 seconds @ PCLK3 = 50MHz
 *        WDT_CLK_DIV8192 + WDT_CNT_PERIOD16384 = 50MHz/8192/16384 ≈ 0.37Hz
 */
void WDT_Config(void);

/**
 * @brief Refresh watchdog - call periodically in main loop
 */
void WDT_Refresh(void);

#endif /* FAULT_HANDLER_H */
