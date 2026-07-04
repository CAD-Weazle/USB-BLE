// file    : clock.c
// author  : rb
// purpose : STM32L1xx RCC module routines 
// date    : 171125
// last    : 200313
//

#include "includes.h"

// define RTC clock source (use only one)
#define USE_LSI_FOR_RTC   0                            // set to 1 when LSI is used for RTC clock
#define USE_LSE_FOR_RTC   ~USE_LSI_FOR_RTC             // set to 1 when LSE is used for RTC clock

// init all clocks
void init_clock (void)
{
  rcc_start_hsi ();
  rcc_set_systemclock ();
//rcc_set_rtcclock    ();
}

// set system clock
void rcc_set_systemclock (void)
{
  // set MCO and bus dividers
  RCC->CFGR = RCC_CFGR_MCOPRE_DIV1   |                 // MCO divided by 1
              RCC_CFGR_MCOSEL_SYSCLK |                 // SYSCLK on MCO         
            //RCC_CFGR_MCOSEL_HSI    |                 // HSI on MCO
              RCC_CFGR_PPRE2_DIV1    |                 // APB2 clock divided by 1
              RCC_CFGR_PPRE1_DIV1    |                 // APB1 clock divided by 1
              RCC_CFGR_SW_MSI;                         // MSI used for SYSCLK 

  // set MSI clock speed (note: MSI is default clock, no enabling/verifying needed)
  RCC->ICSCR &= ~((0b111) << RCC_ICSCR_MSIRANGE_Pos);  // clear all bits first 
//RCC->ICSCR |= RCC_ICSCR_MSIRANGE_4;                  // select clock speed = 1.024Mc
  RCC->ICSCR |= RCC_ICSCR_MSIRANGE_5;                  // select clock speed = 2.048Mc <> debug
}

// set RTC clock
void rcc_set_rtcclock (void)
{
  // enable power interface clock
  // so access to DBP (disable backup write protection) bit is allowed,
  // needed to access to RTC, RTC Backup and RCC CSR registers
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;

  // enable access to RTC, RTC Backup & RCC CSR registers
  PWR->CR |= PWR_CR_DBP;
  (void) PWR->CR;                                // dummy read, make sure write action completed

#if USE_LSI_FOR_RTC
  // turn LSI oscillator on
  RCC->CSR |= RCC_CSR_LSION;

  // wait for LSI stable
  while ((RCC->CSR & RCC_CSR_LSIRDY) == 0)
    ;                   

  // select LSI as RTC clock source
  RCC->CSR |= RCC_CSR_RTCSEL_LSI; 
#endif

#if USE_LSE_FOR_RTC
  // turn LSE oscillator on
  RCC->CSR |= RCC_CSR_LSEON;

  // wait for LSE stable
  while ((RCC->CSR & RCC_CSR_LSERDY) == 0)
    ;                   

  // select LSE as RTC clock source
  RCC->CSR |= RCC_CSR_RTCSEL_LSE; 
#endif

  // enble RTC clock
  RCC->CSR |= RCC_CSR_RTCEN;

  // disable access to RTC, RTC Backup & RCC CSR registers
  PWR->CR &= ~PWR_CR_DBP;
}

// start 16Mc HSI clock (for ADC only)
void rcc_start_hsi (void)
{
  // enable ADC clock
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

  // turn HSI oscillator on
  RCC->CR |= RCC_CR_HSION;

  // wait for HSI stable
  while ((RCC->CR & RCC_CR_HSIRDY) == 0)
    ;                   
}

// stop 16Mc HSI clock (for ADC only)
void rcc_stop_hsi (void)
{
  // turn HSI oscillator on
  RCC->CR &= ~RCC_CR_HSION;
}

