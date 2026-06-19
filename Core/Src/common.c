#include "common.h"

void DWT_Delay_Init(void)
{
  if (!(DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk))
  {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;                                // reset counter
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  }
}

void delay_us(uint32_t us)
{
  uint32_t start_tick = DWT->CYCCNT;
  // assume HCLK is 72MHz, thus 1us = 72 clock cycles
  uint32_t ticks_needed = us * (SystemCoreClock / 1000000U);
  while ((DWT->CYCCNT - start_tick) < ticks_needed);
}