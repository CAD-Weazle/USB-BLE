// file    : ntc.c
// author  : rb
// purpose : NTC linearization routines (temperature sensor in EC probes)
// date    : 080622
// last    : 191219
//
// old    : NCP18XH103F03RB, Murata  10K/1% NTC used, RS# 725-9050
//          B-value = 3380 ±1% (25-50oC)
//
// note   : TDS-NTC-38 sensor used from Ali Express
//          10K NTC, B-value = 3950 (??-??oC, assume 25-50oC)

// use Cypress "AN2017_S_H_Constant_Calc.xls" for Steinhart-Hart coeff. calculation
//

#include "includes.h"

// globals
uint32_t ntc_avg[NTC_NUM_DEVICES];                    // NTC ADC data average 
uint16_t ntc_temp[NTC_NUM_DEVICES];                   // NTC temperature, fixed point notation [doC]
uint16_t ntc_ftab[NTC_NUM_DEVICES][NTC_FILTERDEPTH];  // storage for moving average filter
 int16_t ntc_err[NTC_NUM_DEVICES];                    // temperature offset errors retrieved from user FLASH

// init NTC measurement
void init_ntc (void)
{
  // read NTC offset error parameters from EEPROM
  ntc_get_error ();
}

// handle NTC sensors
void ntc_update (void)
{
  // read sensors & update global state
  ntc_get ();
  ntc_state_update ();
}

// read NTC & filter 
void ntc_get (void)
{
  uint32_t ntc_dat[NTC_NUM_DEVICES];
  
  for (int i = 0; i < NTC_NUM_DEVICES; i++)
  {
    // read ADC 
    ntc_dat[i] = adc_read_ntc (i);

    // apply moving average filter
    ntc_avg[i] = ntc_filter (ntc_dat[i], i);
  
    // convert to temperature
    ntc_temp[i] = ntc_convert (ntc_avg[i], i);
  }
}

// naive moving average filter
uint16_t ntc_filter (uint16_t dat, uint8_t channel)
{
         uint32_t sum = 0;
  static uint8_t  first[NTC_NUM_DEVICES] = {1, 1, 1};

  // sanity check
  if (channel >= NTC_NUM_DEVICES)
    return 0;

  // speed up filter 
  if (first[channel])
  {
    for (int i = 0; i < NTC_FILTERDEPTH; i++)
      ntc_ftab [channel][i] = dat;

    first[channel] = 0;
  }
  
  // shift historical data in array & add new data point
  for (int i = NTC_FILTERDEPTH-1; i > 0; i--)
    ntc_ftab [channel][i] = ntc_ftab [channel][i-1];

  ntc_ftab [channel][0] = dat;

  // calculate average of all data points
  for (int i = 0; i < NTC_FILTERDEPTH; i++)
    sum += ntc_ftab[channel][i];

  return ((uint16_t)(sum/NTC_FILTERDEPTH));
}

// calculate NTC restistance from ADC data & convert to temperature
uint16_t ntc_convert (uint32_t adc_val, uint8_t channel)
{
  float R;

  // sanity check
  if (channel >= NTC_NUM_DEVICES)
    return 0;

  // calculate NTC resistance from voltage divider output 
  R = (float)((R_PULL*adc_val) / (MAX_ADC - adc_val));

  // calculate temperature from NTC resistance using Steinhart-Hart & add correction
  // (*100 for conversion to double digit fixed point notation)
  return (uint16_t)(((1.0/(NTC_PARAM_A + \
                   log(R)*(NTC_PARAM_B + NTC_PARAM_C*log(R)*log(R)))) - 273.15) * 100) \
                   + ntc_err[channel];
}

// dump NTC data
void ntc_dump (void)
{
#ifdef NTC_DEBUG
  printf ("NTC_RAW: %d %d %d ", ntc_avg[0], ntc_avg[1], ntc_avg[2]);
#endif 
  printf ("NTC_TEMP: %d.%02d %d.%02d %d.%02d\n", ntc_temp[0] / 100, ntc_temp[0] % 100, 
                                                 ntc_temp[1] / 100, ntc_temp[1] % 100,
                                                 ntc_temp[2] / 100, ntc_temp[2] % 100);
}

// get NTC error parameter from EEPROM
void ntc_get_error (void)
{
  // read data from EEPROM
  for (int i = 0; i < NTC_NUM_DEVICES; i++)
    ntc_err[i] = ee_get_ntccal (i);
}

// dump NTC error parameters, working set stored in SRAM 
void ntc_dump_error (void)
{
  ntc_get_error ();

  for (int i = 0; i < NTC_NUM_DEVICES; i++)
    printf ("E%d: %02d ", i, ntc_err[i]);

  printf ("\n");
}

// update SunWeb Master state 
void ntc_state_update (void)
{
  for (int i = 0; i < NTC_NUM_DEVICES; i++)
    ms->ntc[i] = ntc_temp[i];
}















// -- only needed during development with new NTC --

#if (NTC_TABLE_DEBUG)
// compare calculated temperature against data from NTC R/T table
void ntc_test_steinhart (void)
{
  float Ttable, Tstein, Rtable;

  for (int i = 0; i < NUM_TABLE_POINTS; i++)
  {
    Rtable = ntc_points[i][1];         // read resistance from table
    Ttable = ntc_points[i][0];         // read temperature from table
    Tstein = steinhart (Rtable);       // calculated T

    printf ("Ttable: %4d ", (long)Ttable);  
    printf ("Rtable: %7d ", (long)Rtable);  
    printf ("Tstein: %d\n", (long)Tstein*10);  
  }
}

// simulate NTC using beta parameter, dev only
// note: not very accurate
float ntc_simulate (float T)
{

  // NTC: Rt = Ro * exp (B * (1/T - 1/To)).
  //         = Ro * exp (-B/To) * exp (B/T)
  //         =  C * exp (B/T)
  // Ro = 10K
  // B  = 3380
  // To = 25 oC (298 K)

  T += 273.15;
  return (exp (3380/T) * 1.4838E-03); // wrong fakte, what'ev
}
#endif







