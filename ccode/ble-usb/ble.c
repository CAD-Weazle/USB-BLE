// file    : ble.c
// author  : rb
// purpose : FSC-BT630 Feasycom BLE module interface - BLE Central Roll
// date    : 191014
// last    : 220907
//
// note    : print using color: printf ("\E[35m%c\E[30m", c);
//
// * BLE Jigs:
// FJIG00  : F64EEED59FD2 - dev module
// FJIG01  : F732DE559A74
// FJIG02  : E1B7D94940FD
// FJIG03  : DAC8F9C0EEB9
//
// * MOISTER:
// MOISTER : F64EEED59FD2

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
//b_state  ble_state[BLE_NUMJIGS];          // BLE Jig data storage
m_state  ble_state[BLE_NUMMOIST];           // BLE MOISTER data storage

// flags
uint8_t ble_echo = 0;                       // 0: suppress BLE module response / 1: dump BLE module response
uint8_t new_dat  = 0;                       // new data from Peripheral flag

// connection statistics
//uint16_t connect_retries[BLE_NUMJIGS][BLE_MAXRETRIES+1] = {{0}};  // connection retry binning
uint16_t connect_retries[BLE_NUMMOIST][BLE_MAXRETRIES+1] = {{0}};  // connection retry binning


// setup BLE Module Central
void init_ble (void)
{
  // setup BLE Jig state
  ble_init_state (ble_state);

  // reset BLE Module
  ble_reset ();

  // <> output reset state for PyQtGraph script
//ble_dump_state (ble_state);
}                      

// setup BLE Jig state
void ble_init_state (m_state bs[])
{
  // enter MAC addresses BLE Jigs
//strcpy (bs[0].mac, "F64EEED59FD2");  // FJIG00 - dev only

//strcpy (bs[0].mac, "F732DE559A74");  // FJIG01
//strcpy (bs[1].mac, "E1B7D94940FD");  // FJIG02
//strcpy (bs[2].mac, "DAC8F9C0EEB9");  // FJIG03

  strcpy (bs[0].mac, "F64EEED59FD2");  // MOISTER proto 

//// set BLE Jig MAC address & reset data
//for (int i = 0; i < BLE_NUMJIGS; i++)
//{
//  // give default name, should be overwritten when BLE Jig is found
//  strcpy (bs[i].name, "XJIG");
//
//  // set module number
//  bs[i].num = i+1;
//
//  // reset measurements
//  bs[i].T    = 0.0;
//  bs[i].RH   = 0.0;
//  bs[i].Vdda = 0.0;
//  bs[i].Vsol = 0.0;
//  bs[i].Vbat = 0.0;
//  bs[i].Vamb = 0.0;
//}

  // set MOISTER MAC address & reset data
  for (int i = 0; i < BLE_NUMMOIST; i++)
  {
    // give default name, should be overwritten when BLE MOISTER is found
    strcpy (bs[i].name, "XMOIST");
  
    // set module number
    bs[i].num = i+1;
  
    // reset measurements
    bs[i].Vdda  = 0.0;
    bs[i].Vsol  = 0.0;
    bs[i].Vbat  = 0.0;
    bs[i].NTC0  = 0.0;
    bs[i].NTC1  = 0.0;
    bs[i].NTC2  = 0.0;
    bs[i].NTC3  = 0.0;
    bs[i].Soil0 = 0.0;
    bs[i].Soil1 = 0.0;
    bs[i].Soil2 = 0.0;
  }
}

// poll BLE Jigs round robin - from Central / to Peripheral
void ble_do (void)
{
  static uint8_t  state = SCAN_START;
  static uint16_t wait;

  // wait for USB VCP configured
//if (usb_configured () == 0)
//{
//  printf ("ERR: USB not configured\n");
//  return;
//}

  switch (state)
  {
    case SCAN_START:
    {
      if (ble_get_data (ble_state[ble_num].mac) == 1)
      {
        // drop connection <> dev ony!!!
      //ble_disconnect ();
        ble_reset ();

        if (++ble_num >= BLE_NUMMOIST)
        {
          // dump data all BLE Peripherals
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

// connect to BLE Peripheral & get data - connection is dropped by Peripheral
int8_t ble_get_data (char *mac)
{
  static uint8_t busy = 0;
  static uint8_t retry = 0;

  if (!busy)
  {
    // set flag & reset state
    busy = 1;

    // make connection with Peripheral - only when no connection active
  //if ((GPIOB->IDR & (BLE_STATUS << 1)) == 0) // check for no connection <> check pin
  //{
      ble_connect (mac);
  //}
  }
  else
  {
    // check for new data from Peripheral or give up after too many retries (use stale data) 
    if ((new_dat) || (retry >= BLE_MAXRETRIES))
    {
      // store number connection retries
      if (retry <= BLE_MAXRETRIES)
        connect_retries [ble_num][retry]++;

      // dump connection state
      if (new_dat)
      {
        debug (BLE_CONNDEBUG, "#new data from Peripheral %d received: %d/%d\n", ble_num, retry, BLE_MAXRETRIES);
      }
      else
      {
        debug (BLE_CONNDEBUG, "\E[31m#no new data from Peripheral %d received\n\E[30m", ble_num);
      }

      // reset flags
      new_dat = 0;  // flag defined in 'ble_parse_data'
      busy = 0;
      retry = 0;

      return 1;
    }
    else
    {
      // clear flag
      busy = 0; 
 
      // increment retry counter
      retry++; 

      // dump connection state
      debug (BLE_CONNDEBUG, "\E[31m#no new data from Peripheral %d received - reconnect %d/%d\n\E[30m", ble_num, retry, BLE_MAXRETRIES);
    }
  }

  return 0;
}

// get data BLE Peripheral - from Peripheral / to Central
void poll_ble (void)
{
  // read BLE module
  int c = uart2_getc ();

  // test for Rx buffer empty
  if (c < 0)
    return;

  // read data from BLE Peripheral
  if (c == '\n')                            // check for enter to end string
  {
    ble_buf[ble_buf_len] = '\0';            // '\0' terminate buffer
    ble_buf_len          = 0;               // reset pointer

    // parse data from BLE Peripheral
    ble_parse ();
  }
  else if (isprint(c) && ble_buf_len < BLE_INBUF_LEN && (c != '~'))
    ble_buf[ble_buf_len++] = c;
}

// parse BLE Jig data & store
void ble_parse (void)
{
  // sanity check
  if (ble_num >= BLE_NUMMOIST)
    return;

  if (ble_echo)
    printf ("#%s\n\r", ble_buf);

  // parse data BLE Peripheral data
  bargc = split_line (ble_buf, bargv);    
/*
  // sanity check, skip faulty response & other Module responses - FJIG
  if (bargc == 14)
  {
    printf (">>%s\n\r", ble_buf);

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
*/
  // sanity check, skip faulty response & other Module responses - MOISTER
  // (example: MOISTER: 1234 VDDA: 2.79 Vsol: 0.30 Vbat: 3.10  NTC: 24.7 24.7 24.4 24.5  SOIL: 2636.0 1778.0 1358.0)
  if (bargc == 17)
  {
    strcpy (ble_state[ble_num].name, bargv[0]);

    ble_state[ble_num].cnt    = atoi (bargv[1]);                             
    ble_state[ble_num].Vdda   = atof (bargv[3]);
    ble_state[ble_num].Vsol   = atof (bargv[5]);
    ble_state[ble_num].Vbat   = atof (bargv[7]);
    ble_state[ble_num].NTC0   = atof (bargv[9]);
    ble_state[ble_num].NTC1   = atof (bargv[10]);
    ble_state[ble_num].NTC2   = atof (bargv[11]);
    ble_state[ble_num].NTC3   = atof (bargv[12]);
    ble_state[ble_num].Soil0  = atof (bargv[14]);
    ble_state[ble_num].Soil1  = atof (bargv[15]);
    ble_state[ble_num].Soil2  = atof (bargv[16]);

    new_dat = 1;
  }
  else
  {
    // dump mangled data <> dev only
    printf ("#%s\n\r", ble_buf);
  }
}

// reset module
void ble_reset (void)
{
  gpio_clr (GPIOA, BLE_RESET_L);
  msleep (200);
  gpio_set (GPIOA, BLE_RESET_L);
  msleep (200);

  // set Mode - has no effect
//gpio_clr (GPIOA, BLE_TRAN);  // set Throughput Mode
  gpio_set (GPIOA, BLE_TRAN);  // set Command Mode

  debug (BLE_DEBUG, "BLE: hardware reset\n");
}

// connect Peripheral using MAC address
void ble_connect (char *mac)
{
  // connect to Peripheral
  printf2 ("AT+LECCONN=%s1\n", mac);

  debug (BLE_DEBUG, "BLE: connect %s\n", mac);
}

// disconnect Peripheral
void ble_disconnect (void)
{
  // connect to Peripheral
  printf2 ("AT+DISC\n");

  debug (BLE_DEBUG, "BLE: disconnect\n");
}


// dump BLE Jig state
void ble_dump_state (m_state bs[])
{
  // dump state FJIG Peripherals
//for (int i = 0; i < BLE_NUMJIGS; i++)
//{
//  printu ("%s %ld T: %2.2f RH: %2.1f VDDA: %1.2f Vsol: %1.2f Vbat: %1.2f Vamb: %1.2f  ", 
//           bs[i].name, bs[i].cnt, bs[i].T, bs[i].RH, bs[i].Vdda, bs[i].Vsol, bs[i].Vbat, bs[i].Vamb);
//  printf ("%s %ld T: %2.2f RH: %2.1f VDDA: %1.2f Vsol: %1.2f Vbat: %1.2f Vamb: %1.2f  ", 
//           bs[i].name, bs[i].cnt, bs[i].T, bs[i].RH, bs[i].Vdda, bs[i].Vsol, bs[i].Vbat, bs[i].Vamb);
//
//}

  // dump state MOISTER Peripherals
  for (int i = 0; i < BLE_NUMMOIST; i++)
  { 
    printu ("%s %ld VDDA: %1.2f Vsol: %1.2f Vbat: %1.2f NTC: %2.1f %2.1f %2.1f %2.1f SOIL: %3.1f %3.1f %3.1f ", 
             bs[i].name, bs[i].cnt, bs[i].Vdda, bs[i].Vsol, bs[i].Vbat, bs[i].NTC0, bs[i].NTC1, bs[i].NTC2, bs[i].NTC3, bs[i].Soil0, bs[i].Soil1, bs[i].Soil2);
  }

  // dump BLE links stats
  ble_dump_stats ();

  printu ("\n\r");
  printf ("\n");
}

// dump connection retry statistics
void ble_dump_stats (void)
{
// printu ("#number of BLE Peripherals: %d\n\r", BLE_NUMJIGS);
// printu ("#poll delay [s]           : %d\n\r", BLE_DELAY);
// printu ("#max connection retries   : %d\n\r", BLE_MAXRETRIES);
// printu ("#retry stats              :\n\r");
 
  printu ("LINK: ");

  for (int i = 0; i <= BLE_MAXRETRIES; i++)
  {
    for (int j = 0; j < BLE_NUMMOIST; j++)
      printu ("%d ", connect_retries[j][i]);
  }
}

















