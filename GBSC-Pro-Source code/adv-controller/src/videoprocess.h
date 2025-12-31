/**
 *******************************************************************************
 * @file  videoprocess.h
 * @brief ADV7280/ADV7391 video processing control interface
 *        - ADV7280: Video decoder (CVBS/S-Video input)
 *        - ADV7391: Video encoder (Component/RGB output)
 *******************************************************************************
 */

#ifndef VIDEOPROCESS_H
#define VIDEOPROCESS_H

#include "i2c.h"
#include "main.h"

/*******************************************************************************
 * GPIO Pin Definitions - Power and Reset Control
 ******************************************************************************/

/* Output switch */
#define OUTPUT_SWITCH_PORT      GPIO_PORT_A
#define OUTPUT_SWITCH_PIN       GPIO_PIN_07

/* Power control */
#define POWER_DOWN_PORT         GPIO_PORT_B
#define GPIO_PIN_POWER_DOWN     GPIO_PIN_00     /* ADV power down control */

/* Input reset */
#define INPUT_RESET_PORT        GPIO_PORT_A
#define GPIO_PIN_INPUT_RESET    GPIO_PIN_03     /* ADV7280 reset */

/* Output enable */
#define OUTPUT_PORT             GPIO_PORT_B
#define GPIO_PIN_OUTPUT_EN      GPIO_PIN_01     /* ADV7391 output enable */

/*******************************************************************************
 * ADV7280/ADV7391 Control Functions
 ******************************************************************************/

/* Module init/deinit */
void ADV_Init(void);
void ADV_Deinit(void);
void ADV_Enable(uint8_t sw);

/* Video settings */
void ADV_SetInput(uint8_t input);
void ADV_SetI2P(uint8_t enable);
void ADV_SetSmooth(uint8_t smooth);
void ADV_SetBCSH(void);
void ADV_SetOutput(uint8_t output);
void ADV_SetACE(uint8_t ace);

/* Detection and monitoring */
void ADV_ReadKeyChange(void);
void ADV_DetectLoop(void);
void ADV_ResetStatus(void);
void ADV_Info(void);

/*******************************************************************************
 * Analog Switch Control Functions
 ******************************************************************************/

void ASW_SetSwitches(uint8_t sw1, uint8_t sw2, uint8_t sw3, uint8_t sw4, uint8_t state);
void ASW_SetSwitchesNot02(uint8_t sw1, uint8_t sw2, uint8_t sw3, uint8_t sw4, uint8_t state);
void ASW_SetAVConnect(uint8_t sw);

/*******************************************************************************
 * LED Control Functions
 ******************************************************************************/

void LED_SetSignal(uint8_t signal);

#endif /* VIDEOPROCESS_H */
