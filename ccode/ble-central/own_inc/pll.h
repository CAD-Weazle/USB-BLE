// file    : pll.h
// author  : rb
// purpose : header file for pll.c
// date    : 190404
// last    : 190430
//

#ifndef _PLL_H_
#define _PLL_H_

// from UM: make sure VCO_Fin ranges from 1 to 2Mc, with 2Mc has lowest jitter
#define PLLM                                     16   // - VCO_Fin  = PLL_Fin/PLLM = HSI/PLLM = 16/16 = 1Mc
#define PLLN                                    168   // - VCO_Fout = VCO_Fin*PLLN = 1*168 = 168Mc 
#define PLLP                                      2   // - PLLCLK   = VCO_Fout/PLLP = 168/2 = 84Mc

#define HSICLK                           16000000UL   // HSI    = 16Mc
#define PLLCLK            (HSICLK*PLLN)/(PLLM*PLLP)   // PLLCLK = 84Mc
#define SYSCLK                               PLLCLK   // SYSCLK = 84Mc
#define HCLK                             (PLLCLK/1)   // HCLK   = 84Mc (max 168Mc)
#define APB1CLK                          (SYSCLK/2)   // APB1   = 42Mc (max 42Mc but Timer clocks SYSCLK/1 = 48Mc!!1!!)
#define APB2CLK                          (SYSCLK/1)   // APB2   = 84Mc (max 84Mc)

// -- prototypes
void init_pll (void);

void pll_dump (void);

#endif


