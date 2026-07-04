// file    : timer.c
// author  : rb
// purpose : ticker & PWM Timers
// date    : 170906
// last    : 220214

#include "includes.h"

// globals
volatile uint8_t csec_elapsed;
volatile uint8_t dsec_elapsed;
volatile uint8_t sec_elapsed;

// ARM core SysTick ISR
void SysTick_Handler (void)
{
  static int csec_cnt, dsec_cnt, sec_cnt;
 
  if (++csec_cnt >= 10)
  {
    csec_cnt = 0;
    csec_elapsed = 1;
  
    if (++dsec_cnt >= 10)
    {
      dsec_cnt = 0;
      dsec_elapsed = 1;
     
      if (++sec_cnt >= 10)
      {
        sec_cnt = 0;
        sec_elapsed = 1;
      }
    }
  } 
}

// init Timers
void init_timer (void) 
{
  // enable Timer clock - Timer clock = APB1CLK*2 = 96Mc
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

  // init 32-bit Timer 2 to free running
  TIM2->PSC     = TIMER_1MHZ;               // set Timer 2 clock frequency
  TIM2->EGR     = TIM_EGR_UG;               // force update
  TIM2->CR1     = TIM_CR1_CEN;              // enable timer 2 

  // init System Timer (from 'core_cm4.h') -- 1.000 ms tick after tweaking
//SysTick_Config ((HCLK / SYSTICK_FREQ) + SYSTICK_TWEAK);       
  SysTick_Config (SYSCLK / 1000);

  // set interrupt priority
  NVIC_SetPriority (SysTick_IRQn, 1); 
}

// kill time with 1us resolution
void usleep (int32_t usecs)
{
  uint32_t start = TIM2->CNT + usecs;
  
  while ((int32_t) (TIM2->CNT - start) < 0)
    continue;
}

// kill time with approx. ms resolution
void msleep (uint32_t msecs)
{
  while (msecs--)
    usleep (1000);
}








