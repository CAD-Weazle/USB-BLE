// file    : scd30.c
// author  : rb
// purpose : routines for Sensirion SCD30 CO2 sensor
// date    : 190702
// last    : 191210
//

#include "includes.h"

// globals
uint8_t tx_dat[32];                    // Tx data for SDC30
uint8_t tx_len;                        // number of Tx bytes
uint8_t rx_dat[32];                    // Rx data from SDC30
uint8_t rx_len;                        // number of Rx bytes

// SCD30 data
float scd30_co2;                       // CO2 level [ppm]
float scd30_temp;                      // temperature [oC]
float scd30_humi;                      // humidity [%RH]

float scd30_ftab[SCD30_FILTERDEPTH];   // storage for moving average filter

// init SCD30 sensor
void init_scd30 (void)
{
//scd30_setoffset (0);    // doesn't seem to do anything
  scd30_setoffset (165);  // doesn't seem to do anything
  scd30_startmeasure ();
}

// get CO2/T/%RH
void scd30_update (void)
{
  if (scd30_getready ())
  {
    if (scd30_getmeasure ())
      scd30_state_update ();
  }
}

// reset SDC30
void scd30_reset (void)
{
  scd30_command (SCD30_RESET, NULL, 0, NULL, 0);
}

// get SDC30 ID 
uint16_t scd30_getid (void)
{
  scd30_command (SCD30_GETID, NULL, 0, rx_dat, 3);

  return (((uint16_t)rx_dat[0] << 8) | rx_dat[1]);
}

// get SDC30 status
uint16_t scd30_getready (void)
{
  scd30_command (SCD30_GETREADY, NULL, 0, rx_dat, 3);

  return (((uint16_t)rx_dat[0] << 8) | rx_dat[1]);
}

// start SDC30 continuous measurement
void scd30_startmeasure (void)
{
  uint8_t sdat [2];

  sdat[0] = 0x00; // pressure adjust (0 [mbar] here)
  sdat[1] = 0x00;

  scd30_command (SCD30_STARTMEASURE, sdat, 2, NULL, 0);
}

// stop SDC30 continuous measurement
void scd30_stopmeasure (void)
{
  scd30_command (SCD30_STOPMEASURE, NULL, 0, NULL, 0);
}

// get SDC30 measurement
uint8_t scd30_getmeasure (void)
{
  // SDC30 response is in Big Endian float notation, needs some manipulation
  union
  {
    uint8_t byte[4];  // <- write SDC30 response here in reverse byte order
    float   f;        // -> get float here
  } c, t, h;

  // send command & retrieve response
  if (scd30_command (SCD30_GETMEASURE, NULL, 0, rx_dat, 18))
  {
    // convert received bytes to float
    c.byte[3] = rx_dat[ 0];
    c.byte[2] = rx_dat[ 1];
    c.byte[1] = rx_dat[ 3];
    c.byte[0] = rx_dat[ 4];

    t.byte[3] = rx_dat[ 6];
    t.byte[2] = rx_dat[ 7];
    t.byte[1] = rx_dat[ 9];
    t.byte[0] = rx_dat[10];

    h.byte[3] = rx_dat[12];
    h.byte[2] = rx_dat[13];
    h.byte[1] = rx_dat[15];
    h.byte[0] = rx_dat[16];

    scd30_co2  = c.f;
    scd30_temp = scd30_filter (t.f) - 1.50;  // <> patch
    scd30_humi = h.f;

    return 1;
  }
  else
    return 0;
}

// naive moving average filter
float scd30_filter (float dat)
{
           float sum = 0;
  static uint8_t first =1;

  // speed up filter 
  if (first)
  {
    for (int i = 0; i < SCD30_FILTERDEPTH; i++)
      scd30_ftab [i] = dat;

    first = 0;
  }
  
  // shift historical data in array & add new data point
  for (int i = SCD30_FILTERDEPTH-1; i > 0; i--)
    scd30_ftab[i] = scd30_ftab[i-1];

  scd30_ftab[0] = dat;

  // calculate average of all data points
  for (int i = 0; i < SCD30_FILTERDEPTH; i++)
    sum += scd30_ftab[i];

  return (sum/SCD30_FILTERDEPTH);
}

// set SDC30 continuous measurement interval
void scd30_setinterval (uint16_t interval)
{
  uint8_t sdat [2];

  // sanity check, range [2..1800] seconds
  if ((interval < 2) || (interval > 1800))
   return;

  // assemble Tx data 
  sdat[0] = (uint8_t)((interval & 0xff00) >> 8); 
  sdat[1] = (uint8_t) (interval & 0x00ff);

  scd30_command (SCD30_SETINTERVAL, sdat, 2, NULL, 0);
}

// set SDC30 temperature offset [oC*100]
void scd30_setoffset (uint16_t offset)
{
  uint8_t sdat [2];

  // assemble Tx data 
  sdat[0] = (uint8_t)((offset & 0xff00) >> 8); 
  sdat[1] = (uint8_t) (offset & 0x00ff);

  scd30_command (SCD30_SETOFFSET, sdat, 2, NULL, 0);
}

// get SDC30 temperature offset [oC*100]
void scd30_getoffset (void)
{
  // send command
  scd30_command (SCD30_SETOFFSET, NULL, 0, rx_dat, 3);

  printf ("offset: %d\n", ((uint16_t)rx_dat[0] << 8) + rx_dat[1]);
}

// set SDC30 altitude compensation [m] above sea
void scd30_setaltitude (uint16_t altitude)
{
  uint8_t sdat [2];

  // assemble Tx data 
  sdat[0] = (uint8_t)((altitude & 0xff00) >> 8); 
  sdat[1] = (uint8_t) (altitude & 0x00ff);

  scd30_command (SCD30_SETALTITUDE, sdat, 2, NULL, 0);
}

// send SDC30 command & collect response
uint8_t scd30_command (uint16_t cmd, uint8_t sdat[], uint8_t slen, uint8_t rdat[], uint8_t rlen)
{
  uint8_t nan = 0;

  // add Command to Tx packet
  tx_dat[0] = (uint8_t)((cmd & 0xff00) >> 8);
  tx_dat[1] = (uint8_t) (cmd & 0x00ff);

  // increment with Command length
  tx_len = 2;

  // add payload to Tx packet
  if (slen)
  {
    for (int i = 0; i < slen; i++)
    {
      tx_dat[tx_len++] = sdat[i];

      // calculate CRC every two bytes & insert in Tx packet
      if ((i%2) != 0)
      {
        tx_dat[tx_len++] = scd30_generate_crc (&sdat[i-1], 2);
      }
    }
  }

  // send Tx packet
  i2c2_write (SCD30_WADR, tx_dat, tx_len);

  // retrieve Rx packet
  if (rlen)
  {
    rx_len = rlen;

    i2c2_read (SCD30_RADR, rx_dat, rx_len);

    // check for NaN response in Rx packet
    // NaN packet: ffff acff ffac ffff acff ffac ffff acff ffac
    for (int i = 0; i < rlen; i++)
    {
      if (rx_dat[i] == 0xff)
      {
        if (++nan >= 12)
        {
        //printf ("#NaN: bail\n");
          return 0;
        }
      }
    }

    // check CRC Rx packet
    for (int i = 0; i < rlen; i++)
    {
      // check 2-byte CRC every three bytes
      if ((i%3) == 0) 
      {
        // check if CRC(byte[n+0], byte[n+1]) == byte[n+2]
        if (rx_dat[i+2] != scd30_generate_crc (&rx_dat[i], 2))
        {
        //printf ("#CRC ERROR: bail\n");
          return 0;
        }
      }
    }
  }

  return 1;
}

// generate CRC8 checksum
uint8_t scd30_generate_crc (uint8_t dat[], uint8_t num) 
{
  uint8_t crc = SCD30_CRC8_INIT;
  uint8_t crc_bit;

  // calculates 8-bit checksum with given polynomial
  for (int i = 0; i < num; ++i) 
  {
    crc ^= (dat[i]);

    for (crc_bit = 8; crc_bit > 0; --crc_bit) 
    {
      if (crc & 0x80)
        crc = (crc << 1) ^ SCD30_CRC8_POLYNOMIAL;
      else
        crc = (crc << 1);
    }
  }

  return crc;
}

// dump SCD30 sensor data
void scd30_log_all (void)
{
  printf ("CO2: %3.2f  temp: %2.2f  humi: %2.2f\n", scd30_co2, scd30_temp, scd30_humi);
}

// update SunWeb Master state 
void scd30_state_update (void)
{
  ms->co2  = scd30_co2;
  ms->temp = scd30_temp;
  ms->humi = scd30_humi;
}

