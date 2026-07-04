// file    : ble.c
// author  : rb
// purpose : RN4871 BLE module interface + Feasycom test
// date    : 191014
// last    : 200510
//
// note    : only connects to BLE module connected with Grandpa Box
// note    : print using color: printf ("\E[35m%c\E[30m", c);
//
// note    : remote mode from Central to Peripheral
//  1/ $$$         -- enter command mode
//  2/ SS,C0       -- enable Transparant UART service
//  3/ R,1         -- force reboot
//  4/ $$$         -- enter command mode
//  5/ F           -- scan devices, get <adr> -> %801F12BE1D35,0,RN4871-1D35,,B3%
//  6/ X           -- stop scanning
//  7/ C,0,0491629985CF -- connect to remote device
//  8/ $$$         -- enter command mode
//  9/ B           -- bond
// 10/ !,1         -- enter remote mode, expect RMT> prompt
// 11/ SB,09       -- remote: set Baudrate to 9600
// 12/ S-,<name>   -- set name
// 13/ !,0         -- leave remote mode 
//
// GrandpaBox   : 801F12B42646
// NEWJIG_85CF  : 0491629985CF - <> rename to 'BLEJIG01'
// BLEJIG02_7D6E: 049162997D6E - dead BLE module
// BLEJIG03_7D7D: 049162997D7D
//
// Feasycom     : F64EEED59FD2 - new
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

// BLE Central response data parser (from BLE module to ARM)
int   bargc;                                // argument counter
char *bargv[128];                           // argument strings

uint8_t ble_buf_len;                        // BLE Central response buffer 
char    ble_buf[BLE_INBUF_LEN];             // BLE Central response buffer length

// BLE Jig data handling
uint16_t ble_num = 0;                       // BLE Jig now polled
b_state ble_state[BLE_NUMJIGS];             // BLE Jig data storage


// setup BLE Module Central
void init_ble (void)
{
  // setup BLE Jig state
  ble_init_state (ble_state);

  // reset BLE Module
  ble_reset ();
}                      

// setup BLE Jig state
void ble_init_state (b_state bs[])
{
  // enter MAC addresses BLE Jigs
  strcpy (bs[0].mac, "F64EEED59FD2");
/*
  strcpy (bs[0].mac, "0491629985CF");
  strcpy (bs[1].mac, "049162997D7D");
  strcpy (bs[2].mac, "049162997D78");
*/
  // set BLE Jig MAC address & reset data
  for (int i = 0; i < BLE_NUMJIGS; i++)
  {
    // set module number
    bs[i].num = i+1;

    // reset measurements
    bs[i].T    = 0.0;
    bs[i].RH   = 0.0;
    bs[i].Vdda = 0.0;
    bs[i].Vsol = 0.0;
    bs[i].Vbat = 0.0;
    bs[i].Vamb = 0.0;
  }
}

// dump BLE Jig state
void ble_dump_state (b_state bs[])
{
  for (int i = 0; i < 3; i++)
  {
  //printf ("BLEJIG%02d - MAC: %s ", bs[i].num, bs[i].mac);
    printf ("%ld T: %2.2f RH: %2.1f VDDA: %1.2f Vsol: %1.2f Vbat: %1.2f Vamb: %1.2f   ", 
             bs[i].cnt, bs[i].T, bs[i].RH, bs[i].Vdda, bs[i].Vsol, bs[i].Vbat, bs[i].Vamb);
  }

  printf ("\n");
}

// poll BLE Jigs round robin - from Central / to Peripheral
void ble_do (void)
{
  static uint8_t  state = SCAN_START;
  static uint16_t wait;
  static uint16_t resp;
  static uint16_t first = 1;

  switch (state)
  {
    case SCAN_START:
    {
      // set wait delay, speed up first sampling
      if (first)
      {
        wait  = (BLE_DELAY - 2);
        first = 0;
      }
      else
        wait = 0;

      // reset counters
      ble_num = 0;
      resp    = 0;

    //printf ("#____________________________________________________________\n#"); // <> remove later
      printf ("#"); 
      for (int i = 0; i < BLE_DELAY; i++)
        printf ("_"); 
      printf ("\n#"); 

      state = SCAN_WAIT;
      break;
    }

    case SCAN_WAIT:
    {
      printf ("_"); // <> remove later

      if (++wait >= BLE_DELAY)
      {
        printf ("\n"); // <> remove later
        state = SCAN_CONNECT;
      }

      break;     
    }

    case SCAN_CONNECT:
    {
      // switch to BLE module command mode
      ble_cmd_mode ();

      // connect to BLE Jig
      printf ("#connect to BLE Jig %d\n", ble_num); // <> remove later
      ble_connect_device (ble_state[ble_num].mac);

      // reset counter
      resp = 0;

      state = SCAN_RESPONSE;
      break;
    }

    case SCAN_RESPONSE:
    {
      // else wait for reply
      if (resp++ < BLE_RESPONSEDLY)
        break;

      // check whether all BLE Jigs scanned
      if (++ble_num >= BLE_NUMJIGS)
      {
        state = SCAN_DONE;
        break;
      }

      state = SCAN_CONNECT;
      break;
    }

    case SCAN_DONE:
    {
      // make sure BLE Module is in known state <> needed?
      ble_reset ();

      // dump data all BLE Jigs
    //ble_dump_state (ble_state);

      state = SCAN_START;
      break;
    }

    default:
      break;
  }
}

// get data BLE Jigs - from Peripheral / to Central
void poll_ble (void)
{
  static uint8_t parse = 0;

  // read BLE module
  int c = uart4_getc ();

  // test for Rx buffer empty
  if (c < 0)
    return;

  printf ("\E[35m%c\E[30m", c);

  // start parsing BLE Jig data
//if ((c == '~'))
  parse = 1;

  // skip all other responses from BLE module
  if (!parse)
    return;

  // read data from BLE Jig
  if ((c == '\r') || (c == '\n'))           // check for enter to end string
  {
    ble_buf[ble_buf_len] = '\0';            // '\0' terminate buffer
    ble_buf_len          = 0;               // reset pointer
    parse                = 0;               // reset parse flag

    printf ("%s", ble_buf);

    // parse data from BLE Jig
    ble_get_data ();
  }
  else if (isprint(c) && ble_buf_len < BLE_INBUF_LEN && (c != '~'))
    ble_buf[ble_buf_len++] = c;
}

// parse BLE Jig data & store
void ble_get_data (void)
{
  // sanity check
  if (ble_num >= BLE_NUMJIGS)
    return;

  // <> dev only, remove later
  printf ("#%d - %s\n", ble_num, ble_buf);

  // parse data BLE Jig data
  bargc = split_line (ble_buf, bargv);    

  // sanity check, skip faulty response
  if (bargc == 13)
  {
    ble_state[ble_num].cnt  = atoi (bargv[0]);
    ble_state[ble_num].T    = atof (bargv[2]);
    ble_state[ble_num].RH   = atof (bargv[4]);
    ble_state[ble_num].Vdda = atof (bargv[6]);  
    ble_state[ble_num].Vsol = atof (bargv[8]);
    ble_state[ble_num].Vbat = atof (bargv[10]);
    ble_state[ble_num].Vamb = atof (bargv[12]);
  }
}



// reset module
void ble_reset (void)
{
  GPIOC->CLR = BLE_RESET_L;
  msleep (200);
  GPIOC->SET = BLE_RESET_L;
  msleep (200);
}

// enter command mode
void ble_cmd_mode (void)
{
  debug (BLE_DEBUG, "BLE: enter command mode\n");

  msleep (100);                        // relax a bit (p. 12 RN4871 User Manual)
  printf4 ("$$$");

  msleep (500);                        // <> test, needed?
}

// enter data mode
void ble_data_mode (void)
{
  debug (BLE_DEBUG, "BLE: enter data mode\n");
  printf4 ("---\r");
}

// connect device using MAC address
void ble_connect_device (char *mac)
{
//printf4 ("C,0,%s\r", mac);   // Microchip
  printf4 ("C,1,%s\r", mac);   // Feasycom

  debug (BLE_DEBUG, "BLE: connect %s\r", mac);
}

































#ifdef OUWEKANKAHZOOI
uint8_t ble_mode;                      // 0: data mode / 1: command mode
uint8_t ble_dev_found;                 // Grandpa Box found during scanning
char    ble_dev_mac[12];               // Grandpa Box BLE module 48-bit MAC address
uint8_t ble_connected;                 // Grandpa Box connected

// start scanning for client Devices
void ble_scan_start (void)
{
  if (ble_mode == BLE_CMD_MODE)
  {
    // send command
    printf4 ("F\r");
    debug (BLE_DEBUG, "BLE: start scanning as Central GAP\n");
  }
  else
    printf ("error: BLE not in command mode\n");
}

// stop scanning for client Devices
void ble_scan_stop (void)
{
  // send command
  printf4 ("X\r");
  debug (BLE_DEBUG, "BLE: stop scanning as Central GAP\n");
}

// process advertising BLE device
uint8_t ble_get_device (char *str)
{
  char c;
  char    buf[5][32];  // storage for BLE advertising data
  uint8_t chr = 0;     // response charachter pointer
  uint8_t wrd = 0;     // response word pointer

  printf ("BLE: client Device found: %s\n", str);

  // process advertising data
  for (int i = 0; (c = str[i]) != '\0'; i++)
  {
    if (c == ',')
    {
      // end of word
      buf[wrd][chr] = '\0';
      chr = 0;
      wrd++;
    }
    else
    {
      // read character of word
      if (c != '%')
        buf[wrd][chr++] = c;
    }    
  }

  // check for connectable & Grandpa Box, ignore rest
  if ((buf[1][0] == '0') && (!strcmp (buf[2], "GrandpaBox")))
  {
    strcpy (ble_dev_mac, buf[0]);
    
    ble_dev_found = 1;

    debug (BLE_DEBUG, "BLE: connectable client Device found, MAC: 0x%s\n", ble_dev_mac);

    return 1;
  }
  else
    return 0;
}
#endif





// poll BLE module via UART4
/*
void poll_ble (void)
{
  static uint8_t ble_state = BLE_RESET_ASSERT;

  switch (ble_state)
  {
    // first reset BLE module
    case BLE_RESET_ASSERT:
    {
      // reset BLE state
      ble_mode = BLE_DAT_MODE;       
      ble_connected = 0;

      // apply reser pulse
      ble_reset ();

      ble_state = BLE_RESET_RESPONSE;   

      break;
    }

    case BLE_RESET_RESPONSE:
    {
      if (ble_scan_response (BLE_RESPONSE_REBOOT))
      {
        printf ("BLE: device rebooting\n");

        ble_state = BLE_CMD_MODE_ENTER; 
      }
   
      break;
    }

    // enter command mode
    case BLE_CMD_MODE_ENTER:
    {
      printf ("BLE: enter command mode\n");

      ble_cmd_mode  ();

      ble_state = BLE_CMD_MODE_RESPONSE;
      break;
    }

    case BLE_CMD_MODE_RESPONSE:
    {
      if (ble_scan_response (BLE_RESPONSE_CMD))
      {
        printf ("BLE: device entered command mode\n");

        // set BLE state
        ble_mode = BLE_CMD_MODE;

        ble_state = BLE_SCAN_START;   
      }

      break;
    }

    // start scanning for BLE devices    
    case BLE_SCAN_START:
    {
      printf ("BLE: device scanning in GAP role\n");

      ble_scan_start ();

      ble_state = BLE_SCAN_START_RESPONSE;   
      break;
    }

    case BLE_SCAN_START_RESPONSE:
    {
      if (ble_scan_response (BLE_RESPONSE_SCAN_START))
      {
        printf ("BLE: Grandpa Box found\n");

        ble_state = BLE_SCAN_STOP;   
      }

      break;
    }

    // stop scanning for BLE devices    
    case BLE_SCAN_STOP:
    {
      printf ("BLE: stop scanning\n");

      // stop scanning
      ble_scan_stop ();
      
      ble_state = BLE_SCAN_STOP_RESPONSE;   
      break;
    }

    case BLE_SCAN_STOP_RESPONSE:
    {
      if (ble_scan_response (BLE_RESPONSE_SCAN_STOP))
      {
        printf ("BLE: scanning stopped\n");

        ble_state = BLE_CONNECT;   
      }

      break;
    }

    // connect to client BLE
    case BLE_CONNECT:
    {
      printf ("BLE: connect with Grandpa Box\n");

      // connect
      ble_connect_device (ble_dev_mac);

      ble_state = BLE_CONNECT_RESPONSE;

      break;
    }

    case BLE_CONNECT_RESPONSE:
    {
      if (ble_scan_response (BLE_RESPONSE_CONNECT))
      {
        printf ("BLE: connected with Grandpa Box\n");

        ble_state = BLE_DATA_MODE_ENTER;   
      }

      break;
    }

    // enter transparent UART data mode
    case BLE_DATA_MODE_ENTER:
    {
      ble_data_mode ();

      ble_state = BLE_DATA_MODE_RESPONSE;   

      break;
    }

    case BLE_DATA_MODE_RESPONSE:
    {
      if (ble_scan_response (BLE_RESPONSE_DATA_MODE))
      {
        printf ("BLE: entered data mode\n");

        ble_state = BLE_STREAM_OPEN;   
      }

      break;
    }

    // scan for '%STREAM_OPEN% or %DISCONNECT% response
    case BLE_STREAM_OPEN:
    {
      int res = ble_scan_response (BLE_RESPONSE_STREAM_OPEN);

      if (res == 1)
      {
        printf ("BLE: connection, go on with UART mode\n");
        ble_connected = 1;
        ble_state = BLE_UART_MODE;   
      }
      else if (res == 2)
      {
        printf ("BLE: no connection, start over\n");
        ble_state = BLE_RESET_ASSERT;   
      }

      break;
    }

    case BLE_UART_MODE:
    {
      // read BLE
      int c = uart4_getc ();

      // test for Rx buffer empty
      if (c < 0)
        return;

      // add byte to buffer & parse - <> dev only: use 'cli.c' commands
      if ((c == '\r') | (c == '\n'))       // check for enter to end string
    //if ((c == '\r'))                     // check for enter to end string
      {
        printf4 ("\n\r");                  // print newline

        ble_buf[ble_buf_len] = '\0';       // '\0' terminate buffer
        ble_buf_len          = 0;          // reset pointer
    
        bargc = split_line (ble_buf, bargv); // parse command
        parse_line (bargc, bargv);          

        prompt ();                         // display new prompt
      }
      else if (c == '\b' || c == 0x7f)     // check for backspace and delete
      {
        if (ble_buf_len > 0)
         ble_buf_len--;
      }
      else if (isprint(c) && ble_buf_len < BLE_INBUF_LEN)
        ble_buf[ble_buf_len++] = c;


      // read UART1 & send <> test only
      int d = uart1_getc ();
    
      if (d > 0)
        printf4 ("%c", d);
    
      break;
    }

    default:
      break;
  }
}
*/

/*
// scan BLE response
uint8_t ble_scan_response (uint8_t resp)
{
  static uint8_t scan_state = 0;

  int c = uart4_getc ();

  if (c < 0)
    return 0;

//printf ("\E[35m%c\E[30m\n", c);

  // jump in FSM 
  if (scan_state == 0)
  {
    if ((resp == BLE_RESPONSE_REBOOT)     || 
        (resp == BLE_RESPONSE_SCAN_START) || 
        (resp == BLE_RESPONSE_CONNECT)    ||
        (resp == BLE_RESPONSE_STREAM_OPEN))
      scan_state = 1;
    else if (resp == BLE_RESPONSE_CMD)
      scan_state = 11;
    else if (resp == BLE_RESPONSE_SCAN_STOP)
      scan_state = 21;
    else if (resp == BLE_RESPONSE_DATA_MODE)
      scan_state = 31;
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
          if (!strcmp (ble_buf, "%REBOOT%"))
            return 1;
        }
        else if (resp == BLE_RESPONSE_SCAN_START)
        {
          // check for client device
          if (ble_get_device (ble_buf))
            return 1;   
        }
        else if (resp == BLE_RESPONSE_CONNECT)
        {
          // check for '%CONNECT%' response (-or- '%CONNECT,0,801F12B42646%' WTF?)
          if (!strcmp (ble_buf, "%CONNECT"))
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
        if (!strcmp (ble_buf, "CMD> ")) 
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
        if (!strcmp (ble_buf, "AOK")) 
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
        if (!strcmp (ble_buf, "END")) 
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
*/

