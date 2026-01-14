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
void ADV_SetACE(uint8_t ace);

/* BCSH (Brightness/Contrast/Saturation/Hue) parameter variables */
extern uint8_t Bright;
extern uint8_t Contrast;
extern uint8_t Saturation;
extern uint8_t Hue;

/* ACE (Adaptive Contrast Enhancement) parameter variables */
extern uint8_t AceLumaGain;      /* 0-31, default 13 (0x0D) */
extern uint8_t AceChromaGain;    /* 0-15, default 8 */
extern uint8_t AceChromaMax;     /* 0-15, default 8 */
extern uint8_t AceGammaGain;     /* 0-15, default 8 */
extern uint8_t AceResponseSpeed; /* 0-15, default 15 */

/* ACE (Adaptive Contrast Enhancement) parameter controls */
void ADV_SetACELumaGain(uint8_t gain);       /* 0-31, default 13 (0x0D) */
void ADV_SetACEChromaGain(uint8_t gain);     /* 0-15, default 8 */
void ADV_SetACEChromaMax(uint8_t max);       /* 0-15, default 8 */
void ADV_SetACEGammaGain(uint8_t gain);      /* 0-15, default 8 */
void ADV_SetACEResponseSpeed(uint8_t speed); /* 0-15, default 15 */
void ADV_SetACEParams(void);                 /* Apply all ACE parameters */
void ADV_SetACEDefaults(void);               /* Reset ACE parameters to defaults */

/* Video Filter parameter variables (ADV7280 registers 0x17, 0x18, 0x19) */
extern uint8_t FilterYShaping;      /* YSFM[4:0] for CVBS (0-31, default 0=Auto Wide) */
extern uint8_t FilterCShaping;      /* CSFM[2:0] for CVBS (0-7, default 0=Auto 1.5MHz) */
extern uint8_t FilterWYShaping;     /* WYSFM[4:0] for S-Video/YPrPb (0-31, default 0=Auto) */
extern uint8_t FilterWYShapingOvr;  /* WYSFMOVR (0=Auto, 1=Manual) */
extern uint8_t FilterCombNTSC;      /* NSFSEL[1:0] (0-3, default 0=Narrow) */
extern uint8_t FilterCombPAL;       /* PSFSEL[1:0] (0-3, default 0=Narrow) */

/* Video Filter parameter controls */
void ADV_SetFilterYShaping(uint8_t filter);     /* 0-31, default 0 (Auto Wide) */
void ADV_SetFilterCShaping(uint8_t filter);     /* 0-7, default 0 (Auto 1.5MHz) */
void ADV_SetFilterWYShaping(uint8_t filter);    /* 0-31, default 0 (Auto) */
void ADV_SetFilterWYShapingOvr(uint8_t ovr);    /* 0=Auto, 1=Manual */
void ADV_SetFilterCombNTSC(uint8_t bw);         /* 0-3, default 0 (Narrow) */
void ADV_SetFilterCombPAL(uint8_t bw);          /* 0-3, default 0 (Narrow) */

/* Comb Control parameter variables (ADV7280 registers 0x38 NTSC, 0x39 PAL) */
extern uint8_t CombLumaModeNTSC;    /* YCMN[2:0] (0,4-7, default 0=Adaptive) */
extern uint8_t CombChromaModeNTSC;  /* CCMN[2:0] (0,4-7, default 0=Adaptive) */
extern uint8_t CombChromaTapsNTSC;  /* CTAPSN[1:0] (0-3, default 2=5→3 lines) */
extern uint8_t CombLumaModePAL;     /* YCMP[2:0] (0,4-7, default 0=Adaptive) */
extern uint8_t CombChromaModePAL;   /* CCMP[2:0] (0,4-7, default 0=Adaptive) */
extern uint8_t CombChromaTapsPAL;   /* CTAPSP[1:0] (0-3, default 3=5→4 lines) */

/* Comb Control parameter controls */
void ADV_SetCombLumaModeNTSC(uint8_t mode);     /* 0=Adaptive, 4=Notch, 5-7=Fixed */
void ADV_SetCombChromaModeNTSC(uint8_t mode);   /* 0=Adaptive, 4=Off, 5-7=Fixed */
void ADV_SetCombChromaTapsNTSC(uint8_t taps);   /* 0-3, default 2 (5→3 lines) */
void ADV_SetCombLumaModePAL(uint8_t mode);      /* 0=Adaptive, 4=Notch, 5-7=Fixed */
void ADV_SetCombChromaModePAL(uint8_t mode);    /* 0=Adaptive, 4=Off, 5-7=Fixed */
void ADV_SetCombChromaTapsPAL(uint8_t taps);    /* 0-3, default 3 (5→4 lines) */

/* Unified video filter functions */
void ADV_SetVideoFilters(void);                 /* Apply all video filters (shaping + comb) */
void ADV_SetVideoFilterDefaults(void);          /* Reset all video filters to defaults */

/* Detection and monitoring */
void ADV_DetectLoop(void);
void ADV_ResetStatus(void);

/*******************************************************************************
 * Analog Switch Control Functions
 ******************************************************************************/

void ASW_SetSwitches(uint8_t sw1, uint8_t sw2, uint8_t sw3, uint8_t sw4, uint8_t state);
void ASW_SetAVConnect(uint8_t sw);

/*******************************************************************************
 * LED Control Functions
 ******************************************************************************/

void LED_SetSignal(uint8_t signal);

#endif /* VIDEOPROCESS_H */
