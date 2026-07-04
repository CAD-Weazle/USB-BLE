// file    : 25aa040.c
// author  : rb
// purpose : 25AA040 512x8 EEPROM SPI routines
// date    : 190730
// last    : 191210
//

#include "includes.h"

#define EE_DEBUG                0

// init EEPROM
void init_eeprom (void)
{
  // check EEPROM for valid pH/EC calibration data
  ee_init_phcal  ();
  ee_init_eccal  ();
  ee_init_ntccal ();

  // check EEPROM for valid TFT state
  ee_tft_init ();
}

// read buffer from EEPROM
void ee_read_buf (uint8_t *buf, uint16_t len, uint16_t adr)
{
  for (int i = 0; i < len; i++)
    buf[i] = ee_read_byte (adr+i);

  debug (EE_DEBUG, "#buffer read from EEPROM\n");
}

// write buffer to EEPROM
void ee_write_buf (uint8_t *buf, uint16_t len, uint16_t adr)
{
  for (int i = 0; i < len; i++)
    ee_write_byte (adr+i, buf[i]);

  debug (EE_DEBUG, "#buffer written to EEPROM\n");
}

// read byte from EEPROM
uint8_t ee_read_byte (uint16_t adr)
{
  uint8_t dat;

  // change SPI mode for EEPROM
  spi3_set_mode (SPI_EEPROM_MODE);

  // select EEPROM
  eeprom_sel ();

  // write command, addres & read data
  spi3_write_byte (EE_CMD_READ | ((uint8_t)(adr>>5) & EE_MSB));
  spi3_write_byte ((uint8_t)adr);
  dat = spi3_read_byte ();

  // deselect EEPROM
  eeprom_unsel ();

  // change SPI mode back for TFT
  spi3_set_mode (SPI_TFT_MODE);

  return (dat);
}

// read word from EEPROM
uint16_t ee_read_word (uint16_t adr)
{
  uint8_t hi, lo;

  // write hi & lo byte
  hi = ee_read_byte (adr);
  lo = ee_read_byte (adr+1);

  return ((uint16_t)hi << 8 | lo);
}

// read long from EEPROM
uint32_t ee_read_long (uint16_t adr)
{
  uint8_t  hi, m1, m2, lo;

  // write hi & lo byte
  hi = ee_read_byte (adr);
  m1 = ee_read_byte (adr+1);
  m2 = ee_read_byte (adr+2);
  lo = ee_read_byte (adr+3);

  return (((uint32_t)hi << 24) | ((uint32_t)m1 << 16) | ((uint32_t)m2 << 8) | lo);
}

// read float from EEPROM
float ee_read_float (uint16_t adr)
{
  uint8_t  hi, m1, m2, lo;

  union u_float
  {
    float f;
    uint32_t u;
  } x;
 
  // write bytes
  hi = ee_read_byte (adr);
  m1 = ee_read_byte (adr+1);
  m2 = ee_read_byte (adr+2);
  lo = ee_read_byte (adr+3);

  // assemble long & store in union
  x.u = ((uint32_t)hi << 24) |
        ((uint32_t)m1 << 16) |
        ((uint32_t)m2 <<  8) |
        ((uint32_t)lo <<  0);

  // return union float part
  return (x.f);
}

// write byte to EEPROM
void ee_write_byte (uint16_t adr, uint8_t dat)
{
  // enable EEPROM write
  ee_write_enable ();

  // change SPI mode for EEPROM
  spi3_set_mode (SPI_EEPROM_MODE);

  // select EEPROM
  eeprom_sel ();

  // write command, addres & data
  spi3_write_byte (EE_CMD_WRITE | ((uint8_t)(adr>>5) & EE_MSB));
  spi3_write_byte ((uint8_t)adr);
  spi3_write_byte (dat);

  // deselect EEPROM
  eeprom_unsel ();

  // change SPI mode back for TFT
  spi3_set_mode (SPI_TFT_MODE);

  // wait while busy
  while (ee_busy ())
    usleep (5);
}

// write word to EEPROM
void ee_write_word (uint16_t adr, uint16_t dat)
{ 
  // write hi & lo byte
  ee_write_byte (adr,   (uint8_t)(dat>>8));
  ee_write_byte (adr+1, (uint8_t)(dat & 0x00ff));
}

// write long to EEPROM
void ee_write_long (uint16_t adr, uint32_t dat)
{
  // write bytes
  ee_write_byte (adr,   (uint8_t)(dat>>24 & 0x000000ff));
  ee_write_byte (adr+1, (uint8_t)(dat>>16 & 0x000000ff));
  ee_write_byte (adr+2, (uint8_t)(dat>>8  & 0x000000ff));
  ee_write_byte (adr+3, (uint8_t)(dat>>0  & 0x000000ff));
}

// write float to EEPROM
void ee_write_float (uint16_t adr, float dat)
{
  union u_float
  {
    float f;
    uint32_t u;
  };

  union u_float x = {dat};

  ee_write_long (adr, x.u);
}

// check for write action busy
uint8_t ee_busy (void)
{
  uint8_t dat;

  // change SPI mode for EEPROM
  spi3_set_mode (SPI_EEPROM_MODE);

  // select EEPROM
  eeprom_sel ();

  // write command & read STATUS register
  spi3_write_byte (EE_CMD_RDSR);
  dat = spi3_read_byte ();

  // deselect EEPROM
  eeprom_unsel ();

  // change SPI mode back for TFT
  spi3_set_mode (SPI_TFT_MODE);

  return (dat & EE_STATUS_WIP);
}

// EEPROM write enable
void ee_write_enable (void)
{
  // change SPI mode for EEPROM
  spi3_set_mode (SPI_EEPROM_MODE);

  // select EEPROM
  eeprom_sel ();

  // write command
  spi3_write_byte (EE_CMD_WREN);

  // deselect EEPROM
  eeprom_unsel ();

  // change SPI mode back for TFT
  spi3_set_mode (SPI_TFT_MODE);
}

// EEPROM write disable
void ee_write_disable (void)
{
  // change SPI mode for EEPROM
  spi3_set_mode (SPI_EEPROM_MODE);

  // select EEPROM
  eeprom_sel ();

  // write command
  spi3_write_byte (EE_CMD_WRDI);

  // deselect EEPROM
  eeprom_unsel ();

  // change SPI mode back for TFT
  spi3_set_mode (SPI_TFT_MODE);
}

// wipe entire EEPROM contents
void ee_wipe (void)
{
  for (int i = 0; i < EE_SIZE; i++)
    ee_write_byte (i, 0x00);
}

// dump entire EEPROM contents
void ee_dump (void)
{
  uint8_t ee_dat[EE_SIZE];

  for (int i = 0; i < EE_SIZE; i++)
    ee_dat[i] = ee_read_byte (i);

  dump_ascii (ee_dat, EE_SIZE/2);
}


// *** bus map part 
// current mapping: 0x11/0x21 0x12/0x22  0x13/0x23, 0xff/0x24

// storage for SimSun & HotBed Bus ID's read from EEPROM
uint8_t bus_map[SW_NUM_DEVICES];

// retrieve busmap from external EEPROM
void ee_read_busmap (void)
{
  // dummy read to get EEPROM going (get out of stand-by?)
  ee_read_byte (0);

  for (int i = 0; i < SW_NUM_DEVICES; i++)
  {
    bus_map[i] = ee_read_byte (EE_BUSMAP_BASE + i);
  }
}

// dump busmap 
void ee_dump_busmap (void)
{
  uint8_t i;
  uint8_t simsun_id, hotbed_id;

  for (i = 0; i < SW_NUM_DEVICES; i += 2)
  {
    simsun_id = (uint8_t)bus_map[i];        // get SimSun ID
    hotbed_id = (uint8_t)bus_map[i+1];      // get HotBed ID

    printf ("GroBox_%d - SimSun ID: 0x%02x  HotBed ID: 0x%02x\n", i/2 + 1, simsun_id, hotbed_id);
  }
}

// add SunWeb Device to busmap
// (note: 1 <= num < SW_NUM_DEVICES/2)
void ee_fill_busmap (uint8_t num, uint8_t id)
{
  // sanity check, bail on error
  if ((num == 0) || (num > SW_NUM_DEVICES/2))
    return;

  // store Bus ID in external EEPROM
  if (id & SW_ID_BCASTLEDS)
  {
    // store SimSun ID
    ee_write_byte ((num-1)*2, id);
    printf ("GroBox %d: SimSun 0x%02x added\n", num, id);
  }
  else if (id & SW_ID_BCASTHEAT)
  {
    // store HotBed ID
    ee_write_byte ((num-1)*2 + 1, id);
    printf ("GroBox %d: HotBed 0x%02x added\n", num, id);
  }
  else
  {
    printf ("busmap error, wrong Bus ID: 0x%02x\n", id);
    return;
  }

  // update busmap
  ee_read_busmap ();
}

// get SunWeb Device IDs
uint8_t ee_get_busmap (uint8_t num, uint8_t id)
{
  // sanity check
  if (num > SW_NUM_DEVICES/2)
    return (0xff);      // return 'no Device present'

  if (id & SW_ID_BCASTLEDS)
  {
    // return SimSun Device ID
    return (bus_map[num*2]);
  }
  else if (id & SW_ID_BCASTHEAT)
  {
    // return HotBed Device ID
    return (bus_map[num*2 + 1]);
  }

  // handle fall tru
  return (0xff);        // return 'no Device present'
}


// *** pH calibration part

// write default pH calibration data to empty EEPROM
void ee_init_phcal (void)
{
  uint16_t dat;

  // get arbitrary pH calibration point to check for empty EEPROM
  dat = ee_get_phcal (PH_SENSOR1, PH4_CALPOINT); // note: PH_SENSOR1 = 0

  // when EEPROM is wiped, write default pH calibration values
  if (dat == 0xffff)
  {
    printf ("#pH calibration data missing, restore default values\n");

    ee_set_phcal (PH_SENSOR1, PH4_CALPOINT, 1701);
    ee_set_phcal (PH_SENSOR2, PH4_CALPOINT, 1709);
    ee_set_phcal (PH_SENSOR3, PH4_CALPOINT, 1703);

    ee_set_phcal (PH_SENSOR1, PH7_CALPOINT, 2054);
    ee_set_phcal (PH_SENSOR2, PH7_CALPOINT, 2058);
    ee_set_phcal (PH_SENSOR3, PH7_CALPOINT, 2052);
  }
  else
    printf ("#pH calibration found\n");
}

// write pH calibration data to EEPROM
void ee_set_phcal (uint8_t sensor, uint8_t cal_point, uint16_t cal_dat)
{
  if (cal_point == PH4_CALPOINT)
  {
    ee_write_word (PH_CALDAT_BASE + sensor*2*sizeof(uint16_t), cal_dat);
  }
  else if (cal_point == PH7_CALPOINT)
  {
    ee_write_word (PH_CALDAT_BASE + (sensor*2*sizeof(uint16_t) + sizeof(uint16_t)), cal_dat);
  }
  else
    return ;
}

// read pH calibration data from EEPROM
uint16_t ee_get_phcal (uint8_t sensor, uint8_t cal_point)
{
  if (cal_point == PH4_CALPOINT)
  {
    debug (EE_DEBUG, "#from EEPROM: pH sensor[%d]_%d: %d\n", sensor, cal_point, ee_read_word (PH_CALDAT_BASE + sensor*4));
    return (ee_read_word (PH_CALDAT_BASE + sensor*2*sizeof(uint16_t)));
  }
  else if (cal_point == PH7_CALPOINT)
  {
    debug (EE_DEBUG, "#from EEPROM: pH sensor[%d]_%d: %d\n", sensor, cal_point, ee_read_word (PH_CALDAT_BASE + (sensor*4 + 2)));
    return (ee_read_word (PH_CALDAT_BASE + (sensor*2*sizeof(uint16_t)) + sizeof(uint16_t)));
  }
  else
    return 0;
}

// dump pH calibration data 
void ee_dump_phcal (void)
{
  for (int i = 0; i < PH_NUM_DEVICES*4; i += 4)
  {
    printf ("from EEPROM: pH sensor[%d]_4: %d\n", i/4, ee_read_word (PH_CALDAT_BASE + i));
    printf ("from EEPROM: pH sensor[%d]_7: %d\n", i/4, ee_read_word (PH_CALDAT_BASE + i+2));   
  }
}


// ***EC calibration part

// write default EC calibration data to empty EEPROM
void ee_init_eccal (void)
{
  float dat;

  // get arbitrary EC calibration point to check for empty EEPROM
  dat = ee_get_eccal (EC_SENSOR1); // note: EC_SENSOR1 = 0

  // when EEPROM is wiped, write default EC calibration values
  if (dat == 0.0)
  {
    printf ("#EC calibration data missing, restore default values\n");

    ee_set_eccal (EC_SENSOR1, 1.00000000);
    ee_set_eccal (EC_SENSOR2, 1.00000000);
    ee_set_eccal (EC_SENSOR3, 1.00000000);
  }
  else
    printf ("#EC calibration found\n");
}

// write EC calibration data to EEPROM
void ee_set_eccal (uint8_t sensor, float cal_dat)
{
  ee_write_float (EC_CALDAT_BASE + sensor*4, cal_dat);
}

// read EC calibration data from EEPROM
float ee_get_eccal (uint8_t sensor)
{
  debug (EE_DEBUG, "#from EEPROM: EC sensor[%d]  : %1.8f\n", sensor, ee_read_float (EC_CALDAT_BASE + sensor*sizeof(float)));
  return (ee_read_float (EC_CALDAT_BASE + sensor*sizeof(float)));
}

// dump EC calibration data 
void ee_dump_eccal (void)
{
  for (int i = 0; i < EC_NUM_DEVICES; i++)
  {
    printf ("from EEPROM: EC sensor[%d]: %1.8f\n", i, ee_read_float (EC_CALDAT_BASE + i*sizeof(float)));
  }
}


// *** NTC offset error part (note: NTC inside EC sensor)

// write default NTC calibration data to empty EEPROM
void ee_init_ntccal (void)
{
  int16_t dat;

  // get arbitrary NTC calibration point to check for empty EEPROM
  dat = ee_get_eccal (NTC_SENSOR1); // note: NTC_SENSOR1 = 0

  // when EEPROM is wiped, write default NTC calibration values
  if (dat == 0xffff)
  {
    printf ("#NTC calibration data missing, restore default values\n");

    ee_set_ntccal (NTC_SENSOR1, -65);
    ee_set_ntccal (NTC_SENSOR2, -58);
    ee_set_ntccal (NTC_SENSOR3, -71);
  }
  else
    printf ("#NTC calibration found\n");
}

// write NTC calibration data to EEPROM
void ee_set_ntccal (uint8_t sensor, int16_t cal_dat)
{
  ee_write_word (NTC_CALDAT_BASE + sensor*sizeof(int16_t), cal_dat);
}

// read NTC calibration data from EEPROM
int16_t ee_get_ntccal (uint8_t sensor)
{
  debug (EE_DEBUG, "#from EEPROM: NTC sensor[%d] : %d\n", sensor, (int16_t)ee_read_word (NTC_CALDAT_BASE + sensor*sizeof(int16_t)));
  return ((int16_t)ee_read_word (NTC_CALDAT_BASE + sensor*sizeof(int16_t)));
}

// dump NTC calibration data 
void ee_dump_ntccal (void)
{
  for (int i = 0; i < NTC_NUM_DEVICES; i++)
  {
    printf ("from EEPROM: NTC sensor[%d]: %d\n", i, (int16_t)ee_read_word (NTC_CALDAT_BASE + i*sizeof(int16_t)));
  }
}


// *** TFT part

// check for valid TFT state
void ee_tft_init (void)
{
  // get arbitrary data point to check for empty EEPROM
  uint16_t dat = ee_read_word (EE_TS_BASE);

  // when EEPROM empty fill with default TFT state
  if (dat == 0xffff)
  {
    // write default TFT settings to EEPROM
    ts->panel = TFT_MONITOR;

    ts->watering = 0;

    ts->rlen[0] = 50;  
    ts->rlen[1] = 50;  
    ts->rlen[2] = 50;  

    ts->ilen[0] = 100;  
    ts->ilen[1] = 100;  
    ts->ilen[2] = 100;  

    ts->tlen[0] = 25200;  
    ts->tlen[1] = 68400;  

    ts->store = 0;  

    ts->wtime_sel = 0;  
    ts->wtime_len = 0;  
    ts->wtime_day = 0;  
    ts->wtime_pos = 7200;  

    ts->pid_setpoint[0] = 34832;
    ts->pid_setpoint[1] = 34832;
    ts->pid_setpoint[2] = 34832;

    ts->pid_active[0] = 0;
    ts->pid_active[1] = 0;
    ts->pid_active[2] = 0;

    ts->bottom_leds[0] = 0;
    ts->bottom_leds[1] = 0;
    ts->bottom_leds[2] = 0;

    ts->fan_speed[0] = 0;
    ts->fan_speed[1] = 0;
    ts->fan_speed[2] = 0;

    ts->led_new[0] = 0;
    ts->led_new[1] = 0;
    ts->led_new[2] = 0;

    ts->fan_new[0] = 0;
    ts->fan_new[1] = 0;
    ts->fan_new[2] = 0;

    ee_tft_write ();
  }
  else
    printf ("#TFT state found\n");
}

// get TFT state from EEPROM
void ee_tft_read (void)
{
  ee_read_buf (ts_p, sizeof (t_state), EE_TS_BASE);

  debug (EE_DEBUG, "#read TFT settings from EEPROM\n");
}   

// write TFT state to EEPROM
void ee_tft_write (void)
{ 
  // reset EEPROM store flag 
  ts->store = 0;            

  ee_write_buf (ts_p, sizeof (t_state), EE_TS_BASE);

  debug (EE_DEBUG, "write TFT settings to EEPROM\n");
}

// dump TFT state read from EEPROM
void ee_tft_dump (void)
{
  uint8_t buf[sizeof(t_state)];

  for (int i = 0; i < sizeof(t_state); i++)
    buf[i] = ee_read_byte (EE_TS_BASE+i);

  dump_ascii (buf, sizeof (t_state));
}







