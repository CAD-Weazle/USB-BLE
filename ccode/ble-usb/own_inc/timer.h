// file    : timer.h
// author  : rb
// purpose : header file timer.c
// date    : 170906
// last    : 220214

#ifndef __TIMER_H__
#define __TIMER_H__

// -- globals
extern volatile uint8_t csec_elapsed;
extern volatile uint8_t dsec_elapsed;
extern volatile uint8_t sec_elapsed;

// -- defines 
#define TIMERCLK                        (APB1CLK*2)
#define TIMER_1HZ          (TIMERCLK/1         - 1)
#define TIMER_1KHZ         (TIMERCLK/1000      - 1)
#define TIMER_1MHZ         (TIMERCLK/1000000UL - 1)

// ARM core SysTick defines for 1 ms ticker
#define SYSTICK_FREQ                           1000   // number of int's / sec
#define SYSTICK_TWEAK                             0   // tweak value to remove RC oscilator error

// -- prototypes
void init_timer (void);

void usleep (int32_t usecs);
void msleep (uint32_t msecs);

#endif

