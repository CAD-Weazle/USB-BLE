// file    : sht15.c
// author  : ao/rb
// purpose : Sensirion SHT15 routines
// board   : NFC Log - LPC1227 ARM Cortex M0
// date    : 150104
// last    : 200320
//

#include "includes.h"

// globals
float sht_temp;
float sht_humi;

// setup SHT15 sensor
void init_sht15 (void)
{
  sht15_reset ();
  sht15_start ();
  sht15_write (SHT15_STAT_WR);    // write status
  sht15_write (SHT15_HI_RES);     // 14-bit %RH / 12-bit T resolution
  sht15_stop();
}

// read temperature & humidity
void sht15_update (void)
{
  sht_temp = sht15_get_t (); // read first, temperature needed for humidity calculation
  sht_humi = sht15_get_h ();
}

// read temperature
float sht15_get_t (void)
{
  uint16_t raw = sht15_measure (SHT15_TEMP);

  // time-out occured, signal error
  if (raw == 0)
    return (99.99);  

  // calculate temperature & adjust error offset
  return (-40.1 + (float)raw/100 - 0.1); 
}

// read humidity
float sht15_get_h (void)
{
  uint16_t raw;
  float rh;

  // get raw temperature
  raw = sht15_measure (SHT15_RH);

  // time-out occured, signal error
  if (raw == 0)
   return (99.99);

  // RH = -4.0 + 0.0405*raw - 2.8000E-6*raw*raw -- V3 sensor
  rh = -4.0 + 0.0405*raw - 0.0000028*raw*raw;

  // adjust for temperature effect (note: read temperature first)
  rh = (sht_temp - 25.0) * (0.01 + 0.00008*raw) + rh;

  // sanity check
  if (rh < 0.0)
    rh = 0;

  if (rh > 100.00)
    rh = 100.0;

  return (rh);
}

// perform measurement
uint16_t sht15_measure (uint8_t cmd)
{
  uint8_t msb, lsb;
  uint16_t count;

  sht15_start ();
  sht15_write (cmd);

  // wait for HDAT low to signal measurement complete
  // note: can take up to 320ms (!!1!) for 14-bit measurements
  for (count = 0; GPIOB->IDR & (1 << SHT_DAT); count++)
  {
    msleep (1);

    if (count >= 500) // timeout
    {
      sht15_stop ();
      return (0); 
    }
  }

  msb = sht15_read (SHT15_ACK);   // read result MSB
  lsb = sht15_read (SHT15_ACK);   // read result LSB

  sht15_read (!SHT15_ACK);        // read CRC (dummy read)
  sht15_stop ();

  return (((uint16_t)msb << 8) + lsb);
}

// reset SHT15
void sht15_reset (void)
{
  HDAT_1 ();

  for (int i = 0; i < 9; i++)
  {
    HCLK_1 ();
    HCLK_0 ();
  }
}

// signal start of transmision
void sht15_start (void)
{
  // assumes HDAT=1, HCLK=0
  HCLK_1 ();
  HDAT_0 ();
  HCLK_0 ();
  HCLK_1 ();
  HDAT_1 ();
  HCLK_0 ();
}

// signal end of transmission (not used)
void sht15_stop (void)
{
}

// read byte from SHT15 sensor
uint8_t sht15_read (uint8_t ack)
{
  uint8_t i;
  uint8_t data = 0;

  HDAT_1 ();

  for (i = 8; i > 0; i--)
  {
    data <<= 1;

    HCLK_1 ();

    if (GPIOB->IDR & (1 << SHT_DAT))
      data++;

    HCLK_0 ();
  }

  if (ack)
    HDAT_0 ();
  else
    HDAT_1 ();

  HCLK_1 ();
  HCLK_0 ();
  HDAT_1 ();

  return (data);
}

// write byte to SHT15 sensor
uint8_t sht15_write (uint8_t data)
{
  uint8_t i;
  uint8_t ack;

  for (i = 8; i > 0; i--)
  {
    HCLK_0 ();

    if (data & 0x80)
      HDAT_1 ();
    else
      HDAT_0 ();

    HCLK_1 ();

    data <<= 1;
  }

  HCLK_0 ();
  HDAT_1 ();
  HCLK_1 ();

  ack = ((GPIOB->IDR & (1 << SHT_DAT)) == 0);

  HCLK_0 ();
  HDAT_1 ();

  return (ack);
}

// clock low
void HCLK_0 (void)
{
  gpio_clr (GPIOB, SHT_CLK);
  usleep (10);
}

// clock high
void HCLK_1 (void)
{
  gpio_set (GPIOB, SHT_CLK);
  usleep (10);
}

// data low (open drain pin)
void HDAT_0 (void)
{
  gpio_clr (GPIOB, SHT_DAT);
  usleep (10);
}

// data high (open drain output)
void HDAT_1 (void)
{
  gpio_set (GPIOB, SHT_DAT);
  usleep (10);
}

// dump current temperature & humidity
void sht15_dump (void)
{
  printf2 ("T: %2.2f %RH: %2.1f ", sht_temp, sht_humi);
}

// dump current temperature & humidity to BLE module
void sht15_dump_ble (void)
{
  printf3 ("T: %2.2f %RH: %2.1f ", sht_temp, sht_humi);
}




