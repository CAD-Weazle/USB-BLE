// file    : adc.c
// author  : rb
// purpose : ST32L1xx ARM ADC routines
// date    : 171130
// last    : 200318
//
// note    : VREFINT_CAL Raw data acquired at temperature of 30 °C ±5 °C VDDA= 3 V ±10 mV x1FF800F8 - 0x1FF800F9
//           is used for ADC-to-voltage calculation

#include "includes.h"

//#define ADC_DEBUG

// globals
uint32_t adc_dat[ADC_NUM_CHANNELS];
uint32_t adc_avg[ADC_NUM_CHANNELS];

uint32_t vrefint;

float vbat = 0.0;            // battery voltage
float vsol = 0.0;            // solar cell voltage
float vdda = 0.0;            // VDDA
float vamb = 0.0;            // ambient light sensor

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
  ADC1->SMPR1 = (0b111 <<  0) |  // channel 20 -- Vsolar (PB14)
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
                (0b010 <<  3) |  // channel 1
                (0b010 <<  6) |  // channel 2
                (0b010 <<  9) |  // channel 3
                (0b010 << 12) |  // channel 4  -- light sensor (PA4)
                (0b010 << 15) |  // channel 5
                (0b010 << 18) |  // channel 6
                (0b010 << 21) |  // channel 7
                (0b010 << 24) |  // channel 8
                (0b010 << 27);   // channel 9

  // setup scan sequence length & scan order
  ADC1->SQR1 = ((ADC_NUM_CHANNELS-1) << 20);     // sequence length
  ADC1->SQR2 = 0;
  ADC1->SQR3 = 0;   
  ADC1->SQR4 = 0;   
  ADC1->SQR5 = (ADC_CHAN_LIGHT << 15) | 
               (ADC_CHAN_VSOL  << 10) | 
               (ADC_CHAN_VBAT  << 5)  | 
               (ADC_CHAN_VREF  << 0);

  ADC1->CR1 = ADC_CR1_SCAN;                      // enable channel scanning 
  ADC1->CR2 = 0;                                 // disable ADC, clear delay bits

#ifdef ADC_DEBUG
  adc_dump_registers ();
#endif

  // setup DMA channel
  init_dma_adc (adc_dat, ADC_NUM_CHANNELS);
}

// update ADC channels
void adc_update (void)
{
  // read ADC channels
  adc_scan (); 

  // convert raw ADC data to voltages
  adc_convert ();
}

// read ADC channels
void adc_scan (void)
{
  // power up ADC
  adc_power_on ();

  // read ADC1 channels & filter data
  for (int i = 0; i < ADC_NUM_SAMPLES; i++)
  {
    // setup ADC, enable DMA & start conversion
    ADC1->CR2 |= ADC_CR2_DMA | ADC_CR2_DDS | ADC_CR2_SWSTART;

    // wait for DMA transfer complete
    while ((DMA1->ISR & DMA_ISR_TCIF1) == 0)   
      usleep (10);

    // clear DMA1 Chanel1 Transfer Complete flag 
    DMA1->IFCR |= DMA_ISR_TCIF1;                   // write '1' to reset flag

    // filter new ADC data
    adc_filter (adc_dat, adc_avg);
  }

  // shut down ADC
  adc_power_off ();
}

// filter raw ADC data
void adc_filter (uint32_t *dat_in, uint32_t *dat_out)
{
  static uint16_t cnt = 0;
  static uint32_t tmp[ADC_NUM_CHANNELS] = {0, 0, 0};

  // fill filter
  if (cnt++ < ADC_NUM_SAMPLES) 
  {
    for (int i = 0; i < ADC_NUM_CHANNELS; i++)
      tmp[i] += dat_in[i];
  }

  // calculate average
  if (cnt == ADC_NUM_SAMPLES)
  {
    for (int i = 0; i < ADC_NUM_CHANNELS; i++)
    {
      dat_out[i] = (tmp[i] / ADC_NUM_SAMPLES);
      tmp[i] = 0;
    }

    cnt = 0;
  }
}

// convert ADC raw data to voltages
void adc_convert (void)
{
  // get internal Vref
  vrefint = adc_avg[0];

  // sanity check
  if (vrefint == 0)
    vrefint = 1700; 

  // calculate Vbat (resistor divider R + R -> factor 2.0)
  vbat = (2.0 * (VREFINT_VDDA*VREFINT_CAL_DAT*adc_avg[1]) / ((float)vrefint*ADC_FULLRANGE));

  // calculate Vsol (resistor divider R + R -> factor 2.0)
  vsol = (2.0 * (VREFINT_VDDA*VREFINT_CAL_DAT*adc_avg[2]) / ((float)vrefint*ADC_FULLRANGE));

  // calculate VDDA (from UM)
  vdda = ((3.0 * VREFINT_CAL_DAT) / vrefint);

  // calculate Vsol (resistor divider 1M + 1M -> factor 2.0)
  vamb = (VREFINT_VDDA*VREFINT_CAL_DAT*adc_avg[3]) / ((float)vrefint*ADC_FULLRANGE);
}

// enable ADC
void adc_power_on (void)
{
  // enable GND connection voltage divider - measure
  vgnd_enable (); 

  // relax a bit, let voltage settle
  msleep (200);

  // wait until ADON can be set (while ADONS == 0b1) <> endless loop, needed???
//while ((ADC1->SR & ADC_SR_ADONS) != 0)
//  ;

  // enable ADC 
  ADC1->CR2 = ADC_CR2_ADON              |       // ADC1 on
              (0b001 < ADC_CR2_DELS_Pos);       // insert delay until ADC result read (due to slow HCLK)
}

// disable ADC
void adc_power_off (void)
{
  // wait until ADON can be cleared (while ADONS == 0b0)
  while ((ADC1->SR & ADC_SR_ADONS) == 0)
    ;

  // disable ADC (ADONS == 1 now)
  ADC1->CR2 = 0;                                 // clear delay bits, otherwise huge power consumption during Stop mode (WTF?)

  // disable GND connection voltage divider - leave floating
  vgnd_disable (); 
}

// enable Vrefint
void vrefint_enable (void)
{
  ADC->CCR |= ADC_CCR_TSVREFE;

  // relax a bit, let Vrefint settle
  msleep (5);
}

// disable Vrefint, save power during Stop mode
void vrefint_disable (void)
{
  ADC->CCR &= ~ADC_CCR_TSVREFE;
}

// dump ADC channels converted
void adc_dump (void)
{
  printf2 ("VDDA: %1.2f Vsol: %1.2f Vbat: %1.2f Vamb: %1.2f  \n", vdda, vsol, vbat, vamb); 
}

// dump ADC channels converted to BLE module
void adc_dump_ble (void)
{
//printf3 ("\E[35mVDDA: %1.2f Vsol: %1.2f Vbat: %1.2f Vamb: %1.2f \E[30m\n", vdda, vsol, vbat, vamb); 
  printf3 ("VDDA: %1.2f Vsol: %1.2f Vbat: %1.2f Vamb: %1.2f\n", vdda, vsol, vbat, vamb); 
}

// dump ADC channels raw
void adc_dump_raw (void)
{
  for (int i = 0; i < ADC_NUM_CHANNELS; i++)
    printf2 ("%d ", adc_dat[i]);

  printf2 ("  ");

  for (int i = 0; i < ADC_NUM_CHANNELS; i++)
    printf2 ("%d ", adc_avg[i]);

  printf2 ("\n");
}










/*
// read analog supply voltage (VDDA = 3V*VREFINT_CAL_DAT / VREFINT_DAT)
float adc_read_vdda (void)
{
   uint32_t vrefint = adc_read_vref ();

   return ((3.0 * VREFINT_CAL_DAT) / vrefint);
}

// read internal reference voltage VREFINT
uint32_t adc_read_vref (void)
{
  uint32_t dat = 0;

  // sample VREFINT a few times
  for (int i = 0; i < ADC_NUM_SAMPLES; i++)
    dat += adc_read_channel (ADC_CHAN_VREF);

  // return average  
  return (dat / ADC_NUM_SAMPLES);
}

// read battery voltage
float adc_read_vbat (void)
{
  uint32_t adc_dat = 0;
  float    adc_avg;

  // first get actual internal reference voltage (used for ADC-to-voltage calculations)
  uint32_t vrefint = adc_read_vref ();

  // enable GND connection voltage divider - measure
  vgnd_enable (); 

  // relax a bit
  msleep (1);

  // take samples & average
  for (int i = 0; i < ADC_NUM_SAMPLES; i++)
    adc_dat += adc_read_channel (ADC_CHAN_VBAT);

  // disable GND connection voltage divider - leave floating
  vgnd_disable (); 

  // calculate average Vbat
  adc_avg = adc_dat/ADC_NUM_SAMPLES;

  // convert ADC value to Vbat (use VREFINT calibration data + actual ADC read-out data)
  // note: 12-bit ADC data, voltage divider = 5M6/5M6 -> factor 2.0
  return (2.0 * (VREFINT_VDDA*VREFINT_CAL_DAT*adc_avg) / ((float)vrefint*ADC_FULLRANGE));
}

// read solar cell voltage
float adc_read_vsol (void)
{
  uint32_t adc_dat = 0;
  float    adc_avg;

  // first get actual internal reference voltage (used for ADC-to-voltage calculations)
  uint32_t vrefint = adc_read_vref ();

  // enable GND connection voltage divider - measure
  vgnd_enable (); 

  // relax a bit
  msleep (1);

  // take samples & average
  for (int i = 0; i < ADC_NUM_SAMPLES; i++)
    adc_dat += adc_read_channel (ADC_CHAN_VSOL);

  // disable GND connection voltage divider - leave floating
  vgnd_disable (); 

  // calculate average Vsol
  adc_avg = adc_dat/ADC_NUM_SAMPLES;

  // convert ADC value to Vbat (use VREFINT calibration data + actual ADC read-out data)
  // note: 12-bit ADC data, voltage divider = 5M6/5M6 -> factor 2.0
  return (2.0 * (VREFINT_VDDA*VREFINT_CAL_DAT*adc_avg) / ((float)vrefint*ADC_FULLRANGE));
}
*/

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
  printf2 ("ADC_CSR    : %08lx\n", ADC->CSR);
  printf2 ("ADC_CCR    : %08lx\n", ADC->CCR);
}
