// file    : ds3234.c
// author  : rb
// purpose : DS3234 Extremely Accurate RTC routines
// date    : 170920
// last    : 191208
//

#include "includes.h"

// current time & date globals
uint8_t time_hrs;
uint8_t time_min;
uint8_t time_sec;

uint8_t date_yrs;
uint8_t date_mnt;
uint8_t date_day;

uint8_t dec_hrs; 
uint8_t dec_min;
uint8_t dec_sec;

uint8_t do_clock = 0;

// set up DS3234
void init_ds3234 (void)
{
}

// get current time
void ds3234_update (void)
{
  // update time
  ds3234_get_time ();

  // update Master state
  ds3234_state_update ();  
  
  // sync SunWeb Devices at every hour
  ds3234_sync ();
}

// set current time
void ds3234_set_time (uint8_t hrs, uint8_t min, uint8_t sec)
{
  uint8_t dat[3];

  // sanity check
  if ((hrs > 23) || (min > 59) || (sec > 59))
   return;

  // convert to BCD
  dat[0] = (sec/10 << 4) | sec%10;
  dat[1] = (min/10 << 4) | min%10;
  dat[2] = (hrs/10 << 4) | hrs%10;

  // change SPI mode for DS3234
  spi3_set_mode (SPI_DS3234_MODE);

  // write time registers
  ds3234_sel ();

  spi3_write_byte (0x80);             // write start address time registers

  for (int i = 0; i < 3; i++)         // read all 'time' bytes 
    spi3_write_byte (dat[i]);

  ds3234_unsel ();

  // change SPI mode back for TFT
  spi3_set_mode (SPI_TFT_MODE);
}

// get current time
void ds3234_get_time (void)
{
  uint8_t dat[3];

  // change SPI mode for DS3234
  spi3_set_mode (0);

  // read time registers
  ds3234_sel ();

  spi3_write_byte (0x00);             // write start address time registers
  
  for (int i = 0; i < 3; i++)         // write new 'time' bytes 
    dat[i] = spi3_read_byte ();

  ds3234_unsel ();

  // change SPI mode back for TFT
  spi3_set_mode (1);

  // copy to globals
  time_sec = dat[0];
  time_min = dat[1];
  time_hrs = dat[2];

  // convert BCD to decimal
  dec_hrs = ((time_hrs & 0x30) >> 4)*10 + (time_hrs & 0x0f);
  dec_min = ((time_min & 0x70) >> 4)*10 + (time_min & 0x0f);
  dec_sec = ((time_sec & 0x70) >> 4)*10 + (time_sec & 0x0f);
}

// set current date
void ds3234_set_date (uint8_t yrs, uint8_t mnt, uint8_t day)
{
  uint8_t dat[3];

  // sanity check
  if ((mnt == 0) || (dat == 0))
    return;
  
  if ((yrs > 99) || (mnt > 12) || (day > 31))
   return;

  // convert to BCD
  dat[0] = (day/10 << 4) | day%10;
  dat[1] = (mnt/10 << 4) | mnt%10;
  dat[2] = (yrs/10 << 4) | yrs%10;

  // write time registers
  ds3234_sel ();

  spi3_write_byte (0x84);             // write start address date registers

  for (int i = 0; i < 3; i++)         // write new 'date' bytes 
    spi3_write_byte (dat[i]);

  ds3234_unsel ();
}

// get current date
void ds3234_get_date (void)
{
  uint8_t dat[3];

  ds3234_sel ();

  spi3_write_byte (0x04);             // write start address date registers
  
  for (int i = 0; i < 3; i++)         // read all 'date' bytes 
    dat[i] = spi3_read_byte ();

  ds3234_unsel ();

  // copy to globals
  date_day = dat[0];
  date_mnt = dat[1];
  date_yrs = dat[2];
}

// get current time & dump to UART
void ds3234_dump_time (uint8_t nl)
{
  ds3234_get_time ();

  printf ("%02d:%02d:%02d", dec_hrs, dec_min, dec_sec);

  if (nl)
    printf ("\n");
  else    
    printf ("  ");
}

// get current date & dump to UART
void ds3234_dump_date (void)
{
  ds3234_get_date ();

  printf ("date: %02d/%02d/%02d\n", (((date_yrs & 0xf0) >> 4) * 10) + (date_yrs & 0x0f),
                                    (((date_mnt & 0x10) >> 4) * 10) + (date_mnt & 0x0f),
                                    (((date_day & 0x30) >> 4) * 10) + (date_day & 0x0f));
}

// get current time and date & dump to UART
void ds3234_timestamp (void)
{
  ds3234_get_time ();
  ds3234_get_date ();

  printf ("time: %02d:%02d:%02d  ", (((time_hrs & 0x30) >> 4) * 10) + (time_hrs & 0x0f),
                                    (((time_min & 0x70) >> 4) * 10) + (time_min & 0x0f),
                                    (((time_sec & 0x70) >> 4) * 10) + (time_sec & 0x0f));

  printf ("date: %02d/%02d/%02d\n", (((date_yrs & 0xf0) >> 4) * 10) + (date_yrs & 0x0f),
                                    (((date_mnt & 0x10) >> 4) * 10) + (date_mnt & 0x0f),
                                    (((date_day & 0x30) >> 4) * 10) + (date_day & 0x0f));
}

// -- SunWeb routines

// sync RTC SunWeb Devices & time logging
void ds3234_sync (void)
{
/* note: clocks of SunWeb Devices are too inaccurate, sync every hour, not every day
  // check for midnight, sync all clocks at midnight
  if (ds3234_midnight ())
  {
    // broadcast CLOCKSYNC packet
    sw_sync_clock (SW_ID_BCASTLEDS); 
  }
*/
  // check for full hour, sync all SunWeb Devices' clocks (less deviation in RTC time)
  if (ds3234_newhour ())
  {
    sw_set_clock (SW_ID_BCASTLEDS);            
    sw_set_clock (SW_ID_BCASTHEAT);            
  }
}

// check for midnight (time = 00:00:00 or 00:00:01 - to counter out of sync clocks)
uint8_t ds3234_midnight (void)
{
  ds3234_get_time ();

  return (((time_hrs == 0) && (time_min == 0) && (time_sec == 0)) ||
          ((time_hrs == 0) && (time_min == 0) && (time_sec == 1))) 
          ? 1 : 0; 
}

// check for new hour (time = xx:00:00 or xx:00:01 - to counter out of sync clocks)
uint8_t ds3234_newhour (void)
{
  ds3234_get_time ();

  return (((time_min == 0) && (time_sec == 0)) ||
          ((time_min == 0) && (time_sec == 1))) 
          ? 1 : 0; 
}

// start logging time to terminal
void ds3234_start_log (void)
{
  do_clock = 1;
}

// stop logging time to terminal
void ds3234_stop_log (void)
{
  do_clock = 0;
}

// update SunWeb Master state 
void ds3234_state_update (void)
{
  // normal time format
  ms->hrs = dec_hrs;
  ms->min = dec_min;
  ms->sec = dec_sec;

  // time in raw [s] format
  ms->tsecs = dec_hrs*60*60 + dec_min*60 + dec_sec;
}


// dump DS3234 registers -- dev only
void ds3234_dump_regs (void)
{
  uint8_t dat[0x13];

  ds3234_sel ();

  spi3_write_byte (0x00);             // write start address registers
  
  for (int i = 0; i < 0x13; i++)      // read all bytes 
    dat[i] = spi3_read_byte ();

  ds3234_unsel ();

  dump_ascii (dat, 0x13);
}
