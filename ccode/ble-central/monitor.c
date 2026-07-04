// file    : monitor.c
// author  : rb
// purpose : SunWeb Master monitor
// date    : 190917
// last    : 191219

#include "includes.h"

// logging flag
uint8_t do_log  = 0;

// log data dump interval [s]
uint16_t log_int = 10;  // chose divider of 60

// SunWeb Master state
m_state master_state;
m_state *ms = &master_state;

uint8_t log_go = 0;

void gaan_met_die_banaan (void)
{
  printf ("start bus\n");
  log_go = 1;
}

// update state SunWeb Master & SunWeb Devices
void monitor_update (void)
{
  static uint8_t dsec = 0;

  // handle SunWeb traffic - run the parser
  sw_update ();         

  // update SunWeb Master state - sensors & stuff
  ph_update     ();     // pH probes
  ec_update     ();     // EC probes
  ntc_update    ();     // NTC sensors of EC probes
  adc_update    ();     // pressure/PAR/ambient light
  ds3234_update ();     // RTC

  // update slow processes
  if (++dsec > 10)
  {
    sw_log_all ();      // log SunWeb Devices - poll all SunWeb Devices 
    pump_update ();     // water pumps
    scd30_update ();    // CO2/T/%RH

    dsec = 0;
  } 
}

// dump SunWeb Master & SunWeb Devices state to serial port
void monitor_dump (void)
{
  // bail when not logging
  if (!do_log)
    return;

  if (ms->sec%log_int)
    return;

  master_dump ();  // dump state SunWeb Master 
  sunweb_dump ();  // dump state SunWeb Devices
}

// dump SunWeb Master state
void master_dump (void)
{
  // dump time
  printf ("%02d:%02d:%02d ", ms->hrs, ms->min, ms->sec);

  // dump pH
  printf ("pH: ");

  for (int i = 0; i < PH_NUM_DEVICES; i++)
    printf ("%d.%02d ", ms->ph[i]/100, ms->ph[i]%100);

  // dump EC 
  printf ("EC: ");

  for (int i = 0; i < EC_NUM_DEVICES; i++)
    printf ("%d.%03d ", ms->ec[i]/1000, ms->ec[i]%1000);

  // dump temperature
  printf ("T: ");

  for (int i = 0; i < EC_NUM_DEVICES; i++)
    printf ("%d.%02d ", ms->ntc[i]/100, ms->ntc[i]%100);

  // dump light & pressure
  printf ("PAR: %d ", ms->par);
  printf ("amb: %d ", ms->amb);
  printf ("p: %d ", ms->prs);

  // dump CO2/T/RH%
  printf ("CO2: %3.2f ", ms->co2); 
  printf ("T: %2.2f "  , ms->temp);
  printf ("RH: %2.2f " , ms->humi);
  printf ("- ");
}

// start logging
void master_log_start (void)
{
  do_log = 1;
}

// stop logging
void master_log_stop (void)
{
  do_log = 0;
}

void master_log_interval (uint16_t dat)
{
  if (dat)
    log_int = dat;
}

