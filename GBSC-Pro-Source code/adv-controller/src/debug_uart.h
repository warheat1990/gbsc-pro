/**
 *******************************************************************************
 * @file  debug_uart.h
 * @brief Debug UART interface on PH2(TX) / PC13(RX)
 *        Uses CM_USART3 @ 115200 baud for printf output and CLI input
 *******************************************************************************
 */

#ifndef DEBUG_UART_H
#define DEBUG_UART_H

#include <stdint.h>

/* Hardware configuration */
#define DEBUG_UART              CM_USART3
#define DEBUG_UART_BAUDRATE     115200UL
#define DEBUG_UART_FCG_ENABLE() FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART3, ENABLE)

/* Function prototypes */
void    DebugUart_Init(void);
void    DebugUart_ClearErrors(void);
int32_t DebugUart_SendChar(char ch);
int32_t DebugUart_ReceiveChar(char *ch);

#endif /* DEBUG_UART_H */
