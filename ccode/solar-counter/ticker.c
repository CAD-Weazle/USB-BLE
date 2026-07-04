// file    : ticker.c
// author  : rb
// purpose : STM32L1xx timer routines
// date    : 170906
// last    : 200425
// note    : timer 6 usleep timer & signal handler for timer 7 wall clock overflow interrupt

#include "includes.h"

// globals
volatile uchar csec_elapsed = 0;
volatile uchar dsec_elapsed = 0;
volatile uchar  sec_elapsed = 0;

// Timer 7 overflow interrupt
void TIM7_IRQHandler (void)
{
  static uchar msec_cnt = 0;
  static uchar csec_cnt = 0;
  static uchar dsec_cnt = 0;

  // clear interrupt (note: first thing to do in the ISR!!!!!)
  TIM7->SR = 0;

  msec_cnt++;

  if (msec_cnt >= 10)
  {
    csec_elapsed = 1;

    msec_cnt = 0;
    csec_cnt++;
  }

  if (csec_cnt == 10)
  {
    dsec_elapsed = 1;

    csec_cnt = 0;
    dsec_cnt++;
  }

  if (dsec_cnt == 10)
  {
    sec_elapsed = 1;
    dsec_cnt = 0;
  }
}        

// setup Timers
void init_ticker (void)
{
  // enable clock for Timer 2, Timer 5, Timer 6 & Timer 7
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN |
                  RCC_APB1ENR_TIM5EN |
                  RCC_APB1ENR_TIM6EN |
                  RCC_APB1ENR_TIM7EN ;

  // init 16-bit Timer 2, PWM 
  TIM2->PSC    = TIMER_10KHZ;               // set Timer 2 clock frequency
  TIM2->ARR    = PWM_PERIOD;                // set period 
  TIM2->CCMR1 |= TIM_CCMR1_OC2M_2 |         // select PWM mode 1 on OC2 (OC2M = 0b110)
                 TIM_CCMR1_OC2M_1 |
                 TIM_CCMR1_OC2PE;           // enable preload register on OC2, channel is output
  TIM2->CCER  |= TIM_CCER_CC2E;             // select active high polarity on OC2 & enable output OC2
  TIM2->CCR2   = 0;                         // PWM off
  TIM2->EGR   |= TIM_EGR_UG;                // force update
  TIM2->CR1   |= TIM_CR1_CEN;               // enable counter, edge allign mode & direction upcounting

  // init 32-bit Timer 5, freerunning at 1.024 MHz <> needed?
  TIM5->PSC    = 0;                         // timer_clock = SYSCLK / 1 (== 1.024 Mc)
  TIM5->EGR    = TIM_EGR_UG;                // force update
  TIM5->CR1    = TIM_CR1_CEN;               // enable timer

  // init 16-bit Timer 6, freerunning at 1.024 MHz
  TIM6->PSC    = 0;                         // timer_clock = SYSCLK / 1 (== 1.024 Mc)
  TIM6->EGR    = TIM_EGR_UG;                // force update
  TIM6->CR1    = TIM_CR1_CEN;               // enable timer

  // init 16-bit Timer 7, generate interrupt every ms
  TIM7->PSC    = 0;                         // timer_clock = SYSCLK / 1 (== 1.024 Mc)
  TIM7->ARR    = 1049;                      // set period, hand tuned
  TIM7->EGR    = TIM_EGR_UG;                // force update
  TIM7->DIER   = TIM_DIER_UIE;              // enable interrupts, disable DMA
  TIM7->CR1    = TIM_CR1_CEN;               // enable timer
  NVIC_EnableIRQ (TIM7_IRQn);               // enable interrupt Timer 7
}

// kill time with 1.024 us resolution (@ 1.024 Mc SYSCLK)
void usleep (int16_t usecs)
{
  uint16_t start = TIM6->CNT + usecs;

  while ((int16_t)(TIM6->CNT - start) < 0)
    continue;
}

// kill time with approx. ms resolution
void msleep (uint16_t msecs)
{
  while (msecs--)
  //usleep (1024);                     // @ 1.024 Mc SYSCLK
    usleep (1030);                     // tuned with 1 sec delay 
}

// PWM routines
void pwm_set (uint8_t pwm)
{
  // sanity check
  if (pwm > 100)
    return;

  // calculate PWM hi-time from 0..100% input parameter
  TIM2->CCR2 = ((PWM_PERIOD * pwm) / 100);

  printf2 ("#set pwm to %d\n", pwm);
}








