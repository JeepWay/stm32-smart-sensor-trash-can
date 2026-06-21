/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#if (ENABLE_UART_DEBUG == 1)
#include "debug.h"
#endif
#include "bsp_hcsr04.h"
#include "bsp_sg90.h"
#include "bsp_eeprom.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
hcsr04_dev_t hcsr04_1;
sg90_dev_t sg90;
eeprom_dev_t eeprom_08;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  hcsr04_timer_isr(&hcsr04_1, htim);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  static led_state_t led1_state = LED_OFF;
  static uint32_t last_led1_toggle_tick = 0;
  UNUSED(last_led1_toggle_tick);
  UNUSED(led1_state);

  static uint32_t dist = 0;

  static lid_state_t lid_state = LID_CLOSED;
  static uint32_t lid_open_tick = 0;

  // ===== Contextual adaptive variable =====
  static uint16_t current_hold_time = LID_HOLD_TIME; 
  static uint16_t saved_hold_time = LID_HOLD_TIME;   // save in eeprom
  static uint16_t normal_use_count = 0;
  static uint32_t last_close_tick = 0;              
  static bool need_cool_down_check = false;          // turn TRUE after lid fully closed
  #define EEPROM_HOLD_TIME_ADDR   0x0000
  #define COOL_DOWN_TIMEOUT       15000   // timeout for check nobody is using
  #define TIME_INCREMENT          1000    // for current_hold_time
  #define MAX_HOLD_TIME           20000
  #define MIN_HOLD_TIME           1000
  #define DECREASE_THRESHOLD      3
  // ========================================
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  // ===== init device =====
  DWT_Delay_Init();
  LOG_INFO("System Clock: %lu\n", HAL_RCC_GetHCLKFreq());
  hcsr04_init_dev(&hcsr04_1, &htim3, TIM_CHANNEL_1, 
    HAL_TIM_ACTIVE_CHANNEL_1, Hcsr04_Trig_GPIO_Port, Hcsr04_Trig_Pin);
  sg90_init_dev(&sg90, &htim1, TIM_CHANNEL_4);
  eeprom_init_dev(&eeprom_08, &hi2c1, EEP_MODEL_AT24C08, 0);
  // =======================
  // ==== read use data in eeprom ====
  LOG_INFO("Reading configuration from EEPROM...\n");
  if (eeprom_read(&eeprom_08, EEPROM_HOLD_TIME_ADDR, (uint8_t *)&saved_hold_time, sizeof(saved_hold_time)) == I2C_STATE_OK)
  {
    if ((saved_hold_time >= MIN_HOLD_TIME) && (saved_hold_time <= MAX_HOLD_TIME))
    {
      current_hold_time = saved_hold_time;
      LOG_INFO("Restored user setting from EEPROM: %lu ms\n", saved_hold_time);
    }
    else
    {
      current_hold_time = LID_HOLD_TIME;
      saved_hold_time = LID_HOLD_TIME;
      eeprom_write(&eeprom_08, EEPROM_HOLD_TIME_ADDR, (uint8_t *)&saved_hold_time, sizeof(saved_hold_time));
      LOG_INFO("Invalid EEPROM data. Initialized with default: %d ms\n", LID_HOLD_TIME);
    }
  }
  else
  {
    LOG_ERR("Please check eeprom state!\n");
  }
  // =================================
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
#if (ENABLE_UART_DEBUG == 1)
    debug_command_handler();
#endif

    // update HCSR04 distance data
    hcsr04_process(&hcsr04_1);

    if (hcsr04_is_data_ready(&hcsr04_1))
    {
      dist = hcsr04_get_distance(&hcsr04_1);
      uint32_t current_tick = HAL_GetTick();
      // LOG_DEBUG("Dist: %lu.%lu cm\n", dist / 10, dist % 10);
      switch (lid_state)
      {
        case LID_CLOSED:
        {
          // update `saved_hold_time` if nobody uses after [last_close_tick + COOL_DOWN_TIMEOUT] ms
          if (need_cool_down_check && (current_tick - last_close_tick > COOL_DOWN_TIMEOUT))
          {
            need_cool_down_check = false;
            if (current_hold_time > saved_hold_time)
            {
              // continuous used
              saved_hold_time = current_hold_time;
              eeprom_write(&eeprom_08, EEPROM_HOLD_TIME_ADDR, (uint8_t *)&saved_hold_time, sizeof(saved_hold_time));
              normal_use_count = 0;
              LOG_INFO("Habit recorded! Saving extended time %lu ms to EEPROM.\n", saved_hold_time);
            }
            else
            {
              // normal used
              normal_use_count++;
              if (normal_use_count >= DECREASE_THRESHOLD && current_hold_time > LID_HOLD_TIME)
              {
                current_hold_time -= 100;

                if (current_hold_time < LID_HOLD_TIME)
                  current_hold_time = LID_HOLD_TIME;

                saved_hold_time = current_hold_time;
                eeprom_write(&eeprom_08, EEPROM_HOLD_TIME_ADDR, (uint8_t *)&saved_hold_time, sizeof(saved_hold_time));
                normal_use_count = 0;
                LOG_INFO("System cooled down. Gradually reducing hold time to: %lu ms\n", saved_hold_time);
              }
            }
          }
          // open lib
          if (dist < 100 && dist > 20)
          {
            // check if need to update `current_hold_time`
            if (last_close_tick != 0)
            {
              // continuous used, update `current_hold_time` if elapsed time < 5s
              if ((current_tick - last_close_tick) < 5000)
              {
                current_hold_time += TIME_INCREMENT;

                if (current_hold_time > MAX_HOLD_TIME)
                {
                  current_hold_time = MAX_HOLD_TIME;
                }
                
                normal_use_count = 0;
                LOG_INFO("Continuous use! Dynamic hold time: %lu ms\n", current_hold_time);
              }
            }
            sg90_set_angle(&sg90, (uint16_t)LID_OPEN_ANGLE);
            lid_open_tick = HAL_GetTick();      // record tick of opening lib
            lid_state = LID_OPENING;
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
            LOG_INFO("Object detected! Opening lid...\n");
          }
          break;
        }
        case LID_OPENING:
        {
          static uint32_t _detection_count = 0;
          if (dist < 100 && dist > 20)
          {
            _detection_count++;
            if (_detection_count >= 8)
            {
              lid_open_tick = HAL_GetTick();  // reset counter
              LOG_INFO("Keeping lid open...\n");
            }
          }
          else
          {
            _detection_count = 0;
          }
          
          if ((HAL_GetTick() - lid_open_tick >= current_hold_time) && (_detection_count == 0))
          {
            sg90_set_angle(&sg90, (uint16_t)LID_CLOSE_ANGLE);
            lid_open_tick = HAL_GetTick();
            lid_state = LID_CLOSING;
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
            LOG_INFO("Time up! Closing lid...\n");
          }
          break;
        }
        case LID_CLOSING:
        {
          if (lid_state == LID_CLOSING)
          {
            if (HAL_GetTick() - lid_open_tick >= LID_CLOSING_TIME)
            {
              lid_state = LID_CLOSED;
              last_close_tick = HAL_GetTick();
              need_cool_down_check = true;     // 蓋子關了，開啟冷卻倒數計時
              LOG_INFO("Lid fully closed. Ready for next trigger.\n");
            }
          }
          break;
        }
        default:
          break;
      }
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* TIM3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART1_IRQn, 0, 2);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
