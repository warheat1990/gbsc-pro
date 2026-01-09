/**
 *******************************************************************************
 * @file  fault_handler.c
 * @brief Fault handlers and watchdog for crash detection/recovery
 *
 * Provides:
 * - HardFault handler with LED indication and auto-reset
 * - Watchdog timer for automatic recovery from hangs
 *******************************************************************************
 */

#include "fault_handler.h"
#include "hc32_ll_wdt.h"
#include "ev_hc32f460_lqfp100_v2_bsp.h"

/*******************************************************************************
 * Watchdog Configuration
 ******************************************************************************/

void WDT_Config(void)
{
    stc_wdt_init_t stcWdtInit;

    /*
     * Watchdog timeout calculation:
     * PCLK3 = 50MHz (assuming default clock config)
     * Divider = 8192
     * Period = 16384 cycles
     * Timeout = 16384 / (50MHz / 8192) = 16384 / 6103 ≈ 2.68 seconds
     *
     * This gives enough margin for normal operation while catching hangs.
     */
    stcWdtInit.u32CountPeriod   = WDT_CNT_PERIOD16384;
    stcWdtInit.u32ClockDiv      = WDT_CLK_DIV8192;
    stcWdtInit.u32RefreshRange  = WDT_RANGE_0TO100PCT;  /* Allow refresh anytime */
    stcWdtInit.u32LPMCount      = WDT_LPM_CNT_CONTINUE; /* Keep counting in sleep */
    stcWdtInit.u32ExceptionType = WDT_EXP_TYPE_RST;     /* Reset on timeout */

    (void)WDT_Init(&stcWdtInit);

    /* First refresh starts the watchdog */
    WDT_FeedDog();

    printf("[WDT] Watchdog enabled (timeout ~2.7s)\n");
}

void WDT_Refresh(void)
{
    WDT_FeedDog();
}

/*******************************************************************************
 * Fault Handlers
 *
 * These override the weak Default_Handler from startup_hc32f460.S
 * On fault: indicate error via LED, then reset the system.
 ******************************************************************************/

/**
 * @brief Hard Fault Handler - catches most severe errors
 *        (invalid memory access, divide by zero, etc.)
 */
void HardFault_Handler(void)
{
    __disable_irq();

    /* Visual indication: RED LED solid */
    BSP_LED_Off(LED_ALL);
    BSP_LED_On(LED_RED);

    /* Brief delay so LED is visible before reset */
    for (volatile uint32_t i = 0; i < 2000000; i++) {
        __NOP();
    }

    /* Reset the system */
    NVIC_SystemReset();

    /* Should never reach here */
    for (;;) {
        __NOP();
    }
}

/**
 * @brief Memory Management Fault Handler
 */
void MemManage_Handler(void)
{
    __disable_irq();
    BSP_LED_Off(LED_ALL);
    BSP_LED_On(LED_RED);
    BSP_LED_On(LED_GREEN);  /* Yellow = MemManage */

    for (volatile uint32_t i = 0; i < 2000000; i++) {
        __NOP();
    }

    NVIC_SystemReset();

    for (;;) {
        __NOP();
    }
}

/**
 * @brief Bus Fault Handler
 */
void BusFault_Handler(void)
{
    __disable_irq();
    BSP_LED_Off(LED_ALL);
    BSP_LED_On(LED_RED);
    BSP_LED_On(LED_BLUE);  /* Magenta = BusFault */

    for (volatile uint32_t i = 0; i < 2000000; i++) {
        __NOP();
    }

    NVIC_SystemReset();

    for (;;) {
        __NOP();
    }
}

/**
 * @brief Usage Fault Handler (undefined instruction, unaligned access, etc.)
 */
void UsageFault_Handler(void)
{
    __disable_irq();
    BSP_LED_Off(LED_ALL);
    BSP_LED_On(LED_GREEN);
    BSP_LED_On(LED_BLUE);  /* Cyan = UsageFault */

    for (volatile uint32_t i = 0; i < 2000000; i++) {
        __NOP();
    }

    NVIC_SystemReset();

    for (;;) {
        __NOP();
    }
}
