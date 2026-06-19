#ifndef _SG90_H_
#define _SG90_H_

#include "common.h"

typedef struct
{
  /* hardware related */
  TIM_HandleTypeDef *htimX;      // Timer handle (e.g., &htim3)
  uint32_t          channel;    // Timer channel (e.g., TIM_CHANNEL_4)
  /* angle status */
  uint16_t          current_angle;
} sg90_dev_t;

void sg90_init_dev(sg90_dev_t *dev, TIM_HandleTypeDef *htimX, uint32_t channel);

void sg90_set_angle(sg90_dev_t *dev, uint16_t angle);

#endif /* _SG90_H_ */