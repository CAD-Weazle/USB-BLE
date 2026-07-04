// file    : ltc4150.c
// author  : rb
// purpose : routines for LTC4150 battery gas guage
// date    : 200426
// last    : 200615
//

#include "includes.h"

// globals
float charge_accu = 0.0;                    // accumulated total charge of battery since POR [C]
float charge_fnew;                          // filtered accumulated charge [C]
float charge_fold;                          // filtered accumulated charge [C] - state for charge current calculation

float current_rnew;                         // charge current [mA] - raw
float current_fnew;                         // charge current [mA] - filtered

// EXTI0 ISR (run time ~200us)
void EXTI0_IRQHandler (void)
{
  // clear pending interrupt
  EXTI->PR = EXTI_PR_PR0;                    

  // <> test only
//led_grn_on ();

  // update charge accu
  if (GPIOB->IDR & (1 << POL))
    charge_accu += dQ;                      // POL = 1: charging
  else
    charge_accu -= dQ;                      // POL = 0: discharging 

  // clear LTC4150 state
  ltc4150_clear ();

  // <> test only
//led_grn_off ();
}

// setup LTC4150 
void init_ltc4150 (void)
{
  // <> test only
  ltc4150_disable ();
  return;

  // clear LTC4150 state
  ltc4150_clear ();

  // setup SYSCFG register 
  SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI0_PA;        // select PA[0] as source for EXTI0 interrupt

  // setup EXTI interrupt/event controller
  EXTI->PR    =  EXTI_PR_PR0;                         // clear interrupt pending bit
  EXTI->IMR   =  EXTI_IMR_MR0;                        // enable interrupt on EXTI line
  EXTI->EMR  &= ~EXTI_EMR_MR0;                        // disable event on EXTI line
  EXTI->RTSR &= ~EXTI_RTSR_TR0;                       // disable trigger rising edge
  EXTI->FTSR |=  EXTI_FTSR_TR0;                       // enable trigger falling edge

  // enable EXTI0 interrupt
  NVIC_EnableIRQ (EXTI0_IRQn);
}

// handle LTC4150 state
void ltc4150_handle (void)
{
  // filter charge data 
  charge_fnew = ltc4150_cfilter (charge_accu);

  // calculate unfiltered charging current (I = dQ/dT) [mA]
  current_rnew = (charge_fnew - charge_fold) / 1;     // assume dT = 1.0000 sec update interval

  // calculate average charging current [mA]
  current_fnew = ltc4150_ifilter (current_rnew);

  // update state
  charge_fold = charge_fnew;
}

// filter accumulated charge data
float ltc4150_cfilter (float dat)
{
         float   sum = 0;
  static uint8_t first = 1;
  static float   csamples[LTC4150_CFILTERDEPTH];

  // add charge data to filter  
  if (first)
  {    
    // speed up filter 
    for (int i = 0; i < LTC4150_CFILTERDEPTH; i++)
      csamples[i] = dat;

    first = 0;
  }
  else
  {
    // shift historical data in array & add new data point
    for (int i = LTC4150_CFILTERDEPTH-1; i > 0; i--)
      csamples[i] = csamples[i-1];
    
    csamples[0] = dat;   
  }

  // calculate sum of all data points
  for (int i = 0; i < LTC4150_CFILTERDEPTH; i++)
    sum += csamples[i];

  // calculate average charge [C]
  return (sum / LTC4150_CFILTERDEPTH);
}

// filter charge current data
float ltc4150_ifilter (float dat)
{
         float   sum = 0;
  static uint8_t first = 1;
  static float   isamples[LTC4150_IFILTERDEPTH];

  // add charge data to filter  
  if (first)
  {    
    // speed up filter 
    for (int i = 0; i < LTC4150_IFILTERDEPTH; i++)
      isamples[i] = dat;

    first = 0;
  }
  else
  {
    // shift historical data in array & add new data point
    for (int i = LTC4150_IFILTERDEPTH-1; i > 0; i--)
      isamples[i] = isamples[i-1];
    
    isamples[0] = dat;   
  }

  // calculate sum of all data points
  for (int i = 0; i < LTC4150_IFILTERDEPTH; i++)
    sum += isamples[i];

  // calculate average current [mA]
  return (1000*sum / LTC4150_IFILTERDEPTH);
}

// enable LTC4150
void ltc4150_enable (void)
{
  gpio_set (GPIOB, SHDN_L);
}

// disable LTC4150
void ltc4150_disable (void)
{
  gpio_clr (GPIOB, SHDN_L);
}

// clear LTC4150 state
void ltc4150_clear (void)
{
  gpio_clr (GPIOB, CLR_L);
  usleep (100); 
  gpio_set (GPIOB, CLR_L);
}

// dump LTC4150 state
void ltc4150_dump (void)
{
//printf2 ("Qtot: %3.4f Ichg: %3.4f\n", charge_fnew, current_cur);   
  printf2 ("Qtot: %3.4f Ichg: %3.4f Iraw: %3.4f\n", charge_fnew, current_fnew, current_rnew*1000);   // <> test only
}

