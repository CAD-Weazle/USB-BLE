// file   : ntc.h
// author : rb
// purpose: header file for ntc.c
// date   : 080622
// last   : 190914

#ifndef __NTC_H__
#define __NTC_H__

//#include "../includes.h"

// -- defines
#define NTC_NUM_DEVICES     3          // number of NTC sensors
#define NTC_FILTERDEPTH    64          // moving average filter depth

#define NTC_SENSOR1         0          // NTC sensor 1
#define NTC_SENSOR2         1          // NTC sensor 2
#define NTC_SENSOR3         2          // NTC sensor 3

#define R_PULL          10000          // value NTC pull-up resistor (note: Vpull-up = Vref ADC)
#define MAX_ADC          4096          // NTC is read with 12-bit ADC

// Steinhart-Hart Coefficients for NCP18XH103F03RB   
// from Cypress Excel datasheet
//#define NTC_PARAM_A  0.000891358 
//#define NTC_PARAM_B  0.000250618 
//#define NTC_PARAM_C  0.000000197 

// Steinhart-Hart Coefficients for TDS-NTC-38 sensor from Ali Express
// 10K NTC, B-value = 3950
// from Cypress Excel datasheet + some guessing (redo with real T measurements)
#define NTC_PARAM_A   0.001022613
#define NTC_PARAM_B   0.000253115
#define NTC_PARAM_C   1.6238E-10

#define NTC_TABLE_DEBUG     0          // include test code, dev only

#if (NTC_TABLE_DEBUG)
#define NUM_TABLE_POINTS   34

// temperature vs resistamce tanle from Murata data sheet
// NTC type NCP18XH103F03RB
// Ro = 10K, B = 3380K, SMD 0603
static float ntc_points [NUM_TABLE_POINTS][2] = 
{
  {-40.0, 195652.0},
  {-35.0, 148171.0},
  {-30.0, 113347.0},
  {-25.0,  87559.0},
  {-20.0,  68237.0},
  {-15.0,  53650.0},
  {-10.0,  42506.0},
  { -5.0,  33892.0},
  {  0.0,  27219.0},
  {  5.0,  22021.0},
  { 10.0,  17926.0},
  { 15.0,  14674.0},
  { 20.0,  12081.0},
  { 25.0,  10000.0},
  { 30.0,   8315.0},
  { 35.0,   6948.0},
  { 40.0,   5834.0},
  { 45.0,   4917.0},
  { 50.0,   4161.0},
  { 55.0,   3535.0},
  { 60.0,   3014.0},
  { 65.0,   2586.0},
  { 70.0,   2228.0},
  { 75.0,   1925.0},
  { 80.0,   1669.0},
  { 85.0,   1452.0},
  { 90.0,   1268.0},
  { 95.0,   1110.0},          
  {100.0,    974.0},
  {105.0,    858.0}, 
  {110.0,    758.0},
  {115.0,    672.0},
  {120.0,    596.0},
  {125.0,    531.0}
};
#endif

// -- prototypes
void init_ntc (void);

void ntc_update (void);

void     ntc_get     (void);
uint16_t ntc_filter  (uint16_t dat, uint8_t channel);
uint16_t ntc_convert (uint32_t adc_val, uint8_t channel);

void ntc_dump (void);

void ntc_get_error (void);
void ntc_dump_error (void);

void ntc_state_update (void);

// -- test/calibration routines
void  ntc_test_steinhart (void);
float ntc_simulate   (float T);

#endif



