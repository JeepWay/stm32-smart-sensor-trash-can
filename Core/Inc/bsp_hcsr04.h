#ifndef _BSP_HCSR04_H_
#define _BSP_HCSR04_H_

#include "common.h"

#ifndef HCSR04_TRIG_INTERVAL
#define HCSR04_TRIG_INTERVAL 100
#endif

typedef enum
{
  HCSR04_STATE_IDLE = 0,
  HCSR04_STATE_MEASURING,
  HCSR04_STATE_FINISHED,
  HCSR04_STATE_ERROR
} hcsr04_state_t;

/**
 * @brief HCSR04 configuration structure
 */
typedef struct
{
  /* hardware related */
  TIM_HandleTypeDef       *htimX;          // Timer handle (e.g., &htim3)
  uint32_t                tim_channel;     // Timer channel (e.g., TIM_CHANNEL_1)
  HAL_TIM_ActiveChannel   active_channel;  // Timer trigger channel (e.g., HAL_TIM_ ACTIVE_CHANNEL_1)
  GPIO_TypeDef            *trig_port;      // Trig Port (e.g., GPIOA)
  uint16_t                trig_pin;        // Trig Pin  (e.g., GPIO_PIN_5)

  /* state and counter */
  volatile hcsr04_state_t status;
  uint32_t                last_trigger_tick;
  volatile uint32_t       echo_start;       // capture rising edge time
  volatile uint32_t       echo_duration;    // echo pulse duration in microseconds
  volatile uint32_t       last_distance_mm; // unit: mm to avoid using float
  volatile bool           data_available;
} hcsr04_dev_t;

/* --------------- exported API ------------------ */
extern void hcsr04_init_dev(hcsr04_dev_t *dev, TIM_HandleTypeDef *htimX, uint32_t tim_channel, 
    HAL_TIM_ActiveChannel active_channel, GPIO_TypeDef *trig_port, uint16_t trig_pin);

extern void hcsr04_process(hcsr04_dev_t *dev);

extern bool hcsr04_is_data_ready(hcsr04_dev_t *dev);
extern uint32_t hcsr04_get_distance(hcsr04_dev_t *dev);

extern void hcsr04_timer_isr(hcsr04_dev_t *dev, TIM_HandleTypeDef *htim);

#endif /* _BSP_HCSR04_H_ */