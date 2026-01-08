/**
 *******************************************************************************
 * @file  i2c.h
 * @brief I2C driver for ADV7280/ADV7391 communication
 *******************************************************************************
 */

#ifndef I2C_H
#define I2C_H

#include "main.h"

/* I2C hardware configuration */
#define I2C_UNIT          (CM_I2C1)
#define I2C_FCG_USE       (FCG1_PERIPH_I2C1)
#define I2C_BAUDRATE      (200000UL)
#define I2C_CLK_DIVx      (I2C_CLK_DIV4)

/* I2C pin configuration - PA06(SCL) / PA07(SDA) */
#define I2C_SCL_PORT      (GPIO_PORT_A)
#define I2C_SCL_PIN       (GPIO_PIN_06)
#define I2C_SDA_PORT      (GPIO_PORT_A)
#define I2C_SDA_PIN       (GPIO_PIN_07)
#define I2C_GPIO_SCL_FUNC (GPIO_FUNC_49)
#define I2C_GPIO_SDA_FUNC (GPIO_FUNC_48)

/* Device addresses and timeout */
#define DEVICE_ADDR       (0x42)   /* ADV7280 I2C address */
#define TIMEOUT           (0x40000UL)

/* Function prototypes */
void    I2C_DriverInit(void);
int32_t I2C_Transmit(uint16_t addr, uint8_t const data[], uint32_t size, uint32_t timeout);
int32_t I2C_TransmitBatch(uint8_t const data[], uint32_t count, uint32_t timeout);
int32_t I2C_Receive(uint16_t addr, uint8_t tx[], uint8_t rx[], uint32_t size, uint32_t timeout);

#endif /* I2C_H */
