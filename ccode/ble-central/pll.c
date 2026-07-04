// file    : pll.c
// author  : rb
// purpose : STM32F4xx RCC module routines 
// date    : 190403
// last    : 190403
//

#include "includes.h"

void init_pll (void)
{ 
  // set 3 wait-states in FLASH & enable prefetching
  FLASH->ACR |= (FLASH_ACR_LATENCY_3WS |
                 FLASH_ACR_PRFTEN      |
                 FLASH_ACR_ICEN        |
                 FLASH_ACR_DCEN);

  // read back to make sure register is set
  (void) FLASH->ACR;      

  // turn on HSI oscillator (not really needed, but ok)
  RCC->CR |= RCC_CR_HSION;

  // wait for HSI stable
  while ((RCC->CR & RCC_CR_HSIRDY) == 0)
    ;                   

  // stop PLL before updating registers
  RCC->CR &= ~RCC_CR_PLLON;

  // wait for PLL stopped
  while ((RCC->CR & RCC_CR_PLLRDY) == 1)
    ;                   

  // clear main PLL, clock source bits & keep rest at reset value
  RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC |
                    RCC_PLLCFGR_PLLN   |
                    RCC_PLLCFGR_PLLM);

  // set main PLL clock rate & HSI as clock source
  RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI |
                  (PLLN << 6)            | 
				          (PLLM << 0); 

  // clear MCO1, bus dividers bits & keep rest at reset value
  RCC->CFGR &= ~(RCC_CFGR_HPRE  |
                 RCC_CFGR_PPRE1 |
                 RCC_CFGR_PPRE2);

  // set MCO1 & bus dividers
  RCC->CFGR |= ((0b00 << 30)         |           // SYSCLK on MCO2
                (0b11 << 21)         |           // PLLCLK on MCO1
                 RCC_CFGR_HPRE_DIV1  |           // SYSCLK not divided
                 RCC_CFGR_PPRE1_DIV2 |           // SYSCLK divided by 2 - APB1 max 42Mc
                 RCC_CFGR_PPRE2_DIV1);           // SYSCLK not divided  - APB2 max 84Mc

  // turn on PLL
  RCC->CR |= RCC_CR_PLLON;

  // wait for PLL locked
  while ((RCC->CR & RCC_CR_PLLRDY) == 0)
    ;                   

  // select PLL as system clock
  RCC->CFGR &= ~(RCC_CFGR_SW);
  RCC->CFGR |=  (RCC_CFGR_SW_PLL);

  // wait for clock switch ready
  while ((RCC->CFGR & RCC_CFGR_SWS_PLL) == 0) 
    ;        
}

// dump PLL constants
void pll_dump (void)
{
  printf ("clock settings:\n");
  printf ("PLLM                    : %ld\n", (uint32_t)PLLM);
  printf ("PLLN                    : %ld\n", (uint32_t)PLLN);
  printf ("PLLP                    : %ld\n", (uint32_t)PLLP);
  printf ("HSI                     : %ld\n", (uint32_t)HSICLK);
  printf ("PLLCLK                  : %ld\n", (uint32_t)PLLCLK);
  printf ("SYSCLK                  : %ld\n", (uint32_t)SYSCLK);
  printf ("HCLK                    : %ld\n", (uint32_t)HCLK);
  printf ("PCLK1 (APB1 Peripheral) : %ld\n", (uint32_t)(APB1CLK));
  printf ("PCLK1 (APB1 Timer)      : %ld\n", (uint32_t)(APB1CLK*2));
  printf ("PCLK2 (APB2 Peripheral) : %ld\n", (uint32_t)(APB2CLK));
  printf ("PCLK2 (APB2 Timer)      : %ld\n", (uint32_t)(APB2CLK));
}

