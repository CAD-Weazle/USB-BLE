// file    : ec.c
// author  : rb
// purpose : SJW Generator (Single-Jolt Wave Generator) EC meter routines
// date    : 190502
// last    : 191123
//

#include "includes.h"

// storage excitation wave DAC/ADC
int16_t dac_tab[NUM_DOTS];                  // excitation sine wave for DAC
int16_t adc_tab[NUM_DOTS];                  // response from EC sensor ADC

uint16_t ec_ftab[EC_NUM_DEVICES][EC_FILTERDEPTH]; // storage for moving average filter

// EC calculation
uint16_t ec_raw[EC_NUM_DEVICES];            // raw data read from EC sensor
uint16_t ec_avg[EC_NUM_DEVICES];            // average value after filtering

uint32_t c_raw[EC_NUM_DEVICES];             // EC calculated [uS]
uint32_t c_adj[EC_NUM_DEVICES];             // EC adjusted for cell constant [uS]
uint32_t c_cell[EC_NUM_DEVICES];            // EC final result with added temperature compensation [uS]

// cell constant for TDS-NTC-38 EC probe
float c_const[EC_NUM_DEVICES] = {1.00000000, 1.00000000, 1.00000000};           

// flags
uint8_t ec_cal_busy = 0;                    // 0: normal operation / 1: calibration busy
uint8_t ec_log = 0;                         // log EC data to serial port
uint8_t first[EC_NUM_DEVICES] = {2, 2, 2};  // speed up filter flag

// init EC
void init_ec (void)
{
  // deselect all EC circuits
  ec_select (EC_MUX_HIZ);

  // apply power to EC driver & NTC
  ec_power_on ();

  // read calibration data from EEPROM
  for (int probe = 0; probe < EC_NUM_DEVICES; probe++)
    c_const[probe] = ee_get_eccal (probe);

  // fill sine table for excitation wave
  for (int i = 0; i < NUM_DOTS; i++)
    dac_tab[i] = (int16_t)(SINE_AMP * sin (2*PI*(float)i/NUM_DOTS));   

  // reset state <> needed?
  ms->ec_cal_stage = EC_CAL_IDLE;
}

// handle ec sensors
void ec_update (void)
{
  if (ec_cal_busy)
    ec_calibrate ();
  else
  {
    // read sensors & update global state
    ec_get ();
    ec_state_update ();
  }
}

// excite & read back EC probe
void ec_get (void)
{
  static uint8_t probe = 0;

  // make sure calibration flag is reset
  ms->ec_cal_busy = 0;

  // set mux to select EC channel
  ec_select (probe);

  // excite EC sensor
  ec_sjw (probe);

  // calculate response (Vtt [mV]) & filter
  ec_raw[probe] = ec_peak_detect ();           
  ec_avg[probe] = ec_filter (probe, ec_raw[probe]);

  // calculate EC cell conductivity [uS]
  c_raw[probe] = (ec_avg[probe] * 1000) / SINE_AMP;  

  // adjust for cell constant 
  c_adj[probe] = (uint32_t)(c_raw[probe] * c_const[probe]);

  // apply temperature compensation [note: C_25 = C_t / (1 + alfa*(t-25))]
//c_cell[probe] = (float)c_adj[probe] * (1 / (1 + EC_ALFA*((float)ntc_temp[probe]/100 - 25)));
  c_cell[probe] = (float)c_adj[probe] * (1 / (1 + EC_ALFA*((float)(ms->ntc[probe])/100 - 25)));

  // adjust EC sensor counter
  if (++probe >= EC_NUM_DEVICES)
    probe = 0;
}

// excite cell with single sine wave & read back response
void ec_sjw (uint8_t channel)
{
  for (int i = 0; i < NUM_DOTS; i++)
  {
    dac2_write (DAC_MAX/2 + dac_tab[i]);    // write to cell &
    adc_tab[i] = adc_read_ec (channel);     // read response

    // delay a bit
    usleep (EC_WAVE_DELAY);
  }
}

// determine min/max value EC response
uint16_t ec_peak_detect (void)
{
  uint16_t adc_max = 0;  
  uint16_t adc_min = 0x0fff;  

  // find min/max
  for (int i = 0; i < NUM_DOTS; i++)
  {
    if (adc_tab[i] >= adc_max)
      adc_max = adc_tab[i];

    if (adc_tab[i] <= adc_min)
      adc_min = adc_tab[i];
  }

  // calculate amplitude 
  return (adc_max - adc_min);
}

// naive moving average filter
uint16_t ec_filter (uint8_t channel, uint16_t dat)
{
  uint32_t sum = 0;

  // speed up filter 
  if (first[channel])
  {
    for (int i = 0; i < EC_FILTERDEPTH; i++)
      ec_ftab[channel][i] = dat;

    // delay a few samples to counter power-up transients
    first[channel] = first[channel] - 1;
  }
 
  for (int i = EC_FILTERDEPTH-1; i > 0; i--)
    ec_ftab[channel][i] = ec_ftab[channel][i-1];

  ec_ftab[channel][0] = dat;

  for (int i = 0; i < EC_FILTERDEPTH; i++)
    sum += ec_ftab[channel][i];

  return (uint16_t)(sum/EC_FILTERDEPTH);
}

// --calibration routines
uint8_t ec_cur_sensor;                      // sensor being calibrated

// start calibration process
void ec_cal_sensor (uint8_t probe)
{
  // sanity check
  if (probe >= EC_NUM_DEVICES)
    return;

  // check which sensor to calibrate & start calibration
  ec_cur_sensor = probe;
  ec_cal_busy = 1;

  // update global state
  ms->ec_cal_stage = EC_CAL_IDLE; // make sure state is reset <> needed?
  ms->ec_cal_probe = probe;       // set EC sensor number
  ms->ec_cal_start = 0;           // make sure calibration start flag is reset
  ms->ec_cal_busy  = 1;           // flag calibration busy, waiting for 'ec_cal_start' 
}

// calibrate EC sensor
void ec_calibrate (void)
{
  // flags
  static uint8_t  cal_state = EC_CAL_IDLE;  // FSM state
  static uint16_t cal_wait = 0;             // calibration waiting time counter
  
  // EC sensor calibration state machine
  switch (cal_state)
  {
    // -------- INITIAL PART ------------
    // check for calibration in progress, else bail
    case EC_CAL_IDLE:
    {
      led_red_on ();                    // signal calibration busy
      ec_log_stop ();                   // stop log output sunWeb
      ph_log_stop ();                   // stop log output pH sensors
      ec_log_stop ();                   // stop log output EC sensors
      master_log_stop ();               // stop all logging

      // wait for start signal TFT code (TFT button press)
      if (ms->ec_cal_start)
        cal_state = EC_CAL_START;

      break;
    }

    // signal user calibration has started
    case EC_CAL_START:
    {
      printf ("\n>EC sensor %d: start calibration\n", ec_cur_sensor+1);

      // set speed up filter flag
      first [ec_cur_sensor] = 1;

      cal_state = EC_CAL_INSERT;
      break;
    }


    // -------- EC SENSOR CALIBRATION PART ------------

    // put EC sensor in calibration liquid with EC 3.00 [mS]
    case EC_CAL_INSERT:
    {
      printf (">EC sensor %d: put sensor in 3.00 [mS] calibration solution & wait\n", ec_cur_sensor+1);
     
      // set mux to select EC channel & set flag
      ec_select (ec_cur_sensor);

      // update global state for TFT handling
      ms->ec_cal_stage = EC_CAL_BUSY;

      cal_state = EC_CAL_BUSY;
      break;
    }

    // & wait some time to reach equilibrium
    case EC_CAL_BUSY:
    {
      // signal calibration busy
      led_red_toggle ();

      // excite EC sensor
      ec_sjw (ec_cur_sensor);

      // calculate response & filter
      ec_raw[ec_cur_sensor] = ec_peak_detect ();                                // calculate Vtt
      ec_avg[ec_cur_sensor] = ec_filter (ec_cur_sensor, ec_raw[ec_cur_sensor]); // apply moving average filter

      // calculate EC cell conductivity [uS]
      c_raw[ec_cur_sensor] = (ec_avg[ec_cur_sensor] * 1000) / SINE_AMP;  

      // assume unity cell constant 
      c_adj[ec_cur_sensor] = (uint32_t)(c_raw[ec_cur_sensor] * 1.0000);

      // apply temperature compensation [note: C_25 = C_t / (1 + alfa*(t-25))]
    //c_cell[ec_cur_sensor] = (float)c_adj[ec_cur_sensor] * (1 / (1 + EC_ALFA*((float)ntc_temp[ec_cur_sensor]/100 - 25)));
      c_cell[ec_cur_sensor] = (float)c_adj[ec_cur_sensor] * (1 / (1 + EC_ALFA*((float)(ms->ntc[ec_cur_sensor])/100 - 25)));

      // count down waiting time & display raw / average ADC value
      if (!(cal_wait % 10))
        printf (">EC sensor %d: calibration busy, please wait: %2d - measured: %ld target: %d\n", ec_cur_sensor+1, (EC_CAL_WAIT - cal_wait)/10, c_cell[ec_cur_sensor], 3000);

      // wait some time for accurate calibration 
      if (cal_wait >= EC_CAL_WAIT)
      {
        float ec_cal_res = 3000.0/c_cell[ec_cur_sensor];

        // update EEPROM & adjust state
        printf ("\n>EC sensor %d calibration factor: %1.8f\n", ec_cur_sensor+1, ec_cal_res);      

        ee_set_eccal (ec_cur_sensor, ec_cal_res);
        c_const[ec_cur_sensor] = ec_cal_res;

        cal_wait = 0;       
        cal_state = EC_CAL_READY;

        // update global state for TFT handling
        ms->ec_cal_stage = EC_CAL_READY;
      }
      else
        cal_wait++;

      // update progress for TFT display
      ms->ec_cal_waittime = EC_CAL_WAIT;
      ms->ec_cal_progress = cal_wait;

      break;
    }

    // -------- FINAL PART ------------

    // calibration done
    case EC_CAL_READY:
    {
      // signal calibration ready & dump calibration data
      printf (">calibration EC sensor %d done\n", ec_cur_sensor);
      ee_dump_eccal ();

      // adjust state
      ec_cal_busy = 0;            // clear busy flag, commence normal operation
      cal_wait = 0;               // reset calibration wait counter

      // all LEDs off to be sure
      led_grn_off  ();            
      led_red_off  ();      

      // resume logging
      master_log_start ();

      // update global state for TFT handling
      ms->ec_cal_stage = EC_CAL_IDLE;

      cal_state = EC_CAL_IDLE; 
      break;
    }

    default:
      break;
  }
}


// -- analog mux routine

// select mux channel for DAC 
void ec_select (uint8_t channel)
{
  switch (channel)
  {
    case 0:
    {      
      // select EC0
      GPIOH->CLR = EC_SEL0;
      GPIOH->SET = EC_SEL1;
      break;
    }

    case 1:
    {
      // select EC1
      GPIOH->CLR = EC_SEL0;
      GPIOH->CLR = EC_SEL1;
      break;
    }

    case 2:
    {
      // select EC2
      GPIOH->SET = EC_SEL0;
      GPIOH->SET = EC_SEL1;
      break;
    }

    case 3:
    default: 
    {
      // deselect all
      GPIOH->SET = EC_SEL0;
      GPIOH->CLR = EC_SEL1;
    }
  }

  // relax a bit for mux switching noise
  usleep (200);
}


// -- dumping & logging

// dump EC data
void ec_dump (void)
{
  for (int i = 0; i < EC_NUM_DEVICES; i++)
  {
  //printf ("T: %d.%02d RAW: %d AVG: %d ", ntc_temp[i]/100, ntc_temp[i]%100, ec_raw[i], ec_avg[i]);
    printf ("T: %d.%02d RAW: %d AVG: %d ", (ms->ntc[i])/100, (ms->ntc[i])%100, ec_raw[i], ec_avg[i]);
    printf ("Craw[uS]: %ld ", c_raw[i]);
    printf ("Cadj[uS]: %ld ", c_adj[i]);
    printf ("Ccell[uS]: %ld  ", (uint32_t)c_cell[i]);
  }

  printf ("\n");
}

// dump DAC & ADC data
void ec_dump_tab (void)
{
  for (int i = 0; i < NUM_DOTS; i++)
    printf ("DAC: %d ADC: %d\n", dac_tab[i], adc_tab[i]);
}

// log EC sensor data to serial port
void ec_log_all (void)
{
  if (!ec_log)
    return;

  ec_dump ();
}

// start EC logging
void ec_log_start (void)
{
  ec_log = 1;
}

// stop EC logging
void ec_log_stop (void)
{
  ec_log = 0;
}

// update SunWeb Master state 
void ec_state_update (void)
{
  for (int i = 0; i < EC_NUM_DEVICES; i++)
    ms->ec[i]  = (uint16_t)c_cell[i];
}
