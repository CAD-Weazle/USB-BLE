// file    : power.c
// author  : rb
// purpose : STM32L1xx power control routines
// date    : 171127
// last    : 200324
//
// note    : wake-up event is enabled/disabled to counter spurs

#include "includes.h"

// setup power control
void init_power (void)
{
  // set core voltage
  power_set_corevoltage ();

  // debug mode off
  DBGMCU->CR     = 0;
  DBGMCU->APB1FZ = 0;
  DBGMCU->APB2FZ = 0;

  // set EXTI event
  SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI0_PA;   // select PA[0] as source for EXTI0 event
}

// power down ARM and BLE module & wait for wake-up event
void power_down (void)
{
  // clear PDD bit in PWR_CR
  PWR->CR &= ~PWR_CR_PDDS;

  // voltage regulator in 'lo-power mode' during Deepsleep/Sleep/Low-power run mode
  PWR->CR |= PWR_CR_LPSDSR;

  // clear WUF bit in PWR_SCR
  PWR->CSR &= ~PWR_CSR_WUF;

  // clear flags in PWR_CR (CSBF & CWUF)
  PWR->CR |= PWR_CR_CSBF | 
             PWR_CR_CWUF;

  // most GPIO's to analog/input
  ports_stop ();

  // enable EXTI wake-up event - wake-up event is enabled/disabled to counter spurs
  power_event_enable ();

  // set SLEEPDEEP bit in Cortex-M3 System Control register
  SCB->SCR |= SCB_SCR_SLEEPDEEP;

  // Wait For Event (inline assembly (yeah) in 'core_cmInstr.h')
  __WFE ();
}

// power up ARM and BLE module
void power_up (void)
{
  // --- continue here after Event on PA[0] ---

  // disable EXTI wake-up event - wake-up event is enabled/disabled to counter spurs
  power_event_disable ();

  // clear SLEEPDEEP bit in Cortex-M3 System Control register
  SCB->SCR &= ~SCB_SCR_SLEEPDEEP;

  // start HSI again for ADC
  rcc_start_hsi ();

  // GPIO's back to normal
  ports_resume ();

  // relax a bit <> needed?
  msleep (10);
}

// enable wake-up event
void power_event_enable (void)
{
  // setup EXTI interrupt/event controller
  EXTI->PR    =  EXTI_PR_PR0;                    // clear interrupt pending bit
  EXTI->IMR  &= ~EXTI_IMR_MR0;                   // disable interrupt on EXTI line
  EXTI->EMR  |=  EXTI_EMR_MR0;                   // enable event on EXTI line
//EXTI->RTSR &= ~EXTI_RTSR_TR0;                  // disable trigger rising edge
//EXTI->FTSR |=  EXTI_FTSR_TR0;                  // enable trigger falling edge
  EXTI->RTSR |=  EXTI_RTSR_TR0;                  // enable trigger rising edge
  EXTI->FTSR &= ~EXTI_FTSR_TR0;                  // disable trigger falling edge
}

// disable wake-up event
void power_event_disable (void)
{
  // setup EXTI interrupt/event controller
  EXTI->PR    =  EXTI_PR_PR0;                    // clear interrupt pending bit
  EXTI->IMR  &= ~EXTI_IMR_MR0;                   // disable interrupt on EXTI line
  EXTI->EMR  &= ~EXTI_EMR_MR0;                   // disable event on EXTI line
  EXTI->RTSR &= ~EXTI_RTSR_TR0;                  // disable trigger rising edge
  EXTI->FTSR &= ~EXTI_FTSR_TR0;                  // disable trigger falling edge
}

// enter Ultra Low Power mode
void power_enter_ulpmode (void)
{
  PWR->CR |= PWR_CR_ULP;
}

// leave Ultra Low Power mode
void power_leave_ulpmode (void)
{
  PWR->CR &= ~PWR_CR_ULP;
}

// enter Low Power Run Mode - from ST, not used
// note: - can only be entered when Vcore is in Rage 2 (1V5)
//       - dynamic voltage scaling not allowed
//       - only Stop and Sleep modes with regulator configured in 
//         Low power mode are allowed
void power_enter_lprmode (void)
{
  PWR->CR |= PWR_CR_LPSDSR;
  PWR->CR |= PWR_CR_LPRUN;
}

// leave Low Power Run Mode - from ST, not used
void power_leave_lprmode (void)
{
  PWR->CR &= ~PWR_CR_LPRUN;
  PWR->CR &= ~PWR_CR_LPSDSR;
}

// set MCU voltage scaling range
void power_set_corevoltage (void)
{
  uint32_t tmp;

  // wait for MCU voltage stable 
  while (PWR->CSR & PWR_CSR_VOSF)
    ;

  // not allowed to clear VOS bits, use tmp variable
  tmp  = PWR->CR;                      // get PWR_CR register
  tmp &= ~PWR_MCU_CLEAR;               // clear VOS bits

  // set VOS bits
////tmp |= PWR_MCU_1V8;                  // 1V8 - 2891 uA run @ 1.048 Mc
////tmp |= PWR_MCU_1V5;                  // 1V5 - 2832 uA run @ 1.048 Mc
//  tmp |= PWR_MCU_1V2;                  // 1V2 - 2787 uA run @ 1.048 Mc (note: SYSCLK 2.1 Mc max. allowed)

  // 210218: stall test
  tmp |= PWR_MCU_1V8;                  // 1V8 - 2891 uA run @ 1.048 Mc
//tmp |= PWR_MCU_1V5;                  // 1V5 - 2832 uA run @ 1.048 Mc
//tmp |= PWR_MCU_1V2;                  // 1V2 - 2787 uA run @ 1.048 Mc (note: SYSCLK 2.1 Mc max. allowed)

  // store back
  PWR->CR = tmp;

  // wait for MCU voltage stable 
  while (PWR->CSR & PWR_CSR_VOSF)
    ;
}

