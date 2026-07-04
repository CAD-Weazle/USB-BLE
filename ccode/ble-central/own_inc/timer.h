// file    : timer.h
// author  : rb
// purpose : header file for timer.h
// date    : 170906
// last    : 191012

#ifndef __TIMER_H__
#define __TIMER_H__

// -- globals
volatile extern uint8_t csec_elapsed;
volatile extern uint8_t dsec_elapsed;
volatile extern uint8_t sec_elapsed;

// -- defines 
#define TIMERCLK                        (APB1CLK*2)
#define TIMER_1HZ          (TIMERCLK/1         - 1)
#define TIMER_1KHZ         (TIMERCLK/1000      - 1)
#define TIMER_1MHZ         (TIMERCLK/1000000UL - 1)

// ARM core SysTick defines for 1 ms ticker
#define SYSTICK_FREQ                           1000   // number of int's / sec
#define SYSTICK_TWEAK                             0   // tweak value to remove RC oscilator error

// for PWM pump control
#define PWM_PERIOD                             9999   // PWM period (== 100 us / 100 Hz)
#define PWM_OFF                                   0   // pumps off
#define PWM_MIN                  (PWM_PERIOD - 100)   // <> ??? 
#define PWM_MAX                  (PWM_PERIOD -  50)   // <> ??? 

#define PWM_PUMP1                                 0     
#define PWM_PUMP2                                 1
#define PWM_PUMP3                                 2

// -- prototypes
void SysTick_Handler (void);

void init_timer (void);

void usleep (int16_t usecs);
void msleep (uint16_t msecs);

void pwm_set (uint16_t pump, uint8_t pwm);

#endif

