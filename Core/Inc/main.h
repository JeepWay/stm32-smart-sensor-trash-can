/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "common.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
/**
  * can lib
  */
typedef enum
{
  LID_CLOSED = 0, // can lib is closed
  LID_OPENING,    // can lib is opening, waiting for stable reading
  LID_CLOSING     // can lib is closing, waiting for delay to ensure it's fully closed
} lid_state_t;

#ifndef LID_CLOSE_ANGLE
#define LID_CLOSE_ANGLE 0
#endif

#ifndef LID_OPEN_ANGLE
#define LID_OPEN_ANGLE 90
#endif

#ifndef LID_HOLD_TIME
#define LID_HOLD_TIME 3000
#endif

#ifndef LID_CLOSING_TIME
#define LID_CLOSING_TIME 1500
#endif
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PWM_SG90_Pin GPIO_PIN_11
#define PWM_SG90_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_12
#define LED1_GPIO_Port GPIOA
#define Hcsr04_Trig_Pin GPIO_PIN_3
#define Hcsr04_Trig_GPIO_Port GPIOB
#define Hcsr04_Echo_Pin GPIO_PIN_4
#define Hcsr04_Echo_GPIO_Port GPIOB
#define I2C_EEP_SCL_Pin GPIO_PIN_8
#define I2C_EEP_SCL_GPIO_Port GPIOB
#define I2C_EEP_SDA_Pin GPIO_PIN_9
#define I2C_EEP_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
