// file    : adc.c
// author  : rb
// purpose : STM32F494xx ARM ADC routines
// date    : 171130
// last    : 191210
//

#include "includes.h"

//#define ADC_DEBUG

// init ADC1
void init_adc (void) 
{
  // enable clock for ADC1
  RCC->APB2ENR |= RCC_APB2Periph_ADC1;

  // set up ADC1, 12-bit resolution, no scan mode
  ADC1->CR1 = 0;

  // set up ADC1, single conversion mode
  ADC1->CR2 = ADC_CR2_EOCS |           // assert EOC after each conversion
              ADC_CR2_ADON;            // ADC1 on

  // set channel sample time <> wat is deze?
  // 0b000 =   3 cycles
  // 0b001 =  15 cycles
  // 0b010 =  28 cycles
  // 0b011 =  56 cycles
  // 0b100 =  84 cycles
  // 0b101 = 112 cycles
  // 0b110 = 144 cycles
  // 0b111 = 480 cycles
  ADC1->SMPR1 = (0b011 << 15) |       // channel 15 - PH0
                (0b000 << 12) |       // channel 14 - NTC0
                (0b011 <<  9) |       // channel 13 - PH2
                (0b000 <<  6) |       // channel 12 - NTC2
                (0b000 <<  3) |       // channel 11 - EC2
                (0b000 <<  0);        // channel 10 - PAR SENSOR

  ADC1->SMPR2 = (0b000 << 27) |       // channel  9 - PRESSURE note: v1.1 only
                (0b000 << 24) |       // channel  8 - LIGHT
                (0b000 << 21) |       // channel  7 - EC0
                (0b011 << 18) |       // channel  6 - PH1
                (0b000 << 12) |       // channel  4 - NTC1
                (0b000 <<  9);        // channel  3 - EC1

#ifdef ADC_DEBUG
  adc_dump_registers ();
#endif
}

// update some ADCs 
void adc_update (void)
{
  adc_state_update ();

  // monitor ambient lighting for TFT backlight control <> add later
//adc_tft_blight ();  
}

// read ADC channel pH sensor
uint16_t adc_read_ph (uint8_t channel)
{
  // set up scan order
  switch (channel)
  {
    // select sensor PH1
    case PH_SENSOR1:
      ADC1->SQR3 = (ADC_CH_PH1 << 0);
      break;

    // select sensor PH2
    case PH_SENSOR2:
      ADC1->SQR3 = (ADC_CH_PH2 << 0);
      break;

    // select sensor PH3
    case PH_SENSOR3:
      ADC1->SQR3 = (ADC_CH_PH3 << 0);
      break;

    default:
      break;
  }

  // start conversion regular channel
  ADC1->CR2 |= ADC_CR2_SWSTART;

  // wait for conversion ready, check EOC flag
  // note: hangs with leaving stop mode (fix this)
  while ((ADC1->SR & ADC_SR_EOC) == 0)
    ;  

  // read result, flip value & clear EOC flag
  return ((uint16_t)(ADC_MAX - ADC1->DR));
}

// read ADC channel EC sensor
uint16_t adc_read_ec (uint8_t channel)
{
  // set up scan order
  switch (channel)
  {
    // select sensor EC1
    case 0:
    {
      ADC1->SQR3 = (ADC_CH_EC1 << 0);
      break;
    }

    // select sensor EC2
    case 1:
    {
      ADC1->SQR3 = (ADC_CH_EC2 << 0);
      break;
    }

    // select sensor EC2
    case 2:
    {
      ADC1->SQR3 = (ADC_CH_EC3 << 0);
      break;
    }

    default:
      break;
  }

  ADC1->SQR2 = 0;
  ADC1->SQR1 = (0b0000 << 20);    // regular channels sequence length - 1

  // start conversion regular channel
  ADC1->CR2 |= ADC_CR2_SWSTART;

  // wait for conversion ready, check EOC flag
  // note: hangs with leaving stop mode (fix this)
  while ((ADC1->SR & ADC_SR_EOC) == 0)
    ;  

  // read result & clear EOC flag
  return ((uint16_t)ADC1->DR);
}

// read ADC channel NTCs 
uint16_t adc_read_ntc (uint8_t channel)
{
  // set up scan order
  switch (channel)
  {
    case 0:
    {
      // select sensor NTC1
      ADC1->SQR3 = (ADC_CH_NTC1 << 0);
      break;
    }

    case 1:
    {
      // select sensor NTC2
      ADC1->SQR3 = (ADC_CH_NTC2 << 0);
      break;
    }

    case 2:
    {
      // select sensor NTC3
      ADC1->SQR3 = (ADC_CH_NTC3 << 0);   
      break;
    }

    default:
      break;
  }

  ADC1->SQR2 = 0;
  ADC1->SQR1 = (0b0000 << 20);    // regular channels sequence length - 1

  // start conversion regular channel
  ADC1->CR2 |= ADC_CR2_SWSTART;

  // wait for conversion ready, check EOC flag
  // note: hangs with leaving stop mode (fix this)
  while ((ADC1->SR & ADC_SR_EOC) == 0)
    ;  

  // read result & clear EOC flag
  return ((uint16_t)ADC1->DR);
}

// read ADC channel ambient light sensor
uint16_t adc_read_amblight (void)
{
  static uint8_t  first = 1;                // filter speed-up flag 
  static uint16_t adc;                      // actual raw ADC light value
  static uint16_t tab[LIGHT_FILTERDEPTH];   // storage for moving average filter
         uint16_t sum = 0;                  // ADC data average 

  // set up scan order
  ADC1->SQR3 = ADC_CHAN_AMBLIGHT;          
  ADC1->SQR2 = 0;
  ADC1->SQR1 = (0b0000 << 20);              // regular channels sequence length - 1

  // start conversion regular channel
  ADC1->CR2 |= ADC_CR2_SWSTART;

  // wait for conversion ready, check EOC flag
  // note: hangs with leaving stop mode (fix this)
  while ((ADC1->SR & ADC_SR_EOC) == 0)
    ;  

  // read ADC light sensor & clear EOC flag
  adc = (uint16_t)ADC1->DR;

  // average ADC data 
  if (first)
  {    
    // speed up filter 
    for (int i = 0; i < LIGHT_FILTERDEPTH; i++)
      tab[i] = adc;

    first = 0;
  }
  else
  {
    // shift historical data in array & add new data point
    for (int i = LIGHT_FILTERDEPTH-1; i > 0; i--)
      tab[i] = tab[i-1];
    
    tab[0] = adc;   
  }

  // calculate sum of all data points
  for (int i = 0; i < LIGHT_FILTERDEPTH; i++)
    sum += tab[i];

  // return average
  return (sum / LIGHT_FILTERDEPTH);
}

// read ADC channel PAR light sensor
uint16_t adc_read_parlight (void)
{
  // set up scan order
  ADC1->SQR3 = (ADC_CHAN_PARLIGHT << 0);         
  ADC1->SQR2 = 0;
  ADC1->SQR1 = (0b0000 << 20);              // regular channels sequence length - 1

  // start conversion regular channel
  ADC1->CR2 |= ADC_CR2_SWSTART;

  // wait for conversion ready, check EOC flag
  // note: hangs with leaving stop mode (fix this)
  while ((ADC1->SR & ADC_SR_EOC) == 0)
    ;  

  // read result & clear EOC flag
  return ((uint16_t)ADC1->DR);
}

// read ADC channel pressure sensor
uint16_t adc_read_pressure (void)
{
  static uint8_t  first = 1;                 // filter speed-up flag 
  static uint16_t adc;                       // actual raw ADC light value
  static uint16_t tab[PRESSURE_FILTERDEPTH]; // storage for moving average filter
         uint16_t sum = 0;                   // ADC data sum
         uint16_t avg;                       // ADC data average 

  // set up scan order
  ADC1->SQR3 = (ADC_CHAN_PRESSURE << 0);         
  ADC1->SQR2 = 0;
  ADC1->SQR1 = (0b0000 << 20);               // regular channels sequence length - 1

  // start conversion regular channel
  ADC1->CR2 |= ADC_CR2_SWSTART;

  // wait for conversion ready, check EOC flag
  // note: hangs with leaving stop mode (fix this)
  while ((ADC1->SR & ADC_SR_EOC) == 0)
    ;  

  // read ADC pressure sensor & clear EOC flag
  adc = (uint16_t)ADC1->DR;

  // average ADC data 
  if (first)
  {    
    // speed up filter 
    for (int i = 0; i < PRESSURE_FILTERDEPTH; i++)
      tab[i] = adc;

    first = 0;
  }
  else
  {
    // shift historical data in array & add new data point
    for (int i = PRESSURE_FILTERDEPTH-1; i > 0; i--)
      tab[i] = tab[i-1];
    
    tab[0] = adc;   
  }

  // calculate sum of all data points
  for (int i = 0; i < PRESSURE_FILTERDEPTH; i++)
    sum += tab[i];

  avg = sum / PRESSURE_FILTERDEPTH;

  // return average & add offset error
  return ((uint16_t)(((float)avg/3.3419) + 6.0));
}

// monitor ambient lighting for TFT backlight control 
void adc_tft_blight (void)
{
  static uint8_t cnt = 0;
  static uint8_t on = 1;

  // delayed TFT backlight turn off
  if (ms->amb < 1850)  
  {
    if (cnt < TFT_BLIGHT_DLY)
      cnt++;
  }
  else
    cnt = 0;

  // control TFT backlight
  if ((cnt >= TFT_BLIGHT_DLY) && (on == 1))
  {
    eve_backlight (EVE_BLIGHT_OFF);
    on = 0;
  }
  else if ((cnt == 0) && (on == 0))
  {
    eve_backlight (EVE_BLIGHT_ON);
    on = 1;
  }
}

// dump ADC registers
void adc_dump_registers (void)
{
  printf ("-- ADC registers --\n");
  printf ("ADC1_CR1   : %08lx\n", ADC1->CR1);
  printf ("ADC1_CR2   : %08lx\n", ADC1->CR2);
  printf ("ADC1_SMPR1 : %08lx\n", ADC1->SMPR1);
  printf ("ADC1_SMPR2 : %08lx\n", ADC1->SMPR2);
  printf ("ADC1_SQR1  : %08lx\n", ADC1->SQR1);
  printf ("ADC1_SQR2  : %08lx\n", ADC1->SQR2);
  printf ("ADC1_SQR3  : %08lx\n", ADC1->SQR3);
  printf ("ADC1_DR    : %08lx\n", ADC1->DR);
}

// update SunWeb Master state 
void adc_state_update (void)
{
  ms->amb = adc_read_amblight ();
  ms->par = adc_read_parlight ();
  ms->prs = adc_read_pressure ();
}
