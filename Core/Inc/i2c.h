/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   This file contains all the function prototypes for
  *          the i2c.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN Private defines */
typedef enum
{
  I2C_POLLING,
  I2C_INTERRUPT,
  I2C_DMA
} i2c_operation_mode_t;

typedef enum
{
  I2C_STATE_OK      = 0x00U,
  I2C_STATE_ERROR   = 0x01U,
  I2C_STATE_BUSY    = 0x02U,
  I2C_STATE_TIMEOUT = 0x03U,
  I2C_STATE_BUSY_TX,
  I2C_STATE_BUSY_RX,
  I2C_STATE_START_ERROR,
  I2C_STATE_TX_ERROR,
  I2C_STATE_RX_ERROR,
  I2C_STATE_END_ERROR,
  I2C_STATE_INIT_DEV_ERROR,
  I2C_STATE_MEM_ADDR_ERROR,
  I2C_STATE_NULL_PTR,
  I2C_STATE_ZERO_LEN,
  I2C_STATE_MAX
} i2c_state_t;

#define DEFAULT_I2C_TIMEOUT 100

/* USER CODE END Private defines */

void MX_I2C1_Init(void);

/* USER CODE BEGIN Prototypes */

/**
  * @brief read/write from/to for LCD/OLED/instruction-type sensors: simple transmit without register addressing
  */
i2c_state_t i2c_bus_transmit(I2C_HandleTypeDef *hi2cX, uint8_t dev_addr, uint8_t *p_data, uint16_t len);
i2c_state_t i2c_bus_receive(I2C_HandleTypeDef *hi2cX, uint8_t dev_addr, uint8_t *p_data, uint16_t len);

/**
  * @brief read/write from/to for register-type sensors (EEPROM, ADXL, RTC)
  */
i2c_state_t i2c_bus_reg_write(I2C_HandleTypeDef *hi2cX, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t *p_data, uint16_t len);
i2c_state_t i2c_bus_reg_read(I2C_HandleTypeDef *hi2cX, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t *p_data, uint16_t len);

/**
  * @brief Check if I2C device is ready for communication
  */
i2c_state_t i2c_bus_is_ready(I2C_HandleTypeDef *hi2cX, uint8_t dev_addr, uint32_t trials);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */

