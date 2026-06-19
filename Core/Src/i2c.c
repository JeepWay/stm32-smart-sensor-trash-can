/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
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
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */
#include "common.h"
/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB8     ------> I2C1_SCL
    PB9     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = I2C_EEP_SCL_Pin|I2C_EEP_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_AFIO_REMAP_I2C1_ENABLE();

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB8     ------> I2C1_SCL
    PB9     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(I2C_EEP_SCL_GPIO_Port, I2C_EEP_SCL_Pin);

    HAL_GPIO_DeInit(I2C_EEP_SDA_GPIO_Port, I2C_EEP_SDA_Pin);

  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/**
  * @brief  for LCD/OLED/instruction-type sensors: simple transmit without register addressing
  * @param  hi2cX: pointer to I2C handle (e.g., &hi2c1)
  * @param  dev_addr: 8-bit I2C device address
  * @param  p_data: pointer to transmit data buffer
  * @param  len: length of data to transmit
  * @retval i2c_state_t (I2C_STATE_OK, I2C_STATE_ERROR, etc.)
  */
i2c_state_t i2c_bus_transmit(I2C_HandleTypeDef *hi2cX, uint8_t dev_addr, uint8_t *p_data, uint16_t len)
{
  HAL_StatusTypeDef res = HAL_I2C_Master_Transmit(hi2cX, dev_addr, p_data, len, DEFAULT_I2C_TIMEOUT);
  
  if (res != HAL_OK)
  {
    LOG_ERR("I2C Transmit Error: %d\n", res);
    if (res == HAL_BUSY)    return I2C_STATE_BUSY;
    if (res == HAL_TIMEOUT) return I2C_STATE_TIMEOUT;
    return I2C_STATE_ERROR;
  }
  return I2C_STATE_OK;
}

/**
  * @brief for LCD/OLED/instruction-type sensors: simple receive without register addressing
  * @param hi2cX: pointer to I2C handle (e.g., &hi2c1)
  * @param dev_addr: 8-bit I2C device address
  * @param p_data: pointer to receive data buffer
  * @param len: length of data to receive
  * @retval i2c_state_t (I2C_STATE_OK, I2C_STATE_ERROR, etc.)
  */
i2c_state_t i2c_bus_receive(I2C_HandleTypeDef *hi2cX, uint8_t dev_addr, uint8_t *p_data, uint16_t len)
{
  HAL_StatusTypeDef res = HAL_I2C_Master_Receive(hi2cX, dev_addr, p_data, len, DEFAULT_I2C_TIMEOUT);
  
  if (res != HAL_OK)
  {
    LOG_ERR("I2C Receive Error: %d\n", res);
    if (res == HAL_BUSY)    return I2C_STATE_BUSY;
    if (res == HAL_TIMEOUT) return I2C_STATE_TIMEOUT;
    return I2C_STATE_ERROR;
  }
  return I2C_STATE_OK;
}

/**
  * @brief for register-type sensors (EEPROM, ADXL, RTC): read from specific register address
  * @param hi2cX: pointer to I2C handle (e.g., &hi2c1)
  * @param dev_addr: 8-bit I2C device address
  * @param mem_addr: register address to read from
  * @param mem_addr_size: length of register address (16 bits or 8 bits)
  * @param p_data: pointer to read data buffer
  * @param len: length of data to read
  * @retval i2c_state_t (I2C_STATE_OK, I2C_STATE_ERROR, etc.)
  */
i2c_state_t i2c_bus_reg_read(I2C_HandleTypeDef *hi2cX, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t *p_data, uint16_t len)
{
  if ((mem_addr_size != I2C_MEMADD_SIZE_8BIT) && (mem_addr_size != I2C_MEMADD_SIZE_16BIT))
  {
    LOG_ERR("Invalid memory address size: %u\n", mem_addr_size);
    return I2C_STATE_MEM_ADDR_ERROR;
  }

  HAL_StatusTypeDef res = HAL_I2C_Mem_Read(hi2cX, dev_addr, mem_addr, mem_addr_size, p_data, len, DEFAULT_I2C_TIMEOUT) ;
  
  if (res != HAL_OK)
  {
    LOG_ERR("I2C Mem Read Error: %d\n", res);
    if (res == HAL_BUSY)    return I2C_STATE_BUSY;
    if (res == HAL_TIMEOUT) return I2C_STATE_TIMEOUT;
    return I2C_STATE_ERROR;
  }
  return I2C_STATE_OK;
}

/**
  * @brief for register-type sensors (EEPROM, ADXL, RTC): write to specific register address
  * @param hi2cX: pointer to I2C handle (e.g., &hi2c1)
  * @param dev_addr: 8-bit I2C device address
  * @param mem_addr: register address to write to 
  * @param mem_addr_size: length of register address (1 or 2 bytes)
  * @param p_data: pointer to write data buffer
  * @param len: length of data to write
  * @retval i2c_state_t (I2C_STATE_OK, I2C_STATE_ERROR, etc.)
  * 
  */
i2c_state_t i2c_bus_reg_write(I2C_HandleTypeDef *hi2cX, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t *p_data, uint16_t len)
{  
  if ((mem_addr_size != I2C_MEMADD_SIZE_8BIT) && (mem_addr_size != I2C_MEMADD_SIZE_16BIT))
  {
    LOG_ERR("Invalid memory address size: %u\n", mem_addr_size);
    return I2C_STATE_MEM_ADDR_ERROR;
  }
  
  HAL_StatusTypeDef res = HAL_I2C_Mem_Write(hi2cX, dev_addr, mem_addr, mem_addr_size, p_data, len, DEFAULT_I2C_TIMEOUT);
  
  if (res != HAL_OK)
  {
    LOG_ERR("I2C Mem Write Error: %d\n", res);
    if (res == HAL_BUSY)    return I2C_STATE_BUSY;
    if (res == HAL_TIMEOUT) return I2C_STATE_TIMEOUT;
    return I2C_STATE_ERROR;
  }
  return I2C_STATE_OK;
}

/** 
  * @brief Check if I2C device is ready by sending its address and checking for ACK
  * @param hi2cX: pointer to I2C handle (e.g., &hi2c1)
  * @param dev_addr: 8-bit I2C device address
  * @param trials: number of attempts to check for device readiness
  * @retval i2c_state_t (I2C_STATE_OK if device is ready, I2C_STATE_ERROR if not)
  */
i2c_state_t i2c_bus_is_ready(I2C_HandleTypeDef *hi2cX, uint8_t dev_addr, uint32_t trials)
{
  HAL_StatusTypeDef res = HAL_I2C_IsDeviceReady(hi2cX, dev_addr, trials, DEFAULT_I2C_TIMEOUT);
  
  if (res != HAL_OK)
  {
    LOG_ERR("I2C Device 0x%x Not Ready: %d\n", dev_addr, res);
    if (res == HAL_BUSY)    return I2C_STATE_BUSY;
    if (res == HAL_TIMEOUT) return I2C_STATE_TIMEOUT;
    return I2C_STATE_ERROR;
  }
  return I2C_STATE_OK;
}
/* USER CODE END 1 */

