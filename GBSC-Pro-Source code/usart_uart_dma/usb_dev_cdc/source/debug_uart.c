/**
 * debug_uart.c - Debug UART interface
 * on PH2(TX) / PC13(RX) [CM_USART3 @ 115200 baud]
 */

#include "main.h"
#include "debug_uart.h"
#include <stdio.h>

/**
 * @brief Initialize debug UART (CM_USART3)
 */
void DebugUart_Init(void)
{
    stc_usart_uart_init_t stcUartInit;

    // Enable DEBUG USART clock
    DEBUG_UART_FCG_ENABLE();

    // Configure USART RX/TX pin
    // HC32F460JEUA-QFN48TR Datasheet - Pages 33 and 38
    GPIO_SetFunc(GPIO_PORT_H, GPIO_PIN_02, GPIO_FUNC_32); // PH2 = USART3_TX
    GPIO_SetFunc(GPIO_PORT_C, GPIO_PIN_13, GPIO_FUNC_33); // PC13 = USART3_RX
    
    // Initialize UART structure
    (void)USART_UART_StructInit(&stcUartInit);
    stcUartInit.u32Baudrate = 115200UL;
    stcUartInit.u32OverSampleBit = USART_OVER_SAMPLE_8BIT;

    // Initialize debug USART
    (void)USART_UART_Init(DEBUG_UART, &stcUartInit, NULL);
    
    // Enable RX and TX
    USART_FuncCmd(DEBUG_UART, (USART_TX | USART_RX), ENABLE);
}

/**
 * @brief Clear UART error flags that can block RX
 */
void DebugUart_ClearErrors(void)
{
    USART_ClearStatus(DEBUG_UART, USART_FLAG_OVERRUN | USART_FLAG_FRAME_ERR | USART_FLAG_PARITY_ERR);
}

/**
 * @brief Send single character (for printf redirect)
 * @param ch Character to send
 * @return Character sent
 */
int32_t DebugUart_SendChar(char ch)
{
    while (RESET == USART_GetStatus(DEBUG_UART, USART_FLAG_TX_EMPTY)) {
        // Timeout protection (optional)
    }
    USART_WriteData(DEBUG_UART, (uint16_t)ch);
    return ch;
}

/**
 * @brief Receive single character (non-blocking)
 * @param ch Pointer to store received character
 * @return 0 if character received, -1 if no data
 */
int32_t DebugUart_ReceiveChar(char *ch)
{
    if (SET == USART_GetStatus(DEBUG_UART, USART_FLAG_RX_FULL)) {
        *ch = (char)USART_ReadData(DEBUG_UART);
        return 0;
    }
    return -1;
}

/**
 * @brief Printf redirect for ARM Compiler 6
 */
__attribute__((weak)) int fputc(int ch, FILE *f)
{
    (void)f;
    return DebugUart_SendChar((char)ch);
}
