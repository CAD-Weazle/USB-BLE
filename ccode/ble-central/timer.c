// file    : timer.c
// author  : rb
// purpose : ticker & PWM Timers
// date    : 170906
// last    : 190812

#include "includes.h"

// globals
volatile uint8_t csec_elapsed = 0;
volatile uint8_t dsec_elapsed = 0;
volatile uint8_t sec_elapsed  = 0;

// ARM core SysTick ISR
void SysTick_Handler (void)
{
  static int csec_cnt, dsec_cnt, sec_cnt;

  if (++csec_cnt >= 10)
  {
    csec_cnt     = 0;
    csec_elapsed = 1;

    if (++dsec_cnt >= 10)
    {
      dsec_cnt     = 0;
      dsec_elapsed = 1;

      if (++sec_cnt >= 10)
      {
        sec_cnt     = 0;
        sec_elapsed = 1;
      }
    }
  } 
}


// init Timers
void init_timer (void) 
{
  // enable Timer clock - Timer clock = APB1CLK*2 = 84Mc (not 42Mc!1!!1!)
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN |
                  RCC_APB1ENR_TIM4EN |
                  RCC_APB1ENR_TIM12EN;

  // enable Timer clock - Timer clock = APB2CLK*1 = 84Mc 
  RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;


  // init 32-bit Timer 2 to free running
  TIM2->PSC     = TIMER_1MHZ;               // set Timer 2 clock frequency
  TIM2->EGR     = TIM_EGR_UG;               // force update
  TIM2->CR1     = TIM_CR1_CEN;              // enable timer 2 


  // init 16-bit Timer 9
  TIM9->PSC    = TIMER_1MHZ;                // set Timer 9 clock frequency
  TIM9->ARR    = PWM_PERIOD;                // set period (== 100 us / 10 kHz)
 
  // - set OC2 for PWM output PUMP1
  TIM9->CCMR1 |= TIM_CCMR1_OC2M_2 |         // select PWM mode 1 on OC2 (OC2M = 0b110)
                 TIM_CCMR1_OC2M_1 |
                 TIM_CCMR1_OC2PE;           // enable preload register on OC2, channel is output
  TIM9->CCER  |= TIM_CCER_CC2E;             // select active high polarity on OC2 & enable output OC2
  TIM9->CCR2   = PWM_OFF;                   // set to 0% PWM

  // - set OC1 for PWM output PUMP2
  TIM9->CCMR1 |= TIM_CCMR1_OC1M_2 |         // select PWM mode 1 on OC1 (OC1M = 0b110)
                 TIM_CCMR1_OC1M_1 |
                 TIM_CCMR1_OC1PE;           // enable preload register on OC1, channel is output
  TIM9->CCER  |= TIM_CCER_CC1E;             // select active high polarity on OC1 & enable output OC1
  TIM9->CCR1   = PWM_OFF;                   // set to 0% PWM

  // - enable Timer 9
  TIM9->EGR   |= TIM_EGR_UG;                // force update
  TIM9->CR1   |= TIM_CR1_CEN;               // enable counter, edge allign mode & direction upcounting


  // init 16-bit Timer 4
  TIM4->PSC    = TIMER_1MHZ;                // set Timer 4 clock frequency
  TIM4->ARR    = PWM_PERIOD;                // set period (== 100 us / 10 kHz)

  // - set OC4 for PWM output PUMP3
  TIM4->CCMR2 |= TIM_CCMR2_OC4M_2 |         // select PWM mode 1 on OC4 (OC4M = 0b110)
                 TIM_CCMR2_OC4M_1 |
                 TIM_CCMR2_OC4PE;           // enable preload register on OC4, channel is output
  TIM4->CCER  |= TIM_CCER_CC4E;             // select active high polarity on OC4 & enable output OC4
  TIM4->EGR   |= TIM_EGR_UG;                // force update
  TIM4->CCR4   = PWM_OFF;                   // set to 0% PWM
  TIM4->CR1   |= TIM_CR1_CEN;               // enable counter, edge allign mode & direction upcounting


  // init System Timer (from 'core_cm4.h') -- 1.000 ms tick after tweaking
  SysTick_Config ((HCLK / SYSTICK_FREQ) + SYSTICK_TWEAK);       
}

// kill time with us resolution
void usleep (int16_t usecs)
{
  uint32_t start = TIM2->CNT + usecs;

  while ((int32_t) (TIM2->CNT - start) < 0)
    continue;
}

// kill time with ms resolution
void msleep (uint16_t msecs)
{
  while (msecs--)
    usleep (1000);              
}

// PWM routines
void pwm_set (uint16_t pump, uint8_t pwm)
{
  uint16_t hi_time;

  if (pwm > PWM_MAX)
    return;

  // calculate PWM hi-time from 0..100% pump speed
  if (pwm)
    hi_time = ((PWM_PERIOD * pwm) / 100);
  else
    hi_time = 0;

  switch (pump)
  {
    // PUMP 1 - Timer 9, OC2
    case PWM_PUMP1:
    {
      TIM9->CCR2 = hi_time;
      break;
    }

    // PUMP 2 - Timer 9, OC1
    case PWM_PUMP2:
    {
      TIM9->CCR1 = hi_time;
      break;
    }

    // PUMP 3 - Timer 4, OC4
    case PWM_PUMP3:
    {
      TIM4->CCR4 = hi_time;
      break;
    }

    default:
     break;
  }
}







