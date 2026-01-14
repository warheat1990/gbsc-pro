/**
 *******************************************************************************
 * @file  uart_dma.c
 * @brief UART DMA communication with gbs-control (ESP8266)
 *        Uses USART4 on PB06(TX) / PB07(RX) @ 115200 baud
 *        Protocol: [0x41 0x44] [cmd] [data...] [0xFE] [checksum]
 *******************************************************************************
 */

#include "main.h"
#include "uart_dma.h"
#include "flash.h"

__IO en_flag_status_t m_enRxFrameEnd;
uint8_t dma_au8RxBuf[APP_FRAME_LEN_MAX];
static uint16_t m_u16RxLen;  /* Number of bytes received in buffer */

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
    /* Get remaining transfer count to calculate received bytes */
    uint16_t remaining = DMA_GetTransCount(RX_DMA_UNIT, RX_DMA_CH);
    uint16_t received = APP_FRAME_LEN_MAX - remaining;

    /* Store received length for processing */
    m_u16RxLen = received;

    printf("[RX %d] ", received);
    for (uint16_t i = 0; i < received && i < APP_FRAME_LEN_MAX; i++) {
        printf("%02X ", dma_au8RxBuf[i]);
    }
    printf("\n");

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

    /* Wait for ESP8266 bootloader garbage to finish (~100ms at 74880 baud) */
    DDL_DelayMS(150);

    /* Clear any pending errors and data from bootloader garbage */
    USART_ClearStatus(USART_UNIT, USART_FLAG_OVERRUN | USART_FLAG_FRAME_ERR | USART_FLAG_PARITY_ERR);
    (void)USART_ReadData(USART_UNIT);  /* Flush RX data register */

    /* Reset DMA buffer and state */
    memset(dma_au8RxBuf, 0, sizeof(dma_au8RxBuf));
    m_enRxFrameEnd = RESET;
    AOS_SW_Trigger();  /* Reset DMA to buffer start */

    /* Now enable RX - clean start after bootloader garbage */
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
        printf("[ADV] %s\n", video_modes[mode - 1].name);
        c_state = 1;
    }
    else
    {
        adv_tv  = 0x04;
        buff[1] = adv_tv;
        (void)I2C_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT);
        printf("[ADV] Err Default Auto\n");
        c_state = 2;
    }
}

/**
 * @brief  Process a single command from the buffer at given offset
 * @param  buf Pointer to command data
 * @param  len Available bytes in buffer
 * @return Command length if valid, 0 if invalid/incomplete
 */
static uint16_t UART_ProcessSingleCommand(uint8_t *buf, uint16_t len)
{
    uint8_t buff[2];

    /* Need at least 7 bytes for standard packet or header check */
    if (len < 7)
        return 0;

    /* Check header */
    if (buf[0] != 0x41 || buf[1] != 0x44)
        return 0;

    /* Handle Custom I2C command with variable length */
    if (buf[2] == 'C')
    {
        uint8_t count = buf[3];
        /* Max triplets limited by buffer size (4096-6)/3=1363, but count is uint8_t so max 255 */
        if (count > 0)
        {
            uint16_t data_len   = count * 3;
            uint16_t fe_pos     = 4 + data_len;
            uint16_t chksum_pos = fe_pos + 1;
            uint16_t cmd_len    = chksum_pos + 1;

            /* Check if we have enough bytes */
            if (len < cmd_len)
                return 0;

            /* Calculate checksum */
            uint8_t calc_sum = 0;
            for (uint16_t i = 0; i <= fe_pos; i++)
            {
                calc_sum += buf[i];
            }

            if (buf[fe_pos] == 0xFE && buf[chksum_pos] == calc_sum)
            {
                uint8_t *i2c_data = &buf[4];
                (void)I2C_TransmitBatch(i2c_data, count, TIMEOUT);
                printf("[ADV] Custom I2C: %d cmds\n", count);
                c_state = 1;
            }
            else
            {
                printf("[ADV] Custom I2C: chksum err\n");
            }
            return cmd_len;
        }
        return 0;
    }

    /* Standard fixed-length packet validation (7 bytes) */
    buff[1] = buf[5] + buf[4] + buf[3] + buf[2] + buf[1] + buf[0];
    if (buf[5] != 0xFE || buf[6] != buff[1])
        return 0;

    /* Process command */
    if (buf[2] == 'T')
    {
        adv_tv  = buf[3];
        FLASH_SaveSettings();
        buff[0] = VID_SEL_REG;
        buff[1] = adv_tv;
        (void)I2C_Transmit(DEVICE_ADDR, buff, 2, TIMEOUT);
        printf("[ADV] Tv 0x%02x\n", adv_tv);
        c_state = 1;
    }
    else if (buf[2] == 'N' && (buf[3] == 0x0a || buf[3] == 0x08 ||
                                buf[3] == 0xe3 || buf[3] == 0xe4 || buf[3] == 'D'))
    {
        if (buf[3] == 'D' && buf[4] == 'E')
        {
            uint8_t I2C_DEFAULT_BCSH[] = {
                0x42, 0x0E, 0x00,
                0x42, 0x0A, 0x00,
                0x42, 0x08, 0x80,
                0x42, 0xE3, 0x80,
                0x42, 0xE4, 0x80,
                0x42, 0x0B, 0x00,
            };
            Bright     = 0x00;
            Contrast   = 0x80;
            Saturation = 0x80;
            (void)I2C_TransmitBatch(I2C_DEFAULT_BCSH, sizeof(I2C_DEFAULT_BCSH) / 3, TIMEOUT);
            printf("[ADV] bcsh: default\n");
        }
        else
        {
            (void)I2C_Transmit(DEVICE_ADDR, &buf[3], 2, TIMEOUT);
            printf("[ADV] bcsh: 0x%02x\n", buf[4]);
            if (buf[3] == 0x0a)
                Bright = buf[4];
            else if (buf[3] == 0x08)
                Contrast = buf[4];
            else if (buf[3] == 0xe3 || buf[3] == 0xe4)
                Saturation = buf[4];
        }
        FLASH_SaveSettings();
        c_state = 1;
    }
    else if (buf[2] == 'S')
    {
        if (buf[3] == 0xfd)
        {
            adv_sw = true;
            FLASH_SaveSettings();
            ADV_Init();
            c_state = 1;
        }
        else if (buf[3] == 0x01)
        {
            adv_sw = false;
            FLASH_SaveSettings();
            ADV_Deinit();
            c_state = 1;
        }
        else if ((buf[3] & 0xf0) == 0x10)
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
            UART_SetVideoMode(buf[3] & 0x0f);
            ADV_Init();
            DDL_DelayMS(300);  /* Allow ADV to stabilize after init */
            Input_signal = 5;
            FLASH_SaveSettings();
            printf("[ADV] mode 0x%02x\n", buf[3]);
            c_state = 1;
        }
        else if ((buf[3] & 0xf0) == 0x20)
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
            UART_SetVideoMode(buf[3] & 0x0f);
            ADV_Init();
            DDL_DelayMS(300);  /* Allow ADV to stabilize after init */
            Input_signal = 6;
            FLASH_SaveSettings();
            printf("[ADV] mode 0x%02x\n", buf[3]);
            c_state = 1;
        }
        else if (buf[3] == 0x90)
        {
            if (adv_i2p == true)
            {
                adv_smooth = true;
                FLASH_SaveSettings();
                ADV_SetSmooth(adv_smooth);
            }
            else
            {
                printf("[ADV] Smooth skip (I2P off)\n");
            }
            c_state = 1;
        }
        else if (buf[3] == 0x91)
        {
            if (adv_i2p == true)
            {
                adv_smooth = false;
                FLASH_SaveSettings();
                ADV_SetSmooth(adv_smooth);
            }
            else
            {
                printf("[ADV] Smooth skip (I2P off)\n");
            }
            c_state = 1;
        }
        else if (buf[3] == 0x30)
        {
            adv_i2p = true;
            FLASH_SaveSettings();
            ADV_SetI2P(adv_i2p);
            c_state = 1;
            ADV_ResetStatus();
        }
        else if (buf[3] == 0x31)
        {
            adv_i2p = false;
            adv_smooth = false;
            FLASH_SaveSettings();
            ADV_SetSmooth(adv_smooth);
            ADV_SetI2P(adv_i2p);
            c_state = 1;
            ADV_ResetStatus();
        }
        else if (buf[3] == 0xa0)
        {
            asw_02 = 0;  /* ASW2=0 enables LM1881 sync stripper */
            ASW_SetSwitches(asw_01, asw_02, asw_03, asw_04, 0);
            FLASH_SaveSettings();
            printf("[ADV] Sync Stripper ON\n");
            c_state = 1;
        }
        else if (buf[3] == 0xa1)
        {
            asw_02 = 1;  /* ASW2=1 bypasses LM1881 sync stripper */
            ASW_SetSwitches(asw_01, asw_02, asw_03, asw_04, 0);
            FLASH_SaveSettings();
            printf("[ADV] Sync Stripper OFF\n");
            c_state = 1;
        }
        else if ((buf[3] & 0xf0) == 0x40)
        {
            AVsw   = 0;
            asw_01 = 0;
            asw_02 = buf[3] & 0x01;
            asw_03 = 0;
            asw_04 = 1;
            ASW_SetSwitches(asw_01, asw_02, asw_03, asw_04, 0);
            ASW_SetAVConnect(AVsw);
            adv_sw       = false;
            Input_signal = 1;
            FLASH_SaveSettings();
            ADV_Deinit();
            printf("[ADV] RGBs (SyncStrip=%d)\n", (buf[3] & 0x0f));
            c_state = 1;
        }
        else if ((buf[3] & 0xf0) == 0x50)
        {
            AVsw   = 0;
            asw_01 = 0;
            asw_02 = buf[3] & 0x01;
            asw_03 = 1;
            asw_04 = 0;
            ASW_SetSwitches(asw_01, asw_02, asw_03, asw_04, 0);
            ASW_SetAVConnect(AVsw);
            adv_sw       = false;
            Input_signal = 2;
            FLASH_SaveSettings();
            ADV_Deinit();
            printf("[ADV] RGsB\n");
            c_state = 1;
        }
        else if ((buf[3] & 0xf0) == 0x60)
        {
            asw_01 = 1;
            asw_02 = buf[3] & 0x0f;
            asw_03 = 1;
            asw_04 = 1;
            ASW_SetSwitches(asw_01, asw_02, asw_03, asw_04, 0);
            adv_sw       = false;
            Input_signal = 3;
            FLASH_SaveSettings();
            ADV_Deinit();
            printf("[ADV] VGA\n");
            c_state = 1;
        }
        else if (buf[3] == 0x70)
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
            printf("[ADV] Ypbpr\n");
            c_state = 1;
        }
        else if (buf[3] == 0x80)
        {
            adv_ace = true;
            FLASH_SaveSettings();
            ADV_SetACE(adv_ace);
            c_state = 1;
        }
        else if (buf[3] == 0x81)
        {
            adv_ace = false;
            FLASH_SaveSettings();
            ADV_SetACE(adv_ace);
            c_state = 1;
        }
        else if (buf[3] == 0x82)
        {
            ADV_SetACELumaGain(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0x83)
        {
            ADV_SetACEChromaGain(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0x84)
        {
            ADV_SetACEChromaMax(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0x85)
        {
            ADV_SetACEGammaGain(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0x86)
        {
            ADV_SetACEResponseSpeed(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0x87)
        {
            ADV_SetACEDefaults();
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0xB0)
        {
            ADV_SetFilterYShaping(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0xB1)
        {
            ADV_SetFilterCShaping(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0xB2)
        {
            ADV_SetFilterWYShaping(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0xB3)
        {
            ADV_SetFilterWYShapingOvr(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0xB4)
        {
            ADV_SetFilterCombNTSC(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0xB5)
        {
            ADV_SetFilterCombPAL(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0xB7)
        {
            ADV_SetVideoFilterDefaults();
            FLASH_SaveSettings();
            c_state = 1;
        }
        /* Comb Control commands (0xB8-0xBD) */
        else if (buf[3] == 0xB8)
        {
            ADV_SetCombLumaModeNTSC(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0xB9)
        {
            ADV_SetCombChromaModeNTSC(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0xBA)
        {
            ADV_SetCombChromaTapsNTSC(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0xBB)
        {
            ADV_SetCombLumaModePAL(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0xBC)
        {
            ADV_SetCombChromaModePAL(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
        else if (buf[3] == 0xBD)
        {
            ADV_SetCombChromaTapsPAL(buf[4]);
            FLASH_SaveSettings();
            c_state = 1;
        }
    }

    return 7;  /* Standard command length */
}

/**
 * @brief  Find next valid header in buffer
 * @param  buf Buffer to search
 * @param  len Buffer length
 * @return Offset to header, or len if not found
 */
static uint16_t UART_FindHeader(uint8_t *buf, uint16_t len)
{
    for (uint16_t i = 0; i < len - 1; i++)
    {
        if (buf[i] == 0x41 && buf[i + 1] == 0x44)
            return i;
    }
    return len;
}

void UART_ProcessCommand(void)
{
    if (SET != m_enRxFrameEnd)
        return;

    /* Copy buffer locally to avoid race condition with DMA */
    uint8_t local_buf[APP_FRAME_LEN_MAX];
    uint16_t local_len = m_u16RxLen;
    memcpy(local_buf, dma_au8RxBuf, local_len);

    /* Reset state immediately to allow new reception */
    m_enRxFrameEnd = RESET;
    m_u16RxLen = 0;

    uint16_t pos = 0;
    uint16_t remaining = local_len;
    uint16_t cmd_count = 0;

    /* Process all commands in buffer */
    while (remaining >= 7)
    {
        /* Find header if not at start */
        if (local_buf[pos] != 0x41 || local_buf[pos + 1] != 0x44)
        {
            uint16_t skip = UART_FindHeader(&local_buf[pos], remaining);
            if (skip >= remaining)
                break;  /* No more headers found */
            pos += skip;
            remaining -= skip;
            continue;
        }

        /* Try to process command at current position */
        uint16_t cmd_len = UART_ProcessSingleCommand(&local_buf[pos], remaining);
        if (cmd_len == 0)
        {
            /* Invalid command, skip header and look for next */
            pos += 2;
            remaining -= 2;
            continue;
        }

        /* Command processed successfully */
        pos += cmd_len;
        remaining -= cmd_len;
        cmd_count++;
    }

    if (cmd_count > 1)
    {
        printf("[UART] Processed %d commands\n", cmd_count);
    }
}
