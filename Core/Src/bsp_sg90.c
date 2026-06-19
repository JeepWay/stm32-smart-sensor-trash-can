#include "bsp_sg90.h"

/**
  * @brief init SG90 device instance
  * @param dev: pointer to sg90_dev_t instance
  * @param htimX: pointer to the hardware timer handle (e.g., &htim1)
  * @param channel: timer channel (e.g., TIM_CHANNEL_1)
  * @retval None
  */
void sg90_init_dev(sg90_dev_t *dev, TIM_HandleTypeDef *htimX, uint32_t channel)
{
  if (!dev || !htimX)
  {
    LOG_ERR("TIM handle or dev pointer is NULL!\n");
    return;
  }

  dev->htimX = htimX;
  dev->channel = channel;
  dev->current_angle = 0;
  
  HAL_TIM_PWM_Start(dev->htimX, dev->channel);
  sg90_set_angle(dev, 0);
}

/**
  * @brief set angle of SG90 servo
  * @param dev: pointer to sg90_dev_t instance
  * @param angle: desired angle (0-180 degrees)
  * @retval None
  */
void sg90_set_angle(sg90_dev_t *dev, uint16_t angle)
{
  if (!dev || !dev->htimX)
  {
    LOG_ERR("TIM handle or dev pointer is NULL!\n");
    return;
  }

  if (angle > 180)
  {
    angle = 180;
  }

  /**
    * 0 degree   -> 0.5 ms -> 500 us -> Pulse = 500
    * 180 degree -> 2.5 ms -> 2500 us -> Pulse = 2500
    * formula：Pulse = 500 + (angle * 2000 / 180)
    */
  uint16_t pulse = 500 + (angle * 2000 / 180);
  __HAL_TIM_SET_COMPARE(dev->htimX, dev->channel, pulse);
  
  dev->current_angle = angle;
}