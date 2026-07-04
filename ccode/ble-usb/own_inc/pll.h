// file    : pll.h
// author  : rb
// purpose : header file for pll.c
// date    : 171120
// last    : 220214
//

#ifndef _PLL_H_
#define _PLL_H_

// -- defines
// VCO freq between 100 and 432
// PLL in between 1MHz and 2MHz (2 is recommended)
// HSE is 16 MHz -> PLLM=4
// VCO = 192MHz * Fin * (PLLN/PLLM) = Fin * (PLLN/4) => PLLN = 96 
// PLL output = 96 MHz => PLLN = 2
// USB is 48 MHz => PLLQ = 4

#define PLLN                  96
#define PLLM                   8
#define PLLP                   0            // code for /2
#define PLLQ                   4 

#define Mc         (1000000UL)              // 1 Mc

#define HSICLK     (16*Mc)                  // HSI clock = 16 Mc
#define MSICLK     (1024000UL)              // MSI clock = 1.024 Mc
//#define PLL_CLK    ((HSI_CLK*3) / 2)      // PLL clock = 24 Mc

//#define SYSCLK     (PLL_CLK)              // system clock
#define SYSCLK     (96*Mc)

#define APB1DIV    (2UL)                    // APB1 clock divider
//#define APB1CLK    (SYSCLK/APB1DIV)         // APB1 clock
#define APB1CLK    (48*Mc)

#define APB2DIV    (1UL)                    // APB2 clock divider
//#define APB2CLK    (SYSCLK/APB2DIV)         // APB2 clock
#define APB2CLK    (96*Mc)
#define AHBCLK     (96*Mc)
#define TMR1CLK    (96*Mc)
#define TMR2CLK    (96*Mc)

////// from UM: make sure VCO_Fin ranges from 1 to 2Mc, with 2Mc has lowest jitter
////#define PLLM            16                  // - VCO_Fin = PLL_Fin / PLLM = HSI / PLLM = 16/16 = 1Mc
////#define PLLN           168                  // - VCO_Fout = VCO_Fin × PLLN = 1 x 168 = 168Mc 
////#define PLLP             2                  // - PLLCLK = VCO_Fout/PLLP = 168 / 2 = 84Mc
////
////#define HSICLK    16000000UL                // HSI    = 16Mc
////#define PLLCLK    (HSICLK*PLLN)/(PLLM*PLLP) // PLLCLK = 84Mc
////#define SYSCLK    PLLCLK                    // SYSCLK = 84Mc
////#define HCLK      (PLLCLK/1)                // HCLK   = 84Mc (max 168Mc)
////#define APB1CLK   (SYSCLK/2)                // APB1   = 42Mc (max 42Mc but Timer clocks SYSCLK/1 = 48Mc!!1!!)
////#define APB2CLK   (SYSCLK/1)                // APB2   = 84Mc (max 84Mc)


// -- prototypes
void init_pll (void);

void pll_dump (void);

#endif



