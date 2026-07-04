// file    : ble.c
// author  : rb
// purpose : FSC-BT630 Feasycom BLE module interface - BLE Central Roll
// date    : 191014
// last    : 200528
//
// note    : print using color: printf ("\E[35m%c\E[30m", c);
//
// FJIG00  : F64EEED59FD2 - dev module
// FJIG01  : F732DE559A74
// FJIG02  : E1B7D94940FD
// FJIG03  : DAC8F9C0EEB9

#include "includes.h"

#define BLE_DEBUG                      0    // debug output on/off
#define BLE_CONNDEBUG                  0    // connection debug output on/off


// BLE Central response data parser (from BLE module to ARM)
int   bargc;                                // argument counter
char *bargv[128];                           // argument strings

uint8_t ble_buf_len;                        // BLE Central response buffer 
char    ble_buf[BLE_INBUF_LEN];             // BLE Central response buffer length

// BLE Jig data handling
uint16_t ble_num = 0;                       // BLE Jig now being polled
b_state  ble_state[BLE_NUMJIGS];            // BLE Jig data storage

// flags
uint8_t ble_echo = 0;                       // 0: suppress BLE module response / 1: dump BLE module response
uint8_t new_dat  = 0;                       // new data from Peripheral flag

// connection statistics
uint16_t connect_retries[BLE_NUMJIGS][BLE_MAXRETRIES+1] = {{0}};  // connection retry binning


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
//strcpy (bs[0].mac, "F64EEED59FD2");  // FJIG00 - dev only
  strcpy (bs[0].mac, "F732DE559A74");  // FJIG01
  strcpy (bs[1].mac, "E1B7D94940FD");  // FJIG02
  strcpy (bs[2].mac, "DAC8F9C0EEB9");  // FJIG03

  // set BLE Jig MAC address & reset data
  for (int i = 0; i < BLE_NUMJIGS; i++)
  {
    // give default name, should be overwritten when BLE Jig is found
    strcpy (bs[i].name, "XJIG");

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

// poll BLE Jigs round robin - from Central / to Peripheral
void ble_do (void)
{
  static uint8_t  state = SCAN_START;
  static uint16_t wait;

  switch (state)
  {
    case SCAN_START:
    {
      if (ble_get_data (ble_state[ble_num].mac) == 1)
      {
        if (++ble_num >= BLE_NUMJIGS)
        {
          // dump data all BLE Jigs
          ble_dump_state (ble_state);
  
          // reset counters & switch state
          wait = 0;
          ble_num = 0;

          state = SCAN_WAIT;
        }
      }

      break;
    }

    case SCAN_WAIT:
    {
      if (wait++ >= BLE_DELAY)
        state = SCAN_START;

      break;
    }

    default:
      break;
  }
}

// connect to BLE Jig & get data
int8_t ble_get_data (char *mac)
{
  static uint8_t busy = 0;
  static uint8_t retry = 0;

  if (!busy)
  {
    // set flag & reset state
    busy  = 1;

    // make connection with Peripheral - only when no connection active
    if ((GPIOB->IDR & CO2_RDY) == 0) // check for no connection
    {
      led_grn_flash (1);  // <> dev only, remove later
      ble_connect (mac);
    }
  }
  else
  {
    // check for new data from Peripheral or give up after too many retries (use stale data) 
    if ((new_dat) || (retry >= BLE_MAXRETRIES))
    {
      // store number connection retries
      connect_retries [ble_num][retry]++;

      // reset flags
      new_dat = 0;  // flag defined in 'ble_parse_data'
      busy    = 0;

      // dump connection state
      debug (BLE_CONNDEBUG, "#new data from Peripheral %d received: %d/%d\n", ble_num, retry, BLE_MAXRETRIES);

      retry   = 0;

      return 1;
    }
    else
    {
      // clear flag
      busy = 0; 
 
      // increment retry counter
      retry++; 

      // dump connection state
      debug (BLE_CONNDEBUG, "\E[35m#no new data from Peripheral %d received - reconnect %d/%d\n\E[30m", ble_num, retry, BLE_MAXRETRIES);
    }
  }

  return 0;
}

// get data BLE Jigs - from Peripheral / to Central
void ble_poll (void)
{
  // read BLE module
  int c = uart4_getc ();

  // test for Rx buffer empty
  if (c < 0)
    return;

  // read data from BLE Jig
  if (c == '\n')                            // check for enter to end string
  {
    ble_buf[ble_buf_len] = '\0';            // '\0' terminate buffer
    ble_buf_len          = 0;               // reset pointer

    // parse data from BLE Jig
    ble_parse ();
  }
  else if (isprint(c) && ble_buf_len < BLE_INBUF_LEN && (c != '~'))
    ble_buf[ble_buf_len++] = c;
}

// parse BLE Jig data & store
void ble_parse (void)
{
  // sanity check
  if (ble_num >= BLE_NUMJIGS)
    return;

  if (ble_echo)
    printf ("#%s\n", ble_buf);

  // parse data BLE Jig data
  bargc = split_line (ble_buf, bargv);    

  // sanity check, skip faulty response & other Module responses
  if (bargc == 14)
  {
    strcpy (ble_state[ble_num].name, bargv[0]);

    ble_state[ble_num].cnt  = atoi (bargv[1]);
    ble_state[ble_num].T    = atof (bargv[3]);
    ble_state[ble_num].RH   = atof (bargv[5]);
    ble_state[ble_num].Vdda = atof (bargv[7]);  
    ble_state[ble_num].Vsol = atof (bargv[9]);
    ble_state[ble_num].Vbat = atof (bargv[11]);
    ble_state[ble_num].Vamb = atof (bargv[13]);

    new_dat = 1;
  }
}

// reset module
void ble_reset (void)
{
  GPIOC->CLR = BLE_RESET_L;
  msleep (200);
  GPIOC->SET = BLE_RESET_L;
  msleep (200);

  debug (BLE_DEBUG, "BLE: hardware reset\r");
}

// connect device using MAC address
void ble_connect (char *mac)
{
  // connect to Peripheral
  printf4 ("AT+LECCONN=%s1\n", mac);

  debug (BLE_DEBUG, "BLE: connect %s\r", mac);
}

// dump BLE Jig state
void ble_dump_state (b_state bs[])
{
  for (int i = 0; i < BLE_NUMJIGS; i++)
  {
    printf ("%s %ld T: %2.2f RH: %2.1f VDDA: %1.2f Vsol: %1.2f Vbat: %1.2f Vamb: %1.2f  ", 
             bs[i].name, bs[i].cnt, bs[i].T, bs[i].RH, bs[i].Vdda, bs[i].Vsol, bs[i].Vbat, bs[i].Vamb);
  }

  printf ("\n");
}

// dump connection retry statistics
void ble_dump_stats (void)
{
  printf ("#number of BLE Jigs    : %d\n", BLE_NUMJIGS);
  printf ("#poll delay [s]        : %d\n", BLE_DELAY);
  printf ("#max connection retries: %d\n", BLE_MAXRETRIES);
  printf ("#retry stats           :\n");

  for (int i = 0; i <= BLE_MAXRETRIES; i++)
  {
    printf ("#%d  %d %d %d\n", i, connect_retries[0][i], connect_retries[1][i], connect_retries[2][i]);
  }
}

















