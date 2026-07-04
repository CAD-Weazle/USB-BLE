// file    : adc.h
// author  : rb
// purpose : header file for adc.c
// date    : 171130
// last    : 200612
//

#ifndef _ADC_H_
#define _ADC_H_

// -- defines 
#define ADC_CHAN_VREF                          17  // ADC channel Vref_int
#define ADC_CHAN_VBAT                          19  // ADC channel Vbat
#define ADC_CHAN_VSOL                           1  // ADC channel Vsolar

#define VREFINT_CAL_ADR ((uint16_t *) 0x1ff800f8L) // absolute address VREFINT calibration data
#define VREFINT_CAL_DAT        *(VREFINT_CAL_ADR)  // VREFINT calibration data
#define VREFINT_VDDA                        (3.0)  // VDDA used during VREFINT calibration 
#define ADC_FULLRANGE                    (4095.0)  // 12-bit ADC full range

#define VBAT_EMPTY                         (3.10)  // battery empty clamp potential
#define VBAT_FULL                          (4.00)  // battery full clamp potential

// -- protoypes
void init_adc (void);

void adc_update (void);

uint32_t adc_read_vref (void);
float    adc_read_vbat (void);
float    adc_read_vsol (void);

uint32_t adc_read_channel (uint8_t chan);

uint8_t adc_battery_ok   (void);
uint8_t adc_battery_full (void);

void adc_dump (void);
void adc_dump_registers (void);

#endif


