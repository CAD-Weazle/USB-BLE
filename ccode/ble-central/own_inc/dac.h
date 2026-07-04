// file    : dac.h
// author  : rb
// purpose : header file for dac.c
// date    : 160312
// last    : 190512
//

#ifndef _DAC_H_
#define _DAC_H_

// -- defines
#define DAC_MAX   0x0fff

// -- prototypes
void init_dac (void);

void dac1_write (uint16_t dat);
void dac2_write (uint16_t dat);

#endif
