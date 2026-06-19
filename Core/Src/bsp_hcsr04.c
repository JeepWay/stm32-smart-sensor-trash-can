#include "bsp_hcsr04.h"
#include <stdint.h>

static void _hcsr04_trigger(hcsr04_dev_t *dev);

/**
  * @brief  init HCSR04 device instance with hardware binding and state initialization
  * @param  dev: pointer to hcsr04_dev_t instance
  * @param  htimX: pointer to timer handle (e.g., &htim3)
  * @param  tim_channel: timer channel (e.g., TIM_CHANNEL_1)
  * @param  active_channel: timer active channel for interrupt (e.g., HAL_TIM_ACTIVE_CHANNEL_1)
  * @param  trig_port: GPIO port for trigger pin (e.g., GPIOA)
  * @param  trig_pin: GPIO pin for trigger (e.g., GPIO_PIN_5)
  * @retval None
  */
void hcsr04_init_dev(hcsr04_dev_t *dev, TIM_HandleTypeDef *htimX, uint32_t tim_channel, HAL_TIM_ActiveChannel active_channel, GPIO_TypeDef *trig_port, uint16_t trig_pin)
{
  if (!dev || !htimX)
  {
    LOG_ERR("TIM handle or dev pointer is NULL!\n");
    return;
  }
  
  dev->htimX = htimX;
  dev->tim_channel = tim_channel;
  dev->active_channel = active_channel;
  dev->trig_port = trig_port;
  dev->trig_pin = trig_pin;

  dev->status = HCSR04_STATE_IDLE;
  dev->last_trigger_tick = 0;
  dev->echo_start = 0;
  dev->echo_duration = 0;
  dev->last_distance_mm = 0;
  dev->data_available = false;

  HAL_TIM_Base_Start(dev->htimX);
	HAL_TIM_IC_Stop_IT(dev->htimX, dev->tim_channel);	 // stop input capture interrupt until first trigger
}

/**
  * @brief  send trigger signal and start IC interrupt capture
  * @param  dev: pointer to hcsr04_dev_t instance
  * @retval None
  */
static void _hcsr04_trigger(hcsr04_dev_t *dev)
{
  if (dev->status == HCSR04_STATE_MEASURING)
    return;

  // reset before trigger
  HAL_TIM_IC_Stop_IT(dev->htimX, dev->tim_channel);
  __HAL_TIM_SET_CAPTUREPOLARITY(dev->htimX, dev->tim_channel, TIM_INPUTCHANNELPOLARITY_RISING);

  dev->status = HCSR04_STATE_MEASURING;
  
  // send trigger pulse
  HAL_GPIO_WritePin(dev->trig_port, dev->trig_pin, GPIO_PIN_SET);
  delay_us(10);   // ≥10μs
  HAL_GPIO_WritePin(dev->trig_port, dev->trig_pin, GPIO_PIN_RESET);
  
  // turn on rising edge detection
  HAL_TIM_IC_Start_IT(dev->htimX, dev->tim_channel);
}

/**
  * @brief Regularly trigger HC-SR04 and update distance when measurement is finished
  * @param dev: pointer to hcsr04_dev_t instance
  * @retval None
  */
void hcsr04_process(hcsr04_dev_t *dev)
{
  if (!dev->htimX || !dev)
  {
    LOG_ERR("TIM handle or dev pointer is NULL!\n");
    return;
  }
  
  // reset if timeout
  if ((dev->status == HCSR04_STATE_MEASURING) && ((HAL_GetTick() - dev->last_trigger_tick) > 25))
  {
    HAL_TIM_IC_Stop_IT(dev->htimX, dev->tim_channel);
    __HAL_TIM_SET_CAPTUREPOLARITY(dev->htimX, dev->tim_channel, TIM_INPUTCHANNELPOLARITY_RISING);
    dev->status = HCSR04_STATE_IDLE;
    LOG_WARN("HCSR04 Timeout! Resetting state...\n");
  }

  // trigger every HCSR04_TRIG_INTERVAL ms when HCSR04 idle
  uint32_t current_tick = HAL_GetTick();
  if (current_tick - dev->last_trigger_tick >= HCSR04_TRIG_INTERVAL)
  {
    if (dev->status == HCSR04_STATE_IDLE)
    {
      dev->last_trigger_tick = current_tick;
      _hcsr04_trigger(dev);
    }
  }

  if (dev->status == HCSR04_STATE_FINISHED)
  {
    // 1 cm = 10 mm
    dev->last_distance_mm = (dev->echo_duration * 10) / 58;
    dev->data_available = true;
    dev->status = HCSR04_STATE_IDLE;
  }
}

/**
  * @brief  Check if new distance data is available
  * @param  dev: pointer to hcsr04_dev_t instance
  * @retval true if data is ready, false otherwise 
  */
bool hcsr04_is_data_ready(hcsr04_dev_t *dev)
{
  return dev ? dev->data_available : false;
}

/**
  * @brief  Get the latest distance measurement and clear the data ready flag
  * @param  dev: pointer to hcsr04_dev_t instance
  * @retval Distance in cm, or 0 if dev is NULL
  */
uint32_t hcsr04_get_distance(hcsr04_dev_t *dev)
{
  if (!dev)
    return 0;

  dev->data_available = false;
  return dev->last_distance_mm;
}

/**
  * @brief used in HAL_TIM_IC_CaptureCallback, called when an input capture event occurs
  * @param  dev: pointer to hcsr04_dev_t instance
  * @param  htim: pointer to timer handle that triggered the interrupt
  * @retval None
  */
void hcsr04_timer_isr(hcsr04_dev_t *dev, TIM_HandleTypeDef *htim)
{
  if (!dev)
    return;

  if ((htim->Instance == dev->htimX->Instance))
  {
    if (dev->status != HCSR04_STATE_MEASURING)
      return;

    if ((htim->Instance->CCER & (TIM_CCER_CC1P << (dev->tim_channel * 4))) == 0)
    {    
      // capture rising edge 
      dev->echo_start = HAL_TIM_ReadCapturedValue(htim, dev->tim_channel);
      __HAL_TIM_SET_CAPTUREPOLARITY(htim, dev->tim_channel, TIM_INPUTCHANNELPOLARITY_FALLING);
    }
    else
    {
      // capture falling edge
      uint32_t echo_end = HAL_TIM_ReadCapturedValue(htim, dev->tim_channel);
      HAL_TIM_IC_Stop_IT(htim, dev->tim_channel);
      __HAL_TIM_SET_CAPTUREPOLARITY(dev->htimX, dev->tim_channel, TIM_INPUTCHANNELPOLARITY_RISING);

      // calcute duration time, considering overflow
      if (echo_end >= dev->echo_start)
      {
        dev->echo_duration = echo_end - dev->echo_start;
      }
      else
      {
        dev->echo_duration = (UINT16_MAX - dev->echo_start) + echo_end;
      }
      dev->status = HCSR04_STATE_FINISHED;
    }
  }
}
