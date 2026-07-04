
// file    : adc.c
// author  : rb
// purpose : STM32L1xx ARM ADC routines
// date    : 171130
// last    : 200424
//
// note    : VREFINT_CAL Raw data acquired at temperature of 30 °C ±5 °C VDDA= 3 V ±10 mV x1FF800F8 - 0x1FF800F9
//           is used for ADC-to-voltage calculation

#include "includes.h"

//#define ADC_DEBUG

// globals
uint32_t vrefint;            // Vref raw voltage 

float vbat = 0.0;            // battery voltage [V]
float vsol = 0.0;            // solar cell voltage [V]
float vdda = 0.0;            // VDDA [V]

// init ADC1
void init_adc (void) 
{
  // enable clock for ADC1
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

  // set HSI divider 
  ADC->CCR = (0b10 << ADC_CCR_ADCPRE_Pos);       // ADC clock = HSI/4 (4 Mc)

  // set sample time
  // * channel sample times:
  // 0b000 =   3 cycles
  // 0b001 =  15 cycles
  // 0b010 =  28 cycles
  // 0b011 =  56 cycles
  // 0b100 =  84 cycles
  // 0b101 = 112 cycles
  // 0b110 = 144 cycles
  // 0b111 = 480 cycles
  ADC1->SMPR1 = (0b010 <<  0) |  // channel 20
                (0b010 <<  3) |  // channel 21
                (0b010 <<  6) |  // channel 22
                (0b010 <<  9) |  // channel 23
                (0b010 << 12) |  // channel 24
                (0b010 << 15) |  // channel 25
                (0b010 << 18) |  // channel 26
                (0b010 << 21) |  // channel 27
                (0b010 << 24) |  // channel 28
                (0b010 << 27);   // channel 29

  ADC1->SMPR2 = (0b010 <<  0) |  // channel 10
                (0b010 <<  3) |  // channel 11
                (0b010 <<  6) |  // channel 12
                (0b010 <<  9) |  // channel 13
                (0b010 << 12) |  // channel 14
                (0b010 << 15) |  // channel 15
                (0b010 << 18) |  // channel 16
                (0b010 << 21) |  // channel 17 -- Vrefint 
                (0b010 << 24) |  // channel 18 
                (0b111 << 27);   // channel 19 -- Vbat (PB13)

  ADC1->SMPR3 = (0b010 <<  0) |  // channel 0
                (0b111 <<  3) |  // channel 1  -- VSOL (PA1)
                (0b010 <<  6) |  // channel 2
                (0b010 <<  9) |  // channel 3
                (0b010 << 12) |  // channel 4  
                (0b010 << 15) |  // channel 5
                (0b010 << 18) |  // channel 6
                (0b010 << 21) |  // channel 7
                (0b010 << 24) |  // channel 8
                (0b010 << 27);   // channel 9

  // setup scan sequence length & scan order
  ADC1->SQR1 = (0 << 20);                        // sequence length = 1
  ADC1->SQR2 = 0;
  ADC1->SQR3 = 0;   
  ADC1->SQR4 = 0;   
  ADC1->SQR5 = ADC_CHAN_VREF;   

  ADC1->CR1 = 0;                                 // no channel scanning 
  ADC1->CR2 = 0;                                 // disable ADC, clear delay bits

  // set EOC flag behavior
  ADC1->CR2 |= ADC_CR2_EOCS;                     // set EOC flag after every conversion

  // get internal reference voltage (used for ADC-to-voltage calculations)
  vrefint = adc_read_vref ();

#ifdef ADC_DEBUG
  adc_dump_registers ();
#endif
}

// update ADCs
void adc_update (void)
{
  // read solar cell voltage
//vsol = adc_read_vsol ();

  // read battery voltage
  vbat = adc_read_vbat ();   
}

// read internal reference voltage VREFINT
uint32_t adc_read_vref (void)
{
  uint32_t dat = 0;

  // sample VREFINT a few times
  for (int i = 0; i < 16; i++)
    dat += adc_read_channel (ADC_CHAN_VREF);

  // return average  
  return (dat / 16);
}

// read battery voltage
float adc_read_vbat (void)
{
         float adc_raw;                // actual ADC data (typecasted, <> fix this?)
  static float adc_old;                // former sample for exponential filter
         float adc_avg;                // average ADC value after filtering

  static uint8_t first = 1;            // flag to speed up filter

  // enable GND connection voltage divider - measure
  vbat_enable (); 

  // read Vbat
  adc_raw = (float) adc_read_channel (ADC_CHAN_VBAT);

  // disable GND connection voltage divider - leave floating
  vbat_disable (); 

  // exponential filter: Xn = (1-a).Xn-1 + a.New (a = 1/4)
  if (first)
  {
    adc_avg = adc_raw;                 // speed up filter
    first = 0;                         // clear flag
  }
  else
    adc_avg = (7*adc_old + adc_raw) / 8.0;

  // store current sample
  adc_old = adc_avg;   

  // convert ADC value to Vbat (use VREFINT calibration data + actual ADC read-out data)
  // note: 12-bit ADC data, voltage divider = 1M/1M
  return (2.0 * (VREFINT_VDDA*VREFINT_CAL_DAT*adc_avg) / ((float)vrefint*ADC_FULLRANGE));
}

// read solar cell voltage
float adc_read_vsol (void)
{
         float adc_raw;                // actual ADC data (typecasted, <> fix this?)
  static float adc_old;                // former sample for exponential filter
         float adc_avg;                // average ADC value after filtering

  static uint8_t first = 1;            // flag to speed up filter

  // enable GND connection voltage divider - measure
  vsol_enable (); 

  // read Vbat
  adc_raw = (float) adc_read_channel (ADC_CHAN_VSOL);

  // disable GND connection voltage divider - leave floating
  vsol_disable (); 

  // exponential filter: Xn = (1-a).Xn-1 + a.New (a = 1/4)
  if (first)
  {
    adc_avg = adc_raw;                 // speed up filter
    first = 0;                         // clear flag
  }
  else
    adc_avg = (7*adc_old + adc_raw) / 8.0;

  // store current sample
  adc_old = adc_avg;   

  // convert ADC value to Vbat (use VREFINT calibration data + actual ADC read-out data)
  // note: 12-bit ADC data, voltage divider = 1M/1M
  return (2.0 * (VREFINT_VDDA*VREFINT_CAL_DAT*adc_avg) / ((float)vrefint*ADC_FULLRANGE));
}

// read ADC channel
uint32_t adc_read_channel (uint8_t chan)
{
  // enable ADC
  ADC1->CR2 |= ADC_CR2_ADON;

  // set single ADC channel sequence
  ADC1->SQR5 = (chan << 0);   

  // relax a bit (skip checking flags)
  msleep (50);

  // start conversion regular channel
  ADC1->CR2 |= ADC_CR2_SWSTART;

  // wait for conversion ready, check EOC flag (called EOCS for some reason, WTF?)
  // note: hangs with leaving stop mode (fix this)
  while ((ADC1->SR & ADC_SR_EOCS) == 0)
    ;  

  // disable ADC
  ADC1->CR2 &= ~ADC_CR2_ADON;

  // read result
  return ADC1->DR;
}

// check battery voltage still ok
uint8_t adc_battery_ok (void)
{
  return ((vbat > VBAT_EMPTY) ? 1 : 0);
}

// check battery fully charged
uint8_t adc_battery_full (void)
{
  return ((vbat >= VBAT_FULL) ? 1 : 0);
}


// dump solar cell & battery voltage
void adc_dump (void)
{
  printf2 ("Vsol: %1.3f Vbat: %1.3f  ", vsol, vbat);   
}

// dump ADC registers -- dev only
void adc_dump_registers (void)
{
  printf2 ("-- ADC registers --\n");
  printf2 ("ADC1_SR    : %08lx\n", ADC1->SR);
  printf2 ("ADC1_CR1   : %08lx\n", ADC1->CR1);
  printf2 ("ADC1_CR2   : %08lx\n", ADC1->CR2);
  printf2 ("ADC1_SMPR1 : %08lx\n", ADC1->SMPR1);
  printf2 ("ADC1_SMPR2 : %08lx\n", ADC1->SMPR2);
  printf2 ("ADC1_SMPR3 : %08lx\n", ADC1->SMPR3);
  printf2 ("ADC1_SQR1  : %08lx\n", ADC1->SQR1);
  printf2 ("ADC1_SQR2  : %08lx\n", ADC1->SQR2);
  printf2 ("ADC1_SQR3  : %08lx\n", ADC1->SQR3);
  printf2 ("ADC1_SQR4  : %08lx\n", ADC1->SQR4);
  printf2 ("ADC1_SQR5  : %08lx\n", ADC1->SQR5);
  printf2 ("ADC1_DR    : %08lx\n", ADC1->DR);
}
