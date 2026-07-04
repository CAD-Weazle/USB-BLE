// file    : dac.c
// author  : ao/rb
// purpose : DAC routines - STM32F4xx
// date    : 160312
// last    : 190910
//

#include "includes.h"

// init DAC
void init_dac (void)
{
  // enable DAC clock
  RCC->APB1ENR |= RCC_APB1Periph_DAC; 

  // set up DAC1
  DAC->DHR12R1 = 0;                    // clear 12-bit data register
//DAC->CR      = DAC_CR_EN1;           // enable DAC1

  // set up DAC2
  DAC->DHR12R2 = 0;                    // clear 12-bit data register
  DAC->CR      = DAC_CR_EN2;           // enable DAC2
//DAC->CR      = DAC_CR_BOFF2 |        // enable output buffer
//               DAC_CR_EN2;           // enable DAC2
}

// write data to 12-bit DAC1
void dac1_write (uint16_t dat)
{
  // clip data to 12-bit value
  if (dat > DAC_MAX)
    dat = DAC_MAX;

  // update DAC register
  DAC->DHR12R1 = (uint32_t)dat;
}

// write data to 12-bit DAC2
void dac2_write (uint16_t dat)
{
  // clip data to 12-bit value
  if (dat > DAC_MAX)
    dat = DAC_MAX;

  // update DAC register
  DAC->DHR12R2 = (uint32_t)dat;
}
