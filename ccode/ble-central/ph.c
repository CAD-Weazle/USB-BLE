// file    : ph.c
// author  : rb
// purpose : pH
// date    : 161111
// last    : 191210
//

//#define PH_DEBUG_ON

#include "includes.h"

// globals
uint16_t ph_adc[PH_NUM_DEVICES];                      // actual raw ADC pH value
uint16_t ph_tab[PH_NUM_DEVICES][PH_FILTERDEPTH];      // storage for moving average filter
uint32_t ph_avg[PH_NUM_DEVICES]  = {0, 0, 0};         // pH ADC data average 

uint16_t ph4_cal[PH_NUM_DEVICES];                     // holds raw ADC calibration value pH 4
uint16_t ph7_cal[PH_NUM_DEVICES];                     // holds raw ADC calibration value pH 7

float    ph_ofs[PH_NUM_DEVICES];                      // offset in 2-point linear fit 

uint16_t ph_cur[PH_NUM_DEVICES];                      // actual pH

// state
uint8_t ph_cal_probe;                                 // sensor being calibrated
uint8_t ph_cal_busy = 0;                              // normal operation / calibration flag

uint8_t ph_log = 0;                                   // log pH data to serial port


// init pH sensors
void init_ph (void)
{
  float offs4[PH_NUM_DEVICES];
  float offs7[PH_NUM_DEVICES];

  // read calibration data from EEPROM
  for (int probe = 0; probe < PH_NUM_DEVICES; probe++)
  {
    ph4_cal[probe] = ee_get_phcal (probe, PH4_CALPOINT);
    ph7_cal[probe] = ee_get_phcal (probe, PH7_CALPOINT);
  }

  // calculate pH offset in 2-point linear fit 
  for (int probe = 0; probe < PH_NUM_DEVICES; probe++)
  {
    offs4[probe] = PH4_CALPOINT - (PH_CALSPAN * (float)ph4_cal[probe] / (float)(ph7_cal[probe] - ph4_cal[probe]));
    offs7[probe] = PH7_CALPOINT - (PH_CALSPAN * (float)ph7_cal[probe] / (float)(ph7_cal[probe] - ph4_cal[probe]));

    // offs4 and offs7 should be equal, take mean anyways
    ph_ofs[probe] = (offs4[probe] + offs7[probe]) / 2.0;
  }

  // power analog pH circuit
  ph_power_on ();
}

// handle pH sensors
void ph_update (void)
{
  if (ph_cal_busy)
  {
    // calibrate pH sensors
    ph_calibrate ();
  }
  else
  {
    // read sensors & update global state
    ph_get ();
  }
}

// -- pH update routines

// calculate current pH
void ph_get (void)
{
  static uint8_t first = 1;

  uint32_t ph_sum[PH_NUM_DEVICES] = {0, 0, 0};
  float    ph[PH_NUM_DEVICES];

  // make sure calibration flag is reset
  ms->ph_cal_busy = 0;
  
  // read ADC pH sensor 
  for (int probe = 0; probe < PH_NUM_DEVICES; probe++)
  {
    ph_adc[probe] = adc_read_ph (probe);
  }

  // average ADC data 
  if (first)
  {    
    // speed up filter 
    for (int probe = 0; probe < PH_NUM_DEVICES; probe++)
      for (int i = 0; i < PH_FILTERDEPTH; i++)
        ph_tab[probe][i] = ph_adc[probe];

    first = 0;
  }
  else
  {
    // shift historical data in array & add new data point
    for (int probe = 0; probe < PH_NUM_DEVICES; probe++)
    {
      for (int i = PH_FILTERDEPTH-1; i > 0; i--)
        ph_tab[probe][i] = ph_tab[probe][i-1];
    
      ph_tab[probe][0] = ph_adc[probe];
    }
  }

  // calculate average of all data points
  for (int probe = 0; probe < PH_NUM_DEVICES; probe++)
  {
    for (int i = 0; i < PH_FILTERDEPTH; i++)
      ph_sum[probe] += ph_tab[probe][i];

    ph_avg[probe] = ph_sum[probe] / PH_FILTERDEPTH;
  }

  // calculate pH
  for (int probe = 0; probe < PH_NUM_DEVICES; probe++)
  {
    ph[probe] = ((PH_CALSPAN * (float)ph_avg[probe]) / (float)(ph7_cal[probe] - ph4_cal[probe]) ) + ph_ofs[probe];
  
    // sanity check
    if (ph[probe] < 0.0)
      ph[probe] = 0.0;

    if (ph[probe] > 14.0)
      ph[probe] = 14.0;

    // round off & cast to int [centi-pH]
    ph_cur[probe] = (uint16_t)((ph[probe] + 0.005) * 100);

    // update global state
    ms->ph[probe] = ph_cur[probe];
  }
}

// dump current pH
void ph_dump (void)
{
  for (int i = 0; i < PH_NUM_DEVICES; i++)
  {
    printf ("pH%d: %d.%02d ", i+1, ph_cur[i]/100, ph_cur[i]%100);
  }

  printf ("\n");
}

// dump current pH - ADC counts
void ph_dump_raw (void)
{
  for (int i = 0; i < PH_NUM_DEVICES; i++)
  {
    printf ("pH%d_raw: %ld ", i+1, ph_avg[i]);
  }
}


// -- calibration routines

// start calibration process
void ph_cal_sensor (uint8_t probe)
{
  // sanity check
  if (probe >= PH_NUM_DEVICES)
    return;

  // check which sensor to calibrate & start calibration
  ph_cal_probe = probe;
  ph_cal_busy  = 1;

  // update global state
  ms->ph_cal_probe = probe;   // set pH sensor number
  ms->ph_cal_start = 0;       // make sure calibration start flag is reset
  ms->ph_cal_busy  = 1;       // flag calibration busy, waiting for 'ph_cal_start'
}

// calibrate pH sensor
void ph_calibrate (void)
{
  // flags
  static uint8_t  cal_state = PH_CAL_IDLE;  // FSM state
  static uint16_t cal_wait = 0;             // calibration waiting time counter
  static uint8_t  first = 1;                // filter speed-up flag

  // filter 
  uint16_t        raw_adc;                  // raw ADC value read during calibration
  static uint32_t avg_adc4 = 0;             // average ADC value after filtering
  static uint32_t avg_adc7 = 0;             // average ADC value after filtering
  uint32_t        sum_adc  = 0;             // for ADC data filtering
  
  // pH sensor calibration state machine
  switch (cal_state)
  {
    // -------- INITIAL PART ------------
    // check for calibration in progress, else bail
    case PH_CAL_IDLE:
    {
      led_red_on ();                    // signal calibration busy
      master_log_stop ();               // stop log output SunWeb
      ph_log_stop ();                   // stop log output pH sensors

      // wait for start signal TFT code (TFT button press)
      if (ms->ph_cal_start)
        cal_state = PH_CAL_START;

      break;
    }

    // signal user calibration has started
    case PH_CAL_START:
    {
      printf ("\n#pH sensor %d: start calibration\n", ph_cal_probe+1);

      cal_state = PH_CAL_4;

      break;
    }

    // -------- pH SENSOR CALIBRATION PART ------------

    // put pH sensor in calibration liquid with pH 4
    case PH_CAL_4:
    {
      printf ("#pH sensor %d: put sensor in pH 4 calibration solution & wait\n", ph_cal_probe+1);

      // update global state for TFT handling
      ms->ph_cal_stage = PH_CAL_4_WAIT;

      // set filter speed-up flag & switch state
      first = 1;
      cal_state = PH_CAL_4_WAIT;

      break;
    }

    // & wait some time to reach equilibrium
    case PH_CAL_4_WAIT:
    {
      // signal calibration busy
      led_red_toggle ();

      // read ADC selected pH sensor
      raw_adc = adc_read_ph (ph_cal_probe);

      // average ADC data 
      if (first)
      {    
        // speed up filter 
        for (int i = 0; i < PH_FILTERDEPTH; i++)
          ph_tab[ph_cal_probe][i] = raw_adc;

        first = 0;
      }
      else
      {
        // shift historical data in array & add new data point
        for (int i = PH_FILTERDEPTH-1; i > 0; i--)
          ph_tab[ph_cal_probe][i] = ph_tab[ph_cal_probe][i-1];
    
        ph_tab[ph_cal_probe][0] = raw_adc;
      }

      // calculate average of all data points
      for (int i = 0; i < PH_FILTERDEPTH; i++)
        sum_adc += ph_tab[ph_cal_probe][i];

      avg_adc4 = sum_adc / PH_FILTERDEPTH;
  
      // count down waiting time & display raw / average ADC value
      if (!(cal_wait % 10))
        printf ("#pH sensor %d: calibration busy, please wait: %2d - raw: %d avg: %ld\n", ph_cal_probe+1, (PH_CAL_WAIT - cal_wait)/10, raw_adc, avg_adc4);

      // update progress for TFT display
      ms->ph_cal_waittime = PH_CAL_WAIT;
      ms->ph_cal_progress = cal_wait;
 
      // wait some time for accurate calibration 
      if (cal_wait++ >= PH_CAL_WAIT)
      {
        // check if raw measurement is within bounds for a valid pH 4 calibration
        if ((avg_adc4 > PH4_NORMAL_ADC + PH_MAXERR) || (avg_adc4 < PH4_NORMAL_ADC - PH_MAXERR))
        {
          // calibration was not valid, bail & don't update EEPROM
          printf ("\n#pH sensor %d: calibration pH 4 failed\n", ph_cal_probe+1);

          // update global state for TFT handling
          ms->ph_cal_stage = PH_CAL_ERR;

          // bail on error
          cal_state = PH_CAL_ERR;
          break;
        }
        else
        {
          // valid calibration, but update EEPROM later
          printf ("\n#pH sensor %d: calibration pH 4 succeeded\n", ph_cal_probe+1);

          // update global state for TFT handling
          ms->ph_cal_stage = PH_CAL_CLEAN;

          // reset wait counter & switch state
          cal_wait  = 0;       
          cal_state = PH_CAL_CLEAN;
          break;
        }
      }

      break;
    }

    case PH_CAL_CLEAN:
    {
      if (cal_wait >= PH_CAL_RINSE)
      {
        // reset wait counter & switch state
        cal_wait  = 0;
        cal_state = PH_CAL_7;
      }  
      else
      {
        if (!(cal_wait % 10))
          printf ("#pH sensor %d: clean, please wait: %2d\n", ph_cal_probe+1, (PH_CAL_RINSE - cal_wait)/10);

        // update progress for TFT display
        ms->ph_cal_waittime = PH_CAL_RINSE;
        ms->ph_cal_progress = cal_wait;

        cal_wait++;
      }

      break;
    }

    // put pH sensor in calibration liquid with pH 7
    case PH_CAL_7:
    {
      printf ("#pH sensor %d: put sensor in pH 7 calibration solution & wait\n", ph_cal_probe+1);

      // update global state for TFT handling
      ms->ph_cal_stage = PH_CAL_7_WAIT;

      // reset filter speed-up flag & switch state
      first = 1;
      cal_state = PH_CAL_7_WAIT;

      break;
    }

    // & wait some time to reach equilibrium
    case PH_CAL_7_WAIT:
    {
      // signal calibration busy
      led_red_toggle ();

      // read ADC selected pH sensor
      raw_adc = adc_read_ph (ph_cal_probe);

      // average ADC data 
      if (first)
      {    
        // speed up filter 
        for (int i = 0; i < PH_FILTERDEPTH; i++)
          ph_tab[ph_cal_probe][i] = raw_adc;

        first = 0;
      }
      else
      {
        // shift historical data in array & add new data point
        for (int i = PH_FILTERDEPTH-1; i > 0; i--)
          ph_tab[ph_cal_probe][i] = ph_tab[ph_cal_probe][i-1];
    
        ph_tab[ph_cal_probe][0] = raw_adc;
      }

      // calculate average of all data points
      for (int i = 0; i < PH_FILTERDEPTH; i++)
        sum_adc += ph_tab[ph_cal_probe][i];

      avg_adc7 = sum_adc / PH_FILTERDEPTH;
  
      // count down waiting time & display raw / average ADC value
      if (!(cal_wait % 10))       
        printf ("#pH sensor %d: calibration busy, please wait: %2d - raw: %d avg: %ld\n", ph_cal_probe+1, (PH_CAL_WAIT - cal_wait)/10, raw_adc, avg_adc7);

      // update progress for TFT display
      ms->ph_cal_waittime = PH_CAL_WAIT;
      ms->ph_cal_progress = cal_wait;
  
      // wait some time for accurate calibration 
      if (cal_wait++ >= PH_CAL_WAIT)
      {
        // check if raw measurement is within bounds for a valid pH 7 calibration
        if ( (avg_adc7 > PH7_NORMAL_ADC + PH_MAXERR) || (avg_adc7 < PH7_NORMAL_ADC - PH_MAXERR) )
        {
          // calibration was not valid, bail & don't update FLASH
          printf ("\n#pH sensor %d: calibration pH 7 failed\n", ph_cal_probe+1);

          // update global state for TFT handling
          ms->ph_cal_stage = PH_CAL_ERR;

          // bail on error
          cal_wait  = PH_CAL_ERR_DLY;
          cal_state = PH_CAL_ERR;
          break;
        }
        else
        {
          // valid calibration, update EEPROM
          printf ("\n#pH sensor %d: calibration pH 7 succeeded\n", ph_cal_probe+1);

          // store in eeparms when both calibrations valid
          ee_set_phcal (ph_cal_probe, PH4_CALPOINT, avg_adc4);
          ee_set_phcal (ph_cal_probe, PH7_CALPOINT, avg_adc7);

          // update pH calculation parameters
          init_ph ();

          // update global state for TFT handling
          ms->ph_cal_stage = PH_CAL_OK;

          // all done ok
          cal_wait  = PH_CAL_OK_DLY;
          cal_state = PH_CAL_OK;
          break;
        }
      }

      break;
    }

    // -------- FINAL PART ------------

    // calibration error
    case PH_CAL_ERR:
    {
      // only print once
      if (cal_wait == PH_CAL_ERR_DLY)
        printf ("#calibration failed, parameters not updated in EEPROM\n\n");
      
      // wait a bit for windows display on TFT
      if (!(cal_wait-- > 0))
      {
        cal_state = PH_CAL_DONE; 
        ms->ph_cal_stage = PH_CAL_DONE; // update global state for TFT handling
      }

      break;
    }

    // calibration done
    case PH_CAL_OK:
    {
      // only print once
      if (cal_wait == PH_CAL_OK_DLY)
      {
        printf ("#calibration done, writing parameters to EEPROM\n\n");
        ee_dump_phcal ();
      }

      // wait a bit for windows display on TFT
      if (!(cal_wait-- > 0))
      {
        cal_state = PH_CAL_DONE; 
        ms->ph_cal_stage = PH_CAL_DONE; // update global state for TFT handling
      }
      break;
    }

    // calibration done
    case PH_CAL_DONE:
    {
      // adjust state
      ph_cal_busy = 0;            // clear 'cal_busy' flag so push buttons are scanned again
      cal_wait    = 0;            // reset calibration wait counter
      first       = 1;            // reset filter speed-up flag

      // all LEDs off to be sure
      led_grn_off  ();            
      led_red_off  ();      

      // resume logging
      master_log_start ();        

      cal_state = PH_CAL_IDLE;
      ms->ph_cal_stage = PH_CAL_DONE; // update global state for TFT handling
      break;
    }
    
    default:
     break;
  }
}


// -- dumping & logging

// log pH sensor data to serial port
void ph_log_all (void)
{
  if (!ph_log)
    return;

  ph_dump_raw ();
  ph_dump ();
}

// start pH logging
void ph_log_start (void)
{
  ph_log = 1;
}

// stop pH logging
void ph_log_stop (void)
{
  ph_log = 0;
}
