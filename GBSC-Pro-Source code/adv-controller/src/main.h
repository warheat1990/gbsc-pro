/**
 *******************************************************************************
 * @file  main.h
 * @brief ADV Controller - Main header with global definitions
 *******************************************************************************
 */

#ifndef MAIN_H
#define MAIN_H

#include "hc32_ll.h"
#include "ev_hc32f460_lqfp100_v2_bsp.h"
#include "uart_dma.h"
#include "i2c.h"
#include "videoprocess.h"
#include "string.h"
#include "stdbool.h"

#define GPIO_PORT_ASW GPIO_PORT_B

#define GPIO_PIN_ASW1 GPIO_PIN_15
#define GPIO_PIN_ASW2 GPIO_PIN_14
#define GPIO_PIN_ASW3 GPIO_PIN_13
#define GPIO_PIN_ASW4 GPIO_PIN_12

#define ASW1_On() GPIO_SetPins(GPIO_PORT_ASW, GPIO_PIN_ASW1)
#define ASW2_On() GPIO_SetPins(GPIO_PORT_ASW, GPIO_PIN_ASW2)
#define ASW3_On() GPIO_SetPins(GPIO_PORT_ASW, GPIO_PIN_ASW3)
#define ASW4_On() GPIO_SetPins(GPIO_PORT_ASW, GPIO_PIN_ASW4)

#define ASW1_Off() GPIO_ResetPins(GPIO_PORT_ASW, GPIO_PIN_ASW1)
#define ASW2_Off() GPIO_ResetPins(GPIO_PORT_ASW, GPIO_PIN_ASW2)
#define ASW3_Off() GPIO_ResetPins(GPIO_PORT_ASW, GPIO_PIN_ASW3)
#define ASW4_Off() GPIO_ResetPins(GPIO_PORT_ASW, GPIO_PIN_ASW4)

#define AV_Connecte_Off() GPIO_SetPins(GPIO_PORT_A, GPIO_PIN_08)
#define AV_Connecte_On()  GPIO_ResetPins(GPIO_PORT_A, GPIO_PIN_08)

#define RES_CHANGED     true
#define VID_SEL_REG     0x02 // Video Format Selection Register
#define AUTO_DETECT_REG 0x07 // Automatic Detection Register

#define VID_SEL_PAL   0x80
#define VID_SEL_NTSC  0x50
#define VID_SEL_SECAM 0xE0
#define VID_SEL_AUTO  0x00

#define AD_PAL_EN   0x01
#define AD_NTSC_EN  0x02
#define AD_SECAM_EN 0x04

#define LED_ERR_RED   0x80
#define LED_ERR_GREEN 0x40
#define LED_ERR_BLUE  0x20
#define LED_OK        0x10

#define LED_TIME 35
#define Boundary 10

#define AV_INPUT false
#define SV_INPUT true

// Flash storage definitions
#define USER                1
#define FLASH_LEAF_ADDR(x)  (uint32_t)(0x7a000U + (0x2000U * (x)))

extern uint8_t led_state;
extern uint8_t led_sw;
extern uint8_t adv_input;
extern uint8_t adv_i2p;
extern uint8_t adv_smooth;
extern uint8_t adv_ace;
extern uint8_t adv_sw;
extern uint8_t adv_tv;
extern uint8_t err_flag;
extern uint8_t c_state;
extern bool    asw_01, asw_02, asw_03, asw_04;
extern bool    AVsw;
extern uint8_t Input_signal;
extern uint8_t Bright;
extern uint8_t Contrast;
extern uint8_t Saturation;

#endif