// file    : adc.h
// author  : rb
// purpose : header file for adc.c
// date    : 171130
// last    : 200318
//

#ifndef _ADC_H_
#define _ADC_H_

// -- defines 
#define ADC_NUM_CHANNELS                        4  // number of ADC channels in use
#define ADC_NUM_SAMPLES                        16  // for average filter

#define ADC_CHAN_LIGHT                          4  // ADC channel light sensor
#define ADC_CHAN_VREF                          17  // ADC channel Vref_int
#define ADC_CHAN_VBAT                          19  // ADC channel Vbat
#define ADC_CHAN_VSOL                          20  // ADC channel Vsolar

#define VREFINT_CAL_ADR ((uint16_t *) 0x1ff800f8L) // absolute address VREFINT calibration data
#define VREFINT_CAL_DAT        *(VREFINT_CAL_ADR)  // VREFINT calibration data
#define VREFINT_VDDA                        (3.0)  // VDDA used during VREFINT calibration 
#define ADC_FULLRANGE                    (4095.0)  // 12-bit ADC full range

// -- protoypes
void init_adc (void);

void adc_update (void);

void adc_scan    (void); 
void adc_filter  (uint32_t *dat_in, uint32_t *dat_out);
void adc_convert (void);

void adc_power_on  (void);
void adc_power_off (void);

void vrefint_enable  (void);
void vrefint_disable (void);

void adc_dump     (void);
void adc_dump_ble (void);
void adc_dump_raw (void);

void adc_dump_registers (void);

#endif
