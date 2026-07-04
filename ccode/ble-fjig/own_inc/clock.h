// file    : clock.h
// author  : rb
// purpose : header file for clock.c
// date    : 171120
// last    : 220220
//

#ifndef _CLOCK_H_
#define _CLOCK_H_

// -- defines
#define Mc         (1000000UL)              // 1 Mc

#define HSI_CLK    (8*Mc)                   // HSI Clock = 8 Mc
//#define MSI_CLK  (1024000ul)              // MSI Clock = 1.024 Mc
#define MSI_CLK    (2048000ul)              // MSI Clock = 2.097 Mc  <> debug
#define SYSCLK     (MSI_CLK)                // System Clock is HSE Clock

#define APB1DIV    (1UL)                    // APB1 clock divider
#define APB1CLK    (SYSCLK/APB1DIV)         // APB1 clock

#define APB2DIV    (1UL)                    // APB2 clock divider
#define APB2CLK    (SYSCLK/APB2DIV)         // APB2 clock

// -- prototypes
void init_clock (void);

void rcc_set_systemclock (void);
void rcc_set_rtcclock    (void);

void rcc_start_hsi (void);
void rcc_stop_hsi  (void);

#endif
