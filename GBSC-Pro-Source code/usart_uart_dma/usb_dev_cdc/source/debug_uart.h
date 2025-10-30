/**
 * debug_uart.h - Debug UART interface
 * on PH2(TX) / PC13(RX) [CM_USART3 @ 115200 baud]
 */

#ifndef DEBUG_UART_H
#define DEBUG_UART_H

#include <stdint.h>

#define DEBUG_UART                 CM_USART3
#define DEBUG_UART_FCG_ENABLE()    (FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART3, ENABLE))

#ifdef __cplusplus
extern "C" {
#endif

void DebugUart_Init(void);
void DebugUart_ClearErrors(void);
int32_t DebugUart_SendChar(char ch);
int32_t DebugUart_ReceiveChar(char *ch);

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_UART_H */