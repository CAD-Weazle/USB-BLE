// file    : ticker.h
// author  : rb
// purpose : header file ticker.h
// date    : 170906
// last    : 200425

#ifndef __TICKER_H__
#define __TICKER_H__

#include "types.h"

// -- globals
volatile extern uchar csec_elapsed;
volatile extern uchar dsec_elapsed;
volatile extern uchar sec_elapsed;

// -- defines 
#define TIMER_1HZ       (SYSCLK/1    - 1)   // Timer clock = 1Hz
#define TIMER_1KHZ      (SYSCLK/1000 - 1)   // Timer clock = 1kHz
#define TIMER_10KHZ    (SYSCLK/10000 - 1)   // Timer clock = 10kHz
#define TIMER_100KHZ  (SYSCLK/100000 - 1)   // Timer clock = 100kHz
#define TIMER_1MHZ   (SYSCLK/1000000 - 1)   // Timer clock = 1MHz

// Timer 2 PWM 
#define PWM_PERIOD                   100    // PWM period = PWM_PERIOD/Timer_clock 
#define PWM_OFF                        0    // PWM off 

// -- prototypes
//void TIMER0_IRQHandler (void);
//void RIT_IRQHandler    (void);

void init_ticker (void);

void usleep (int16_t usecs);
void msleep (uint16_t msecs);

void pwm_set (uint8_t pwm);

#endif
