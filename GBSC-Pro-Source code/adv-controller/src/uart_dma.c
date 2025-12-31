/**
 *******************************************************************************
 * @file  uart_dma.c
 * @brief UART DMA communication with gbs-control (ESP8266)
 *        Uses USART4 on PB06(TX) / PB07(RX) @ 115200 baud
 *        Protocol: [0x41 0x44] [cmd] [data...] [0xFE] [checksum]
 *******************************************************************************
 */

#include "uart_dma.h"
#include "main.h"
#include "flash.h"

uint8_t dma_au8RxBuf[APP_FRAME_LEN_MAX];

static void RX_DMA_TC_IrqCallback(void)
{
    m_enRxFrameEnd = SET;
    USART_FuncCmd(USART_UNIT, USART_RX_TIMEOUT, DISABLE);
    DMA_ClearTransCompleteStatus(RX_DMA_UNIT, RX_DMA_TC_FLAG);
}

static void TX_DMA_TC_IrqCallback(void)
{
    USART_FuncCmd(USART_UNIT, USART_INT_TX_CPLT, ENABLE);
    DMA_ClearTransCompleteStatus(TX_DMA_UNIT, TX_DMA_TC_FLAG);
}

int32_t DMA_Config(void)
{
    int32_t                         i32Ret;
    stc_dma_init_t                  stcDmaInit;
    stc_dma_llp_init_t              stcDmaLlpInit;
    stc_irq_signin_config_t         stcIrqSignConfig;
    static stc_dma_llp_descriptor_t stcLlpDesc;

    RX_DMA_FCG_ENABLE();
    TX_DMA_FCG_ENABLE();
    FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_AOS, ENABLE);

    (void)DMA_StructInit(&stcDmaInit);
    stcDmaInit.u32IntEn       = DMA_INT_ENABLE;
    stcDmaInit.u32BlockSize   = 1UL;
    stcDmaInit.u32TransCount  = ARRAY_SZ(dma_au8RxBuf);
    stcDmaInit.u32DataWidth   = DMA_DATAWIDTH_8BIT;
    stcDmaInit.u32DestAddr    = (uint32_t)dma_au8RxBuf;
    stcDmaInit.u32SrcAddr     = (uint32_t)(&USART_UNIT->RDR);
    stcDmaInit.u32SrcAddrInc  = DMA_SRC_ADDR_FIX;
    stcDmaInit.u32DestAddrInc = DMA_DEST_ADDR_INC;
    i32Ret                    = DMA_Init(RX_DMA_UNIT, RX_DMA_CH, &stcDmaInit);
    if (LL_OK == i32Ret)
    {
        (void)DMA_LlpStructInit(&stcDmaLlpInit);
        stcDmaLlpInit.u32State = DMA_LLP_ENABLE;
        stcDmaLlpInit.u32Mode  = DMA_LLP_WAIT;
        stcDmaLlpInit.u32Addr  = (uint32_t)&stcLlpDesc;
        (void)DMA_LlpInit(RX_DMA_UNIT, RX_DMA_CH, &stcDmaLlpInit);

        stcLlpDesc.SARx = stcDmaInit.u32SrcAddr;
        stcLlpDesc.DARx = stcDmaInit.u32DestAddr;
        stcLlpDesc.DTCTLx =
            (stcDmaInit.u32TransCount << DMA_DTCTL_CNT_POS) | (stcDmaInit.u32BlockSize << DMA_DTCTL_BLKSIZE_POS);
        stcLlpDesc.LLPx   = (uint32_t)&stcLlpDesc;
        stcLlpDesc.CHCTLx = stcDmaInit.u32SrcAddrInc | stcDmaInit.u32DestAddrInc | stcDmaInit.u32DataWidth |
                            stcDmaInit.u32IntEn | stcDmaLlpInit.u32State | stcDmaLlpInit.u32Mode;

        DMA_ReconfigLlpCmd(RX_DMA_UNIT, RX_DMA_CH, ENABLE);
        DMA_ReconfigCmd(RX_DMA_UNIT, ENABLE);
        AOS_SetTriggerEventSrc(RX_DMA_RECONF_TRIG_SEL, RX_DMA_RECONF_TRIG_EVT_SRC);

        stcIrqSignConfig.enIntSrc    = RX_DMA_TC_INT_SRC;
        stcIrqSignConfig.enIRQn      = RX_DMA_TC_IRQn;
        stcIrqSignConfig.pfnCallback = &RX_DMA_TC_IrqCallback;
        (void)INTC_IrqSignIn(&stcIrqSignConfig);
        NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
        NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
        NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

        AOS_SetTriggerEventSrc(RX_DMA_TRIG_SEL, RX_DMA_TRIG_EVT_SRC);

        DMA_Cmd(RX_DMA_UNIT, ENABLE);
        DMA_TransCompleteIntCmd(RX_DMA_UNIT, RX_DMA_TC_INT, ENABLE);
        (void)DMA_ChCmd(RX_DMA_UNIT, RX_DMA_CH, ENABLE);
    }

    (void)DMA_StructInit(&stcDmaInit);
    stcDmaInit.u32IntEn       = DMA_INT_ENABLE;
    stcDmaInit.u32BlockSize   = 1UL;
    stcDmaInit.u32TransCount  = ARRAY_SZ(dma_au8RxBuf);
    stcDmaInit.u32DataWidth   = DMA_DATAWIDTH_8BIT;
    stcDmaInit.u32DestAddr    = (uint32_t)(&USART_UNIT->TDR);
    stcDmaInit.u32SrcAddr     = (uint32_t)dma_au8RxBuf;
    stcDmaInit.u32SrcAddrInc  = DMA_SRC_ADDR_INC;
    stcDmaInit.u32DestAddrInc = DMA_DEST_ADDR_FIX;
    i32Ret                    = DMA_Init(TX_DMA_UNIT, TX_DMA_CH, &stcDmaInit);
    if (LL_OK == i32Ret)
    {
        stcIrqSignConfig.enIntSrc    = TX_DMA_TC_INT_SRC;
        stcIrqSignConfig.enIRQn      = TX_DMA_TC_IRQn;
        stcIrqSignConfig.pfnCallback = &TX_DMA_TC_IrqCallback;
        (void)INTC_IrqSignIn(&stcIrqSignConfig);
        NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
        NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
        NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);

        AOS_SetTriggerEventSrc(TX_DMA_TRIG_SEL, TX_DMA_TRIG_EVT_SRC);

        DMA_Cmd(TX_DMA_UNIT, ENABLE);
        DMA_TransCompleteIntCmd(TX_DMA_UNIT, TX_DMA_TC_INT, ENABLE);
    }

    return i32Ret;
}

void TMR0_Config(uint16_t u16TimeoutBits)
{
    uint16_t        u16Div;
    uint16_t        u16Delay;
    uint16_t        u16CompareValue;
    stc_tmr0_init_t stcTmr0Init;

    DMATMR0_FCG_ENABLE();

    stcTmr0Init.u32ClockSrc = TMR0_CLK_SRC_XTAL32;
    stcTmr0Init.u32ClockDiv = TMR0_CLK_DIV8;
    stcTmr0Init.u32Func     = TMR0_FUNC_CMP;
    if (TMR0_CLK_DIV1 == stcTmr0Init.u32ClockDiv)
    {
        u16Delay = 7U;
    }
    else if (TMR0_CLK_DIV2 == stcTmr0Init.u32ClockDiv)
    {
        u16Delay = 5U;
    }
    else if ((TMR0_CLK_DIV4 == stcTmr0Init.u32ClockDiv) || (TMR0_CLK_DIV8 == stcTmr0Init.u32ClockDiv) ||
             (TMR0_CLK_DIV16 == stcTmr0Init.u32ClockDiv))
    {
        u16Delay = 3U;
    }
    else
    {
        u16Delay = 2U;
    }

    u16Div                      = (uint16_t)1U << (stcTmr0Init.u32ClockDiv >> TMR0_BCONR_CKDIVA_POS);
    u16CompareValue             = ((u16TimeoutBits + u16Div - 1U) / u16Div) - u16Delay;
    stcTmr0Init.u16CompareValue = u16CompareValue;
    (void)TMR0_Init(DMATMR0_UNIT, DMATMR0_CH, &stcTmr0Init);

    TMR0_HWStartCondCmd(DMATMR0_UNIT, DMATMR0_CH, ENABLE);
    TMR0_HWClearCondCmd(DMATMR0_UNIT, DMATMR0_CH, ENABLE);
}

void USART_RxTimeout_IrqCallback(void)
{
    if (m_enRxFrameEnd != SET)
    {
        m_enRxFrameEnd = SET;
        AOS_SW_Trigger();
    }
    TMR0_Stop(DMATMR0_UNIT, DMATMR0_CH);
    USART_ClearStatus(USART_UNIT, USART_FLAG_RX_TIMEOUT);
}

void USART_TxComplete_IrqCallback(void)
{
    USART_FuncCmd(USART_UNIT, (USART_TX | USART_INT_TX_CPLT), DISABLE);
    TMR0_Stop(DMATMR0_UNIT, DMATMR0_CH);
    USART_ClearStatus(USART_UNIT, USART_FLAG_RX_TIMEOUT);
    USART_FuncCmd(USART_UNIT, USART_RX_TIMEOUT, ENABLE);
    USART_ClearStatus(USART_UNIT, USART_FLAG_TX_CPLT);
}

void USART_RxError_IrqCallback(void)
{
    (void)USART_ReadData(USART_UNIT);

    USART_ClearStatus(USART_UNIT, (USART_FLAG_PARITY_ERR | USART_FLAG_FRAME_ERR | USART_FLAG_OVERRUN));
}

void UART_DMA_Init(void)
{
    stc_usart_uart_init_t   stcUartInit;
    stc_irq_signin_config_t stcIrqSigninConfig;

    (void)DMA_Config();
    GPIO_SetFunc(USART_RX_PORT, USART_RX_PIN, USART_RX_GPIO_FUNC);
    GPIO_SetFunc(USART_TX_PORT, USART_TX_PIN, USART_TX_GPIO_FUNC);

    USART_FCG_ENABLE();
    (void)USART_UART_StructInit(&stcUartInit);
    stcUartInit.u32ClockDiv      = USART_CLK_DIV64;
    stcUartInit.u32CKOutput      = USART_CK_OUTPUT_ENABLE;
    stcUartInit.u32Baudrate      = USART_BAUDRATE;
    stcUartInit.u32OverSampleBit = USART_OVER_SAMPLE_8BIT;
    if (LL_OK != USART_UART_Init(USART_UNIT, &stcUartInit, NULL))
    {
        BSP_LED_On(LED_RED);
        for (;;)
        {
        }
    }

    stcIrqSigninConfig.enIRQn      = USART_TX_CPLT_IRQn;
    stcIrqSigninConfig.enIntSrc    = USART_TX_CPLT_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_TxComplete_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSigninConfig);
    NVIC_ClearPendingIRQ(stcIrqSigninConfig.enIRQn);
    NVIC_SetPriority(stcIrqSigninConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSigninConfig.enIRQn);

    stcIrqSigninConfig.enIRQn      = USART_RX_ERR_IRQn;
    stcIrqSigninConfig.enIntSrc    = USART_RX_ERR_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_RxError_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSigninConfig);
    NVIC_ClearPendingIRQ(stcIrqSigninConfig.enIRQn);
    NVIC_SetPriority(stcIrqSigninConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSigninConfig.enIRQn);

    stcIrqSigninConfig.enIRQn      = USART_RX_TIMEOUT_IRQn;
    stcIrqSigninConfig.enIntSrc    = USART_RX_TIMEOUT_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_RxTimeout_IrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSigninConfig);
    NVIC_ClearPendingIRQ(stcIrqSigninConfig.enIRQn);
    NVIC_SetPriority(stcIrqSigninConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSigninConfig.enIRQn);

    TMR0_Config(USART_TIMEOUT_BITS);
    USART_FuncCmd(USART_UNIT, (USART_RX | USART_INT_RX | USART_RX_TIMEOUT | USART_INT_RX_TIMEOUT), ENABLE);
}

static const struct
{
    uint8_t     reg;
    const char *name;
} video_modes[] = {
    {0x04, "auto"},           // 0x01
    {0x84, "PAL"},            // 0x02
    {0x54, "NTSC-M"},         // 0x03
    {0x64, "PAL-60"},         // 0x04
    {0x74, "NTSC443"},        // 0x05
    {0x44, "NTSC-J"},         // 0x06
    {0x94, "PAL-N w p"},      // 0x07
    {0xA4, "PAL-M wo p"},     // 0x08
    {0xB4, "PAL-M"},          // 0x09
    {0xC4, "PAL Cmb -N"},     // 0x0A
    {0xD4, "PAL Cmb -N w p"}, // 0x0B
    {0xE4, "SECAM"},          // 0x0C
};

static void UART_SetVideoMode(const uint8_t mode_byte)
{
    uint8_t mode = mode_byte & 0x0f;
    if (mode == 0)
        return;

    uint8_t buff[2] = {VID_SEL_REG, 0};

    if (mode >= 1 && mode <= 12)
    {
        adv_tv  = video_modes[mode - 1].reg;
        buff[1] = adv_tv;
        (void)I2C_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT);
        printf("%s\n", video_modes[mode - 1].name);
        c_state = 1;
    }
    else
    {
        adv_tv  = 0x04;
        buff[1] = adv_tv;
        (void)I2C_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT);
        printf("Err Default Auto\n");
        c_state = 2;
    }
}

void UART_ProcessCommand(void)
{
    uint8_t buff[2];

    if (SET == m_enRxFrameEnd)
    {
        /* Check header first */
        if (dma_au8RxBuf[0] != 0x41 || dma_au8RxBuf[1] != 0x44)
        {
            m_enRxFrameEnd = RESET;
            memset(dma_au8RxBuf, 0, sizeof(dma_au8RxBuf));
            return;
        }

        /* Handle Custom I2C command with variable length */
        if (dma_au8RxBuf[2] == 'C')
        {
            uint8_t count = dma_au8RxBuf[3];
            /* Max triplets limited by buffer size: (APP_FRAME_LEN_MAX - 6) / 3 */
            uint8_t max_count = (APP_FRAME_LEN_MAX - 6) / 3;
            if (count > 0 && count <= max_count)
            {
                /* Variable length packet:
                 * [0x41 0x44] ['C'] [count] [data...] [0xFE] [checksum]
                 * Data length = count * 3
                 * 0xFE position = 4 + count*3
                 * checksum position = 5 + count*3
                 */
                uint16_t data_len   = count * 3;
                uint16_t fe_pos     = 4 + data_len;
                uint16_t chksum_pos = fe_pos + 1;

                /* Calculate checksum for variable length packet */
                uint8_t calc_sum = 0;
                for (uint16_t i = 0; i <= fe_pos; i++)
                {
                    calc_sum += dma_au8RxBuf[i];
                }

                if (dma_au8RxBuf[fe_pos] == 0xFE && dma_au8RxBuf[chksum_pos] == calc_sum)
                {
                    uint8_t *i2c_data = &dma_au8RxBuf[4];
                    (void)I2C_TransmitBatch(i2c_data, count, TIMEOUT);
                    printf("Custom I2C: %d cmds\n", count);
                    c_state = 1;
                }
                else
                {
                    printf("Custom I2C: chksum err\n");
                }
            }
            m_enRxFrameEnd = RESET;
            memset(dma_au8RxBuf, 0, sizeof(dma_au8RxBuf));
            return;
        }

        /* Standard fixed-length packet validation (7 bytes) */
        buff[1] =
            dma_au8RxBuf[5] + dma_au8RxBuf[4] + dma_au8RxBuf[3] + dma_au8RxBuf[2] + dma_au8RxBuf[1] + dma_au8RxBuf[0];
        if (dma_au8RxBuf[5] != 0xFE || dma_au8RxBuf[6] != buff[1])
        {
            m_enRxFrameEnd = RESET;
            memset(dma_au8RxBuf, 0, sizeof(dma_au8RxBuf));
            return;
        }
        else
        {
            if (dma_au8RxBuf[2] == 'T')
            {
                adv_tv  = dma_au8RxBuf[3];
                FLASH_SaveSettings();
                buff[0] = VID_SEL_REG;
                buff[1] = adv_tv;
                (void)I2C_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT);
                printf("Tv 0x%02x\n", adv_tv);
                c_state = 1;
            }
            else if (dma_au8RxBuf[2] == 'N' && (dma_au8RxBuf[3] == 0x0a || dma_au8RxBuf[3] == 0x08 ||
                                                dma_au8RxBuf[3] == 0xe3 || dma_au8RxBuf[3] == 'D'))
            {
                if (dma_au8RxBuf[3] == 'D' && dma_au8RxBuf[4] == 'E')
                {
                    uint8_t I2C_DEFAULT_BCSH[] = {
                        0x42, 0x0E, 0x00, // ADV7280 - ADI Control 1: main register
                        0x42, 0x0A, 0x00, // ADV7280 - Brightness adjust: 0 IRE
                        0x42, 0x08, 0x80, // ADV7280 - Contrast: 1 gain
                        0x42, 0xE3, 0x80, // ADV7280 - SD saturation Cb channel: 0dB
                        0x42, 0x0B, 0x00, // ADV7280 - Hue adjust: 0 default
                    };
                    Bright     = 0x00;
                    Contrast   = 0x80;
                    Saturation = 0x80;

                    (void)I2C_TransmitBatch(I2C_DEFAULT_BCSH, sizeof(I2C_DEFAULT_BCSH) / 3, TIMEOUT);
                    printf("bcsh: default \n");
                }
                else
                {
                    (void)I2C_Transmit(DEVICE_ADDR, &dma_au8RxBuf[3], 2, TIMEOUT);
                    printf("bcsh: 0x%02x \n", dma_au8RxBuf[4]);
                    if (dma_au8RxBuf[3] == 0x0a)
                        Bright = dma_au8RxBuf[4];
                    else if (dma_au8RxBuf[3] == 0x08)
                        Contrast = dma_au8RxBuf[4];
                    else if (dma_au8RxBuf[3] == 0xe3)
                        Saturation = dma_au8RxBuf[4];
                }
                FLASH_SaveSettings();
                c_state = 1;
            }
            else if (dma_au8RxBuf[2] == 'S')
            {
                if (dma_au8RxBuf[3] == 0xfd)
                {
                    adv_sw = true;
                    FLASH_SaveSettings();
                    ADV_Init();
                    c_state = 1;
                }
                else if (dma_au8RxBuf[3] == 0x01)
                {
                    adv_sw = false;
                    FLASH_SaveSettings();
                    ADV_Deinit();
                    c_state = 1;
                }
                else if ((dma_au8RxBuf[3] & 0xf0) == 0x10)
                {
                    AVsw   = 1;
                    asw_01 = 0;
                    asw_02 = 0;
                    asw_03 = 1;
                    asw_04 = 0;
                    ASW_SetSwitches(asw_01, asw_02, asw_03, asw_04, 0);
                    ASW_SetAVConnect(AVsw);
                    adv_sw    = true;
                    adv_input = SV_INPUT;
                    UART_SetVideoMode(dma_au8RxBuf[3] & 0x0f);
                    ADV_Init();
                    Input_signal = 5;
                    FLASH_SaveSettings();
                    printf("mode 0x%02x ", dma_au8RxBuf[3]);
                    c_state = 1;
                }
                else if ((dma_au8RxBuf[3] & 0xf0) == 0x20)
                {
                    AVsw   = 1;
                    asw_01 = 0;
                    asw_02 = 0;
                    asw_03 = 1;
                    asw_04 = 0;
                    ASW_SetSwitches(asw_01, asw_02, asw_03, asw_04, 0);
                    ASW_SetAVConnect(AVsw);
                    adv_sw    = true;
                    adv_input = AV_INPUT;
                    UART_SetVideoMode(dma_au8RxBuf[3] & 0x0f);
                    ADV_Init();
                    Input_signal = 6;
                    FLASH_SaveSettings();
                    printf("mode 0x%02x ", dma_au8RxBuf[3]);
                    c_state = 1;
                }
                else if (dma_au8RxBuf[3] == 0x90)
                {
                    if (adv_i2p == true)
                    {
                        adv_smooth = true;
                        FLASH_SaveSettings();
                        ADV_SetSmooth(adv_smooth);
                    }
                    c_state = 1;
                }
                else if (dma_au8RxBuf[3] == 0x91)
                {
                    if (adv_i2p == true)
                    {
                        adv_smooth = false;
                        FLASH_SaveSettings();
                        ADV_SetSmooth(adv_smooth);
                    }
                    c_state = 1;
                }
                else if (dma_au8RxBuf[3] == 0x30)
                {
                    adv_i2p = true;
                    FLASH_SaveSettings();
                    ADV_SetI2P(adv_i2p);
                    c_state = 1;
                    ADV_ResetStatus();
                }
                else if (dma_au8RxBuf[3] == 0x31)
                {
                    adv_i2p = false;
                    adv_smooth = false;
                    FLASH_SaveSettings();
                    ADV_SetSmooth(adv_smooth);
                    DDL_DelayMS(50);
                    ADV_SetI2P(adv_i2p);
                    c_state = 1;
                }
                else if (dma_au8RxBuf[3] == 0xa0)
                {
                    asw_02 = 1;
                    ASW_SetSwitches(asw_01, asw_02, asw_03, asw_04, 0);
                    FLASH_SaveSettings();
                    printf("Open Compatibility \n");
                    c_state = 1;
                }
                else if (dma_au8RxBuf[3] == 0xa1)
                {
                    asw_02 = 0;
                    ASW_SetSwitches(asw_01, asw_02, asw_03, asw_04, 0);
                    FLASH_SaveSettings();
                    printf("Close Compatibility \n");
                    c_state = 1;
                }
                else if ((dma_au8RxBuf[3] & 0xf0) == 0x40)
                {
                    AVsw   = 0;
                    asw_01 = 0;
                    asw_02 = dma_au8RxBuf[3] & 0x01;
                    asw_03 = 0;
                    asw_04 = 1;
                    ASW_SetSwitches(asw_01, asw_02, asw_03, asw_04, 0);
                    ASW_SetAVConnect(AVsw);
                    adv_sw       = false;
                    Input_signal = 1;
                    FLASH_SaveSettings();
                    ADV_Deinit();
                    printf("Compatibility %d\n", (dma_au8RxBuf[3] & 0x0f));
                    printf("RGBs\n");
                    c_state = 1;
                }
                else if ((dma_au8RxBuf[3] & 0xf0) == 0x50)
                {
                    AVsw   = 0;
                    asw_01 = 0;
                    asw_02 = dma_au8RxBuf[3] & 0x01;
                    asw_03 = 1;
                    asw_04 = 0;
                    ASW_SetSwitches(asw_01, asw_02, asw_03, asw_04, 0);
                    ASW_SetAVConnect(AVsw);
                    adv_sw       = false;
                    Input_signal = 2;
                    FLASH_SaveSettings();
                    ADV_Deinit();
                    printf("RGsB\n");
                    c_state = 1;
                }
                else if ((dma_au8RxBuf[3] & 0xf0) == 0x60)
                {
                    asw_01 = 1;
                    asw_02 = dma_au8RxBuf[3] & 0x0f;
                    asw_03 = 1;
                    asw_04 = 1;
                    ASW_SetSwitches(asw_01, asw_02, asw_03, asw_04, 0);
                    adv_sw       = false;
                    Input_signal = 3;
                    FLASH_SaveSettings();
                    ADV_Deinit();
                    printf("VGA\n");
                    c_state = 1;
                }
                else if (dma_au8RxBuf[3] == 0x70)
                {
                    asw_01 = 0;
                    asw_02 = 0;
                    asw_03 = 1;
                    asw_04 = 0;
                    ASW_SetSwitches(asw_01, asw_02, asw_03, asw_04, 0);
                    adv_sw       = false;
                    Input_signal = 4;
                    FLASH_SaveSettings();
                    ADV_Deinit();
                    printf("Ypbpr\n");
                    c_state = 1;
                }
                else if (dma_au8RxBuf[3] == 0x80)
                {
                    adv_ace = true;
                    FLASH_SaveSettings();
                    ADV_SetACE(adv_ace);
                    printf("AceOn\n");
                    c_state = 1;
                }
                else if (dma_au8RxBuf[3] == 0x81)
                {
                    adv_ace = false;
                    FLASH_SaveSettings();
                    ADV_SetACE(adv_ace);
                    printf("AceOff\n");
                    c_state = 1;
                }
            }
        }
        m_enRxFrameEnd = RESET;
    }
}
