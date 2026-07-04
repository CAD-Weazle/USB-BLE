// file    : pll.c
// author  : rb
// purpose : STM32F4xx RCC module routines 
// date    : 190403
// last    : 220214
//

#include "includes.h"

#define PLLN  96
#define PLLM  8
#define PLLP  0         // code for /2
#define PLLQ  4 

void init_pll (void)
{
  // set MCO1 and bus dividers
  RCC->CFGR = (0b110 << RCC_CFGR_MCO1PRE_Pos) |       // MCO1 divided by 4 -> 24Mc on scope
              (0b11 << RCC_CFGR_MCO1_Pos)     |       // SYSCLK on MCO1
              RCC_CFGR_PPRE2_DIV1             |       // APB2 clock divided by 1
              RCC_CFGR_PPRE1_DIV2;                    // APB1 clock divided by 2 -> 48Mc

  // enable HSE and HSI
  RCC->CR |= RCC_CR_HSEON | RCC_CR_HSION;

  // & wait for HSE stable
  while (!(RCC->CR & RCC_CR_HSERDY))
    continue;

  // configure PLL 
  RCC->PLLCFGR = RCC_PLLCFGR_PLLSRC | 
                 (PLLQ << 24)       | 
                 (PLLP << 16)       | 
                 (PLLN << 6)        | 
                 (PLLM << 0);

  // enanle PLL
  RCC->CR |= RCC_CR_PLLON;

  // & wait for PLL stable
  while( !(RCC->CR & RCC_CR_PLLRDY) )
    continue;

  // set flash wait states and enable caches
  FLASH->ACR = FLASH_ACR_LATENCY_3WS | FLASH_ACR_DCEN | FLASH_ACR_ICEN;

  // select PLL as system clock
  RCC->CFGR |= RCC_CFGR_SW_1; 
}

// dump PLL constants
void pll_dump (void)
{
  printf ("clock settings:\n");
  printf ("PLLM                    : %ld\n", (uint32_t)PLLM);
  printf ("PLLN                    : %ld\n", (uint32_t)PLLN);
  printf ("PLLP                    : %ld\n", (uint32_t)PLLP);
  printf ("HSI                     : %ld\n", (uint32_t)HSICLK);
//printf ("PLLCLK                  : %ld\n", (uint32_t)PLLCLK);
  printf ("SYSCLK                  : %ld\n", (uint32_t)SYSCLK);
//printf ("HCLK                    : %ld\n", (uint32_t)HCLK);
  printf ("PCLK1 (APB1 Peripheral) : %ld\n", (uint32_t)(APB1CLK));
  printf ("PCLK1 (APB1 Timer)      : %ld\n", (uint32_t)(APB1CLK*2));
  printf ("PCLK2 (APB2 Peripheral) : %ld\n", (uint32_t)(APB2CLK));
  printf ("PCLK2 (APB2 Timer)      : %ld\n", (uint32_t)(APB2CLK));
}

