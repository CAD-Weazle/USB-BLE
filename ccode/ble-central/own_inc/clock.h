// file    : clock.h
// author  : rb
// purpose : header file for clock.c
// date    : 171120
// last    : 181031
//

#ifndef _CLOCK_H_
#define _CLOCK_H_

// -- defines
#define Mc         (1000000UL)              // 1 Mc

#define HSI_CLK    (8*Mc)                   // HSI Clock = 8 Mc
#define SYSCLK     (HSI_CLK)                // System Clock is HSE Clock

#define APB1DIV    (1UL)                    // APB1 clock divider
#define APB1CLK    (SYSCLK/APB1DIV)         // APB1 clock

#define APB2DIV    (1UL)                    // APB2 clock divider
#define APB2CLK    (SYSCLK/APB2DIV)         // APB2 clock


// -- prototypes
void init_clock (void);

void rcc_set_systemclock (void);

void rcc_start_lsi (void);
void rcc_stop_lsi  (void);

void rcc_start_hsi (void);
void rcc_stop_hsi  (void);

void rcc_start_hse (void);

void rcc_start_pll (void);
void rcc_stop_pll  (void);

#endif
