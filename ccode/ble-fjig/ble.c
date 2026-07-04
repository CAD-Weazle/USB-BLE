// file    : ble.c
// author  : rb
// purpose : Feasycom FSC-BT630 Bluetooth Low Energy (BLE) Module interface - BLE Peripheral Roll
// date    : 191014
// last    : 220220
//
// note    : compile with 'ble_echo = 1' to set Module name with 'name <some_name>' command
//           compile with 'ble_echo = 0' for normal operation
//
// note    : Advertising Interval >= 800ms severly hampers connection speed

#include "includes.h"

#define BLE_DEBUG                      0    // debug output on/off
#define BLE_ERROR                      0    // error output on/off

// globals
char    ble_name[16] = "FJIG";              // default BLE Jig name

uint8_t ble_buf_len;                        // input buffer (from BLE module to ARM)
char    ble_buf[BLE_INBUF_LEN];             // input buffer length

uint8_t ble_echo = 0;                       // set BLE operation mode   - 0: normal operation / 1: setup mode
uint8_t ble_log  = 0;                       // dump BLE module response - 0: no output / 1: dump log to serial port

// set up BLE modules
void init_ble (void)
{
  // reset BLE module
  ble_reset ();

  // relax a bit
  msleep (500);

  // get BLE Module name
  ble_get_name ();
  msleep (1000);

  // set BLE Advertising Interval [ms]
  ble_set_advin (750);  // 750 seems rather ok, gives low latency connections
  msleep (1000);
}

// handle BLE traffic
void ble_do (void)
{ 
  static uint16_t ble_state = BLE_POWER_DOWN;
  static uint32_t cnt = 0;

  // bail when in setup mode
  if (ble_echo)
    return;

  switch (ble_state)
  {
    case BLE_POWER_DOWN:
    {
      ble_state = BLE_POWER_UP;

      // enable BLE Low Power Mode
      ble_enter_lpm ();

      // enter STM32 Stop mode, wait for wake-up event
      power_down ();
      // fall thru (needed here! (???))
    }
  
    case BLE_POWER_UP:
    {
      // leave STM32 Stop mode, triggered by wake-up event
      power_up ();

      // signal active state
    //led_red_flash (1);

      ble_state = BLE_READ_SENSORS;
      break;
    }

    case BLE_READ_SENSORS:
    {
      // enable power light sensor & SHT15
      sensor_power_enable ();
      debug (BLE_DEBUG, "#sensor_power_enable: done\n");
	  
      // read supply voltages & light sensor
      adc_update ();
      debug (BLE_DEBUG, "#adc_update: done\n");
	  
      // read temperature & humidity SHT15
      sht15_update ();
      debug (BLE_DEBUG, "#sht15_update: done\n");
	  
      // disable power light sensor & SHT15
      sensor_power_disable ();
      debug (BLE_DEBUG, "#sensor_power_disable: done\n");
	  
      ble_state = BLE_SEND_DATA;
      break;
    }

    case BLE_SEND_DATA:
    {
      // write Module name to BLE Module
      printf3 ("%s: ", ble_name);  
      debug (BLE_DEBUG, "%s: ", ble_name);
	  
      // write packet counter to BLE module
      printf3 ("%d ", cnt);  
      debug (BLE_DEBUG, "%d\n", cnt);
	  
      // write temperature & humidity to BLE module
      sht15_dump_ble ();
	  
      // write voltages to BLE module
      adc_dump_ble ();

      // update packet counter
      cnt++;

      ble_state = BLE_RESET;
      break;
    }

    case BLE_RESET:
    {
      // hardware reset BLE module
      ble_reset ();    

      ble_state = BLE_POWER_DOWN;
      break;
    }

    default:
      break;
  }
}

// send AT command
void ble_at (void)
{
  printf3 ("AT\n");
  debug (BLE_DEBUG, "#send AT command\n");
}

// hardware reset BLE module
void ble_reset (void)
{
  // apply reset pulse
  gpio_clr (GPIOB, BLE_RESET_L);
  msleep (50);
  gpio_set (GPIOB, BLE_RESET_L);

  // relax a bit
  msleep (10);
}


// enter Low Power Mode, non-volatile state
// note: module enters LPM after ~5 seconds, wake up UART first with single 'AT' command which has no response
void ble_enter_lpm (void)
{
  printf3 ("AT+LPM=1\n");
  debug (BLE_DEBUG, "#BLE enter Low Power Mode\n");
}

// leave Low Power Mode, non-volatile state
// note: constant higher current consumption, UART active permanently
void ble_leave_lpm (void)
{
  printf3 ("AT+LPM=0\n");
  debug (BLE_DEBUG, "#BLE leave Low Power Mode\n");
}

// enter command mode - not used
void ble_cmd_mode (void)
{
  // clear BT630 TRAN pin
  gpio_set (GPIOB, BLE_RXI_L);
  debug (BLE_DEBUG, "#BLE enter Command Mode\n");
}

// enter data mode - not used
void ble_data_mode (void)
{
  // clr BT630 TRAN pin
  gpio_clr (GPIOB, BLE_RXI_L);
  debug (BLE_DEBUG, "#BLE enter Data Mode\n");
}

// get BLE Module name
void ble_get_name (void)
{
  // send AT command to make sure BLE is active
  ble_at ();
  msleep (10);

  // get BLE Module name (name will be retreived by parser)
  printf3 ("AT+NAME\n");
  debug (BLE_DEBUG, "#send name command\n");
}

// dump BLE Module name
void ble_dump_name (void)
{
  printf2 ("#name: %s\n", ble_name);
}

// set BLE Advertising Interval
void ble_set_advin (uint16_t dat)
{
  // send AT command to make sure BLE is active
  ble_at ();
  msleep (10);

  printf3 ("AT+ADVIN=%d\n", dat);
  debug (BLE_DEBUG, "#send advin command\n");
}

// parse data from BLE Jigs 
void poll_ble (void)
{
  // read BLE module
  int c = uart3_getc ();

  // test for Rx buffer empty
  if (c < 0)
    return;

  // read data from BLE Jig
  if (c == '\n')                            // check for enter to end string
  {
    // null terminate buffer
    ble_buf[ble_buf_len] = '\0';            

    // send BLE Module response to serial port
    if ((ble_echo) || (ble_log))
      printf2 ("%s\n", ble_buf);

    // parse data from BLE Jig
    ble_parse_line (ble_buf, ble_buf_len);
   
    // reset pointer
    ble_buf_len = 0;               
  }
  else if (isprint(c) && ble_buf_len < BLE_INBUF_LEN && (c != '~'))
    ble_buf[ble_buf_len++] = c;
}

// handle BLE Module response
void ble_parse_line (char buf[], uint8_t len)
{
  int head = -1;
  int tail = -1;
  char cmd[16] = {0};
  int j = 0;

  // bail on empty line
  if (!len)
    return;

  // parse 'OK' response
  if (!strcmp (buf, "OK"))
    return;
 
  // parse other responses
  for (int i = 0; i < len; i++)
  {
    if (buf[i] == '+')
      head = i;

    if (buf[i] == '=')
      tail = i;
  }

  // bail on faulty response
  if ((head < 0) || (tail < 0))
    return;

  // get command
  for (int i = head+1; i < (tail-head); i++)
    cmd[j++] = buf[i];

  // handle 'NAME' command response
  j = 0;

  if (!strcmp (cmd, "NAME"))
  {
    // get BLE Module name
    for (int i = tail+1; i < len; i++)
      ble_name[j++] = buf[i];
  }
}










