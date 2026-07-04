// file    : pump.c
// author  : rb
// purpose : water pump control
// date    : 190911
// last    : 200207

#include "includes.h"

#define PUMP_DEBUG                                0   // dump pump timer stats

// globals
uint8_t  do_pump = 0;               // pump control flag
uint32_t pump_secs;                 // pump time in [s]

// init water pumps
void init_pump (void)
{
}

// control water pumps
void pump_update (void)
{
  static uint8_t pump_dly = 0;
  static uint8_t pump_num = 0;

  // check pump start time
  pump_check ();

  // check flag, bail if cleared
  if (!do_pump)
    return;

  // drive single pump
  pump_go (pump_num, PUMP_TIME);

  // wait to switch to next pump
  if (pump_dly++ >= (PUMP_TIME + PUMP_INTERDELAY))
  {
    // check for all pumps done
    if (++pump_num >= PUMP_NUM_DEVICES)
    {
      pump_off (); 
      pump_num = 0;
    }

    pump_dly = 0;
  }
}

// drive water pump with PWM profile
void pump_go (uint8_t num, uint8_t delay)
{
  static uint8_t state = PUMP_START;
  static uint8_t cnt = 0;

  switch (state)
  {
    // ramp down pump speed
    case PUMP_START:
    {
      debug (PUMP_DEBUG, "#pump %d ramp down\n", num);

      for (int i = 0; i < (PUMP_PWM_INITIAL - PUMP_PWM_STEADY) + 1; i++)
      {
        pwm_set (num, PUMP_PWM_INITIAL - i);
      //msleep (5);
        msleep (10); // <> edit 200207
      }

      state = PUMP_BUSY;

      break;
    }

    // wait while filling reservoir
    case PUMP_BUSY:
    {
      debug (PUMP_DEBUG, "#pump %d pumping: %d\n", num, delay - cnt);

      if (++cnt > delay)
        state = PUMP_DONE;
        
      break;
    }

    // done pumping, let water flow back
    case PUMP_DONE:
    {
      debug (PUMP_DEBUG, "#pump %d stopped\n", num);

      pwm_set (num, 0);
      cnt = 0;
 
      state = PUMP_START;

      break;
    }

    default:
      break;
  }
}

// set pump control flag
void pump_on (void)
{
  // bail when busy
  if (do_pump)
    return;

  // set flags
  do_pump = 1;
  ts->watering = 1;

  // play sound
  eve_sound_on (EVE_WARBLE, EVE_MIDI_D6, PUMP_SOUND_VOLUME); 

  printf ("#pump on\n");
}

// clear pump control flag
void pump_off (void)
{
  // clear TFT flags
  do_pump = 0; 
  ts->watering = 0;

  // stop playing sound
  eve_sound_off ();

  printf ("#pump off\n");
}

// set start time pump action
void pump_set_start (uint32_t tsecs)
{
  // input data sanity check, bail on error
  if (tsecs > DAY_SECS)
    return;

  pump_secs = tsecs;
}

// get start time pump action
void pump_get_start (void)
{
  uint8_t pump_hrs = (uint8_t)(pump_secs/(60*60));
  uint8_t pump_min = (uint8_t)((pump_secs - (pump_hrs*60*60))/60);
  uint8_t pump_sec = 0;

  printf ("#pump starts at %02d:%02d:%02d (%ld %ld)\n", pump_hrs, pump_min, pump_sec, pump_secs, ms->tsecs);
}

// check for pump start time
void pump_check (void)
{
  // check day & time for watering
  if (ts->wtime_sel == ts->wtime_day)
    if ((ms->tsecs == ts->wtime_pos) || (ms->tsecs == ts->wtime_pos-1)) // against clock jitter
      pump_on ();

  // dev only
  debug (PUMP_DEBUG, "sel: %d len: %d day: %d pos: %ld - time: %ld\n", ts->wtime_sel, ts->wtime_len, 
                                                                       ts->wtime_day, ts->wtime_pos,
                                                                       ms->tsecs);
}







