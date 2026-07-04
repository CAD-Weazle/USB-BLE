// file    : ble.c
// author  : rb
// purpose : RN4871 Bluetooth Low Energy (BLE) module interface - BLE Peripheral Roll
// date    : 191014
// last    : 200417
//
// note    : print using color: printf2 ("\E[35m%c\E[30m", c);
//
// note    : remote mode from Central to Peripheral
//  1/ $$$              -- enter command mode
//  2/ SS,C0            -- enable Transparant UART service
//  3/ R,1              -- force reboot
//  4/ $$$              -- enter command mode
//  5/ F                -- scan devices, get <adr> -> %801F12BE1D35,0,RN4871-1D35,,B3%
//  6/ X                -- stop scanning
//  7/ C,0,801F12BE1D30 -- connect to remote device
//  8/ $$$              -- enter command mode
//  9/ B                -- bond
// 10/ !,1              -- enter remote mode, expect RMT> prompt
// 11/ SB,09            -- remote: set Baudrate to 9600
// 12/ S-,BLEJIG01      -- set name
// 13/ !,0              -- leave remote mode 
//
// note, status signals on pins, for testing only:
// sw,0a,07 -- 'Status 1' on pin 3
// sw,0b,08 -- 'Status 2' on pin 4
//
// st,0008,0018,0000,0200 == 8*1.25ms, 24*1.25ms, 0, 512*10ms
// == 10ms, 30ms, 0, 5.12s / min. interval, max. interval, latency, time-out
// 27.5ms inter-packet interval on scope
//
// note, adjust this for new modules:
// ss,c0                -- support device info and UART Transparent services 
//
// note, adjust this for new modules:
// sta,0640,0001,0640   -- fast advertising each 1s for 10.24s, then slow advertizing every 1s
// sta,0640,0001,3e80   -- fast advertising each 1s for 10.24s, then slow advertizing every 10s
//
// note, uitzoek0n:
// sr,0080              -- reboot after connection close
//

#include "includes.h"

#define BLE_DEBUG                      0    // debug output on/off
#define BLE_ERROR                      0    // error output on/off

// globals
uint8_t ble_buf_len;                        // input buffer (from BLE module to ARM)
char    ble_buf[BLE_INBUF_LEN];             // input bufer length

uint8_t ble_log = 0;                        // dump BLE module response, normal: 0 / test mode: 1

// set up RN4871 BLE modules
void init_ble (void)
{
  // set to Command Mode
  GPIOB->ODR |= BLE_MODE;

  // clear BLE_RXI_L
  GPIOB->ODR &= ~BLE_RXI_L;

  // reset BLE module
  ble_reset ();
}

// handle BLE traffic
void ble_do (void)
{ 
  static uint16_t ble_state = BLE_POWER_DOWN;
  static uint32_t cnt = 0;

  led_grn_flash (1);

  switch (ble_state)
  {
    case BLE_POWER_DOWN:
    {
      led_red_flash (1);

      ble_state = BLE_POWER_UP;

      // power down BLE module
      ble_power_down ();

      // enter Stop mode, wait for wake-up event
      power_down ();
      // fall thru (needed here! (???))
    }
  
    case BLE_POWER_UP:
    {
      // leave Stop mode, triggered by wake-up event
      power_up ();
      
      // power up BLE module
      ble_power_up ();

      ble_state = BLE_READ_SENSORS;
      break;
    }
  
    case BLE_READ_SENSORS:
    {
      // enable power light sensor & SHT15
      sensor_power_enable ();

      // read supply voltages & light sensor
      adc_update ();

      // read temperature & humidity SHT15
      sht15_update ();

      // disable power light sensor & SHT15
      sensor_power_disable ();

      ble_state = BLE_SEND_DATA;
      break;
    }

    case BLE_SEND_DATA:
    {
      // write data to serial port
      if (ble_log)
      {
        printf2 ("%d ", cnt);

        // write temperature & humidity to serial port
        sht15_dump ();

        // write voltages to serial port
        adc_dump ();
      }

      // write prompt & counter
      printf3 ("~");           
      printf3 ("%d ", cnt++);  

      // write temperature & humidity to BLE module
      sht15_dump_ble ();

      // write voltages to BLE module
      adc_dump_ble ();

      ble_state = BLE_CONNECTION_CLOSE;
      break;
    }

<> wait some secs here

    case BLE_CONNECTION_CLOSE:
    {
      // close connection with Central
      ble_connection_close ();

      ble_state = BLE_RESET;
      break;
    }

    case BLE_RESET:
    {
      // make sure state is reset <> needed?

      // set BLE_RXI_L to input (as per errata)
      GPIOB->MODER &= ~(0b11 << 24);
      msleep (500);

      // hardware reset BLE module
      ble_reset ();    

      // set BLE_RXI_L & set to output
      GPIOB->ODR   |= BLE_RXI_L;
      GPIOB->MODER |= (GPIO_MODE_OUT << 24);

      ble_state = BLE_POWER_DOWN;

      led_red_flash (1);
      break;
    }

    default:
      break;
  }
}

// hardware reset BLE module
void ble_reset (void)
{
  // apply reset pulse
  GPIOB->ODR &= ~BLE_RESET_L;
  msleep (50);
  GPIOB->ODR |= BLE_RESET_L;

  // wait for BLE MCU ready (p.35 DS)
  msleep (100);

  debug (BLE_DEBUG, "#BLE: hardware reset\n");
}

// enter command mode
void ble_cmd_mode (void)
{
  // relax a bit (p. 12 RN4871 User Manual)
  msleep (100);                        

  // send command
  printf3 ("$$$");

  // wait for response "CMD>" -- not working, use delay
//while (!ble_scan_response (BLE_RESPONSE_CMD))
//  ;  

  // don't scan response, just delay
  msleep (100);                        

  debug (BLE_DEBUG, "#BLE: enter command mode\n");
}

// enter data mode
void ble_data_mode (void)
{
  // send command
  printf3 ("---\r");

  // <> todo: wait for response
//while (!ble_scan_response (BLE_RESPONSE_DATA_MODE???))
//  ;  

  debug (BLE_DEBUG, "#BLE: enter data mode\n");
}

// enter low power mode
void ble_power_down (void)
{
  debug (BLE_DEBUG, "#BLE: enter low power mode\n");

  // enter Command mode
  ble_cmd_mode ();

  // shut down BLE module
  printf3 ("SO,1\r");

  // relax a bit, don't check response
  msleep (500);

  // set BLE_RXI_L
  GPIOB->ODR |= BLE_RXI_L;  // <> test
}

// leave low power mode
void ble_power_up (void)
{
  // clear BLE_RXI_L
  GPIOB->ODR &= ~BLE_RXI_L;
  
  debug (BLE_DEBUG, "#BLE: leave low power mode\n");
}

// close connection
void ble_connection_close (void)
{
  // enter Command mode
  ble_cmd_mode ();

  // close connection
  printf3 ("K,1\r");

  // wait for response "%DISCONNECT%" -- not working, use delay
//  while (!ble_scan_response (BLE_RESPONSE_DISCONNECT))
//    ;  

  // don't scan response, just delay
  msleep (100); 

  debug (BLE_DEBUG, "#BLE: close connection with Central\n");
}

// scan BLE response
uint8_t ble_scan_response (uint8_t resp)
{
  static uint16_t scan_state = 0;
  static uint16_t scan_count = 0;

  // get charachter from BLE module
  int c = uart3_getc ();

  // check for BLE module response character or timeout
  if (c < 0)
  {
    // no response character
    return 0;    
  }
  else if (scan_count++ > SCAN_TIMEOUT)
  {
    debug (BLE_ERROR, "#BLE: ERROR - module response timeout\n");

    // timeout: reset FSM state & go on anyway
    scan_state = 0;
    scan_count = 0;

    return 1;     
  }

  // jump in FSM 
  if (scan_state == 0)
  {
    if ((resp == BLE_RESPONSE_REBOOT)     || 
        (resp == BLE_RESPONSE_SCAN_START) || 
        (resp == BLE_RESPONSE_CONNECT)    ||
        (resp == BLE_RESPONSE_DISCONNECT) ||
        (resp == BLE_RESPONSE_STREAM_OPEN))
      scan_state = 1;
    else if (resp == BLE_RESPONSE_CMD)
      scan_state = 11;
    else if (resp == BLE_RESPONSE_AOK)
      scan_state = 21;
    else if (resp == BLE_RESPONSE_DATA_MODE)
      scan_state = 31;

    // reset timeout counter 
    scan_count = 0;
  }

  switch (scan_state)
  {
    case 1:
    {
      // scan for response of format: '%<response>%'
      if (c == '%')
      {
        // reset pointer
        ble_buf_len = 0;
        ble_buf[ble_buf_len++] = c;

        // switch state
        scan_state = 2;
      }

      break;   
    }

    // read rest of response & scan for last character
    case 2:
    {
      // read in rest untill trailing '%' found
      if (c == '%')
      {
        ble_buf[ble_buf_len++] = c;
        ble_buf[ble_buf_len] = '\0';   // done, terminate string

        // reset FSM state
        scan_state = 0;
   
        if (resp == BLE_RESPONSE_REBOOT)
        {
          // check for '%REBOOT%' response
          if (!strcmp (ble_buf, "%REBOOT%")); 
            return 1;
        }
        else if (resp == BLE_RESPONSE_CONNECT)
        {
          // check for '%CONNECT%' response (-or- '%CONNECT,0,801F12B42646%' WTF?)
          if (!strcmp (ble_buf, "%CONNECT")); 
            return 1;
        }
        else if (resp == BLE_RESPONSE_STREAM_OPEN)
        {
          // check for '%STREAM_OPEN% or %DISCONNECT% response
          if (!strcmp (ble_buf, "%STREAM_OPEN%")) 
            return 1;

          if (!strcmp (ble_buf, "%DISCONNECT%")) 
            return 2;
        }
        else if (resp == BLE_RESPONSE_DISCONNECT)
        {
          // check for %DISCONNECT% response
          if (!strcmp (ble_buf, "%DISCONNECT%")) 
            return 2;
        }

      }
      else if (isprint (c))
      {
        ble_buf[ble_buf_len++] = c;    // add character
      }

      break;
    }
   
    // scan 'CMD> ' prompt
    case 11:
    {
      // scan for response of format: 'CMD> '
      if (c == 'C')
      {
        // reset pointer
        ble_buf_len = 0;
        ble_buf[ble_buf_len++] = c;

        // switch state
        scan_state = 12;
      }

      break;
    }

    case 12:
    {
      // read in rest untill trailing ' ' found
      if (c == ' ')
      {
        ble_buf[ble_buf_len++] = c;
        ble_buf[ble_buf_len] = '\0';   // done, terminate string

        scan_state = 0;

        // check for 'CMD> ' response
        if (!strcmp (ble_buf, "CMD> "));  
          return 1;
      }
      else if (isprint (c))
      {
        ble_buf[ble_buf_len++] = c;    // add character
      }

      break;
    }

    // scan for 'AOK' scan stop response
    case 21:
    {
      // scan for response of format: 'AOK'
      if (c == 'A')
      {
        // reset pointer
        ble_buf_len = 0;
        ble_buf[ble_buf_len++] = c;

        // switch state
        scan_state = 22;
      }

      break;
    }

    case 22:
    {
      // read in rest untill trailing 'K' found
      if (c == 'K')
      {
        ble_buf[ble_buf_len++] = c;
        ble_buf[ble_buf_len] = '\0';   // done, terminate string

        scan_state = 0;

        // check for 'AOK' response
        if (!strcmp (ble_buf, "AOK")); 
          return 1;
      }
      else if (isprint (c))
      {
        ble_buf[ble_buf_len++] = c;    // add character
      }

      break;
    }

    // scan 'END' prompt
    case 31:
    {
      // scan for response of format: 'END'
      if (c == 'E')
      {
        // reset pointer
        ble_buf_len = 0;
        ble_buf[ble_buf_len++] = c;

        // switch state
        scan_state = 32;
      }

      break;
    }

    case 32:
    {
      // read in rest untill trailing 'D' found
      if (c == 'D')
      {
        ble_buf[ble_buf_len++] = c;
        ble_buf[ble_buf_len] = '\0';   // done, terminate string

        scan_state = 0;

        // check for 'END' response
        if (!strcmp (ble_buf, "END")); 
          return 1;
      }
      else if (isprint (c))
      {
        ble_buf[ble_buf_len++] = c;    // add character
      }

      break;
    }

    default: 
      break;
  }

  return 0;
}

// poll BLE module via UART3 & echo back to CLI via UART2
void poll_ble (void)
{
  // bail when not logging BLE module response
  if (!ble_log)
    return;

  // read BLE
  int c = uart3_getc ();

  // test for Rx buffer empty
  if (c < 0)
    return;

  // echo to serial <> dev only
  uart2_putc (c);
}

/// start polling BLE serial port
void ble_poll_on (void)
{
  ble_log = 1;

  printf2 ("BLE poll on\n");
}

// stop polling BLE serial port
void ble_poll_off (void)
{
  ble_log = 0;

  printf2 ("BLE poll off\n");
}


///////////////////////////// obselete //////////////////////////////

// start advertizing
void ble_advertizing_start (void)
{
  printf3 ("A,0100\r"); 

  debug (BLE_DEBUG, "#BLE: start advertizing\n");
}

// stop advertizing
void ble_advertizing_stop (void)
{
  printf3 ("Y\r");  
  debug (BLE_DEBUG, "#BLE: stop advertizing\n");
}



