// file    : adc.h
// author  : rb
// purpose : header file for adc.c
// date    : 171130
// last    : 191210
//

#ifndef _ADC_H_
#define _ADC_H_

// -- defines 
#define TFT_BLIGHT_DLY                          100   // delay TFT backlight turn off [s/10]

#define ADC_CH_EC1                                7   // ADC channel EC1
#define ADC_CH_EC2                                3   // ADC channel EC2
#define ADC_CH_EC3                               11   // ADC channel EC3 
#define ADC_CH_PH1                               15   // ADC channel pH1
#define ADC_CH_PH2                                6   // ADC channel pH2
#define ADC_CH_PH3                               13   // ADC channel pH3
#define ADC_CH_NTC1                              14   // ADC channel NTC1
#define ADC_CH_NTC2                               4   // ADC channel NTC2
#define ADC_CH_NTC3                              12   // ADC channel NTC3
#define ADC_CH_VREF                              17   // ADC channel Vref_int

#define LIGHT_FILTERDEPTH                        16   // filter for ambient light sensor 
#define PAR_FILTERDEPTH                          16   // filter for PAR light sensor 
#define PRESSURE_FILTERDEPTH                     16   // filter for pressure sensor 

#define ADC_MAX                              0x0fff   // max. value 12-bit ADC

#define ADC_CHAN_PRESSURE                         8   // ADC channel pressure sensor
#define ADC_CHAN_AMBLIGHT                         9   // ADC channel ambient light sensor
#define ADC_CHAN_PARLIGHT                        10   // ADC channel PAR light sensor
#define ADC_CHAN_VREF                            17   // ADC channel Vref_int
#define ADC_CHAN_VBAT                            18   // ADC channel Vbat

#define VREFINT_CAL_ADR  ((uint16_t *) 0x1ff800f8L)   // absolute address VREFINT calibration data
#define VREFINT_CAL_DAT          *(VREFINT_CAL_ADR)   // VREFINT calibration data
#define VREFINT_VDDA                          (3.0)   // VDDA used during VREFINT calibration 
#define ADC_FULLRANGE                      (4096.0)   // 12-bit ADC full range

// -- protoypes
void init_adc (void);

void adc_update (void);

uint16_t adc_read_ph  (uint8_t channel);
uint16_t adc_read_ec  (uint8_t channel);
uint16_t adc_read_ntc (uint8_t channel);

uint16_t adc_read_amblight (void);
uint16_t adc_read_parlight (void);
uint16_t adc_read_pressure (void);

void adc_tft_blight (void);

void adc_dump_registers (void);

void adc_state_update (void);

#endif
