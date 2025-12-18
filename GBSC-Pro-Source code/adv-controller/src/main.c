/**
 *******************************************************************************
 * @file  main.c
 * @brief ADV Controller - Main entry point
 *******************************************************************************
 */

#include "main.h"
#include "debug_uart.h"
#include "adv_cli.h"
#include "flash.h"

#define LL_PERIPH_SEL (LL_PERIPH_GPIO | LL_PERIPH_FCG | LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_EFM | LL_PERIPH_SRAM)

extern uint16_t g_u16_sys_timer;
extern uint16_t g_u16_key_timer;
extern uint16_t g_u16_mis_timer;
extern uint16_t g_u16_osd_timer;

static void GPIO_Init_Output(uint8_t port, uint16_t pin, stc_gpio_init_t *cfg)
{
    (void)GPIO_Init(port, pin, cfg);
}

static void GPIO_InitPins(void)
{
    stc_gpio_init_t cfg;

    // Output pins (PB01, PB00, PA03)
    (void)GPIO_StructInit(&cfg);
    cfg.u16PullUp   = PIN_PU_ON;
    cfg.u16PinDir   = PIN_DIR_OUT;
    cfg.u16PinAttr  = PIN_ATTR_DIGITAL;
    cfg.u16PinState = PIN_STAT_RST;
    GPIO_Init_Output(GPIO_PORT_B, GPIO_PIN_01, &cfg);
    GPIO_Init_Output(GPIO_PORT_B, GPIO_PIN_00, &cfg);
    GPIO_Init_Output(GPIO_PORT_A, GPIO_PIN_03, &cfg);

    // Input pin (PB05)
    cfg.u16PinDir   = PIN_DIR_IN;
    cfg.u16PinState = PIN_STAT_SET;
    GPIO_Init_Output(GPIO_PORT_B, GPIO_PIN_05, &cfg);

    // Analog switch outputs (high drive)
    cfg.u16PullUp        = PIN_PU_OFF;
    cfg.u16PinDir        = PIN_DIR_OUT;
    cfg.u16PinState      = PIN_STAT_RST;
    cfg.u16PinDrv        = PIN_HIGH_DRV;
    cfg.u16PinOutputType = PIN_OUT_TYPE_CMOS;
    GPIO_Init_Output(GPIO_PORT_ASW, GPIO_PIN_ASW1, &cfg);
    GPIO_Init_Output(GPIO_PORT_ASW, GPIO_PIN_ASW3, &cfg);
    GPIO_Init_Output(GPIO_PORT_ASW, GPIO_PIN_ASW4, &cfg);

    // Load settings from flash
    FLASH_LoadSettings();

    // Configure ASW2 and AV connect based on loaded settings
    cfg.u16PinState = asw_02 ? PIN_STAT_SET : PIN_STAT_RST;
    GPIO_Init_Output(GPIO_PORT_ASW, GPIO_PIN_ASW2, &cfg);

    cfg.u16PullUp   = AVsw ? PIN_PU_OFF : PIN_PU_ON;
    cfg.u16PinState = AVsw ? PIN_STAT_RST : PIN_STAT_SET;
    GPIO_Init_Output(GPIO_PORT_A, GPIO_PIN_08, &cfg);

    ASW_SetSwitches(asw_01, asw_02, asw_03, asw_04, 1);
}

static void LED_UpdateState(void)
{
    static uint8_t last_signal = 0;

    switch (c_state)
    {
        case 1: C_LED_OK();        break;
        case 2: C_LED_ERR_RED();   break;
        case 3: C_LED_ERR_GREEN(); break;
        case 4: C_LED_ERR_BLUE();  break;
    }
    c_state = 0;

    if (Input_signal != last_signal)
    {
        LED_SetSignal(Input_signal);
        last_signal = Input_signal;
    }
}

int main(void)
{
    // System init
    LL_PERIPH_WE(LL_PERIPH_SEL);
    __enable_irq();
    EFM_FWMC_Cmd(ENABLE);
    BSP_CLK_Init();
    BSP_LED_Init();
    GPIO_InitPins();

#if (LL_TMR0_ENABLE == DDL_ON)
    TMR02_A_Config();
#endif
#if (LL_PRINT_ENABLE == DDL_ON)
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_Preinit);
#endif

    // Peripheral init
    I2C_DriverInit();
    TMR0_Start(CM_TMR0_2, TMR0_CHA);
    UART_DMA_Init();
    DebugUart_Init();
    ADVCLI_Init();

    printf("\r\n[SYSTEM] ADV Controller ready\r\n");

    ADV_Enable(adv_sw);
    LED_SetSignal(Input_signal);

    // Main loop
    for (;;)
    {
        ADVCLI_Task();

        if (g_u16_sys_timer >= SYS_TIMEOUT_100MS)
        {
            ADV_DetectLoop();
            FLASH_Task();
            g_u16_sys_timer = 0;
        }

        if (g_u16_key_timer >= SYS_TIMEOUT_50MS)
        {
            UART_ProcessCommand();
            g_u16_key_timer = 0;
        }

        if (g_u16_mis_timer >= SYS_TIMEOUT_100MS)
        {
            LED_UpdateState();
            g_u16_mis_timer = 0;
        }

        if (g_u16_osd_timer >= SYS_TIMEOUT_500MS)
        {
            err_flag = 1;
            g_u16_osd_timer = 0;
        }
    }
}
