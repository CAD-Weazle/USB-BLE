// file    : clock.h
// author  : rb
// purpose : header file for clock.c
// date    : 171120
// last    : 171201
//

#ifndef _CLOCK_H_
#define _CLOCK_H_

// -- prototypes
void init_clock (void);

void rcc_set_systemclock (void);
void rcc_set_rtcclock    (void);

void rcc_start_hsi (void);
void rcc_stop_hsi  (void);

#endif
