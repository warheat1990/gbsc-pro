/**
 *******************************************************************************
 * @file  uart_dma.h
 * @brief UART DMA communication with gbs-control (ESP8266)
 *        Uses USART4 on PB06(TX) / PB07(RX) @ 115200 baud
 *******************************************************************************
 */

#ifndef UART_DMA_H
#define UART_DMA_H

#include "main.h"

#define RX_DMA_UNIT                (CM_DMA1)
#define RX_DMA_CH                  (DMA_CH0)
#define RX_DMA_FCG_ENABLE()        (FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_DMA1, ENABLE))
#define RX_DMA_TRIG_SEL            (AOS_DMA1_0)
#define RX_DMA_TRIG_EVT_SRC        (EVT_SRC_USART4_RI)
#define RX_DMA_RECONF_TRIG_SEL     (AOS_DMA_RC)
#define RX_DMA_RECONF_TRIG_EVT_SRC (EVT_SRC_AOS_STRG)
#define RX_DMA_TC_INT              (DMA_INT_TC_CH0)
#define RX_DMA_TC_FLAG             (DMA_FLAG_TC_CH0)
#define RX_DMA_TC_IRQn             (INT000_IRQn)
#define RX_DMA_TC_INT_SRC          (INT_SRC_DMA1_TC0)

#define TX_DMA_UNIT         (CM_DMA2)
#define TX_DMA_CH           (DMA_CH0)
#define TX_DMA_FCG_ENABLE() (FCG_Fcg0PeriphClockCmd(FCG0_PERIPH_DMA2, ENABLE))
#define TX_DMA_TRIG_SEL     (AOS_DMA2_0)
#define TX_DMA_TRIG_EVT_SRC (EVT_SRC_USART4_TI)
#define TX_DMA_TC_INT       (DMA_INT_TC_CH0)
#define TX_DMA_TC_FLAG      (DMA_FLAG_TC_CH0)
#define TX_DMA_TC_IRQn      (INT001_IRQn)
#define TX_DMA_TC_INT_SRC   (INT_SRC_DMA2_TC0)

#define USART_RX_PORT      (GPIO_PORT_B)
#define USART_RX_PIN       (GPIO_PIN_07)
#define USART_RX_GPIO_FUNC (GPIO_FUNC_37)

#define USART_TX_PORT      (GPIO_PORT_B)
#define USART_TX_PIN       (GPIO_PIN_06)
#define USART_TX_GPIO_FUNC (GPIO_FUNC_36)

#define USART_UNIT         (CM_USART4)
#define USART_FCG_ENABLE() (FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART4, ENABLE))

#define USART_BAUDRATE     (115200UL)
// RX idle timeout: 250 bits → ~6.7ms idle time to detect end of packet
#define USART_TIMEOUT_BITS (250U)

#define USART_TX_CPLT_IRQn    (INT002_IRQn)
#define USART_TX_CPLT_INT_SRC (INT_SRC_USART4_TCI)

#define USART_RX_ERR_IRQn    (INT003_IRQn)
#define USART_RX_ERR_INT_SRC (INT_SRC_USART4_EI)

#define USART_RX_TIMEOUT_IRQn    (INT004_IRQn)
#define USART_RX_TIMEOUT_INT_SRC (INT_SRC_USART4_RTO)

#define DMATMR0_UNIT         (CM_TMR0_2)
#define DMATMR0_CH           (TMR0_CH_B)
#define DMATMR0_FCG_ENABLE() (FCG_Fcg2PeriphClockCmd(FCG2_PERIPH_TMR0_2, ENABLE))

#define APP_FRAME_LEN_MAX (4096U)

extern __IO en_flag_status_t m_enRxFrameEnd;
extern uint8_t dma_au8RxBuf[APP_FRAME_LEN_MAX];

void    TMR0_Config(uint16_t u16TimeoutBits);
int32_t DMA_Config(void);
void    USART_TxComplete_IrqCallback(void);
void    USART_RxError_IrqCallback(void);
void    USART_RxTimeout_IrqCallback(void);
void    UART_DMA_Init(void);
void    UART_ProcessCommand(void);

#endif
