// file    : bus-handler.c
// author  : rb
// purpose : SunWeb RS-485 Serial Bus Master - for SunWeb Master
// date    : 180826
// last    : 190712
//
// note    : packet format:
//           SYN | ID | CMD | 0x00 | CHK           - no payload  : LEN == 0
//           SYN | ID | CMD | LEN  | PAY |...| CHK - with payload: LEN == payload length
// rx[]/tx[]  0     1    2     3      4         N
//
// note    : polling bus Devices must be non-blocking
//
// SYN = 'U'
// ID  = SunWeb Bus ID:
//       0x00: broadcast
//       0x1X: SimSun, X = PCB number -- 0 = broadcast all SimSun boards
//       0x2X: Hotbed, X = PCB number -- 0 = broadcast all HotBed boards
//
// CMD = 0xAB
// LEN = payload length, checksum excluded
// CHK = checksum over header & payload, SYNC byte excluded
//

#include "includes.h"

#define SW_DEBUG_POLL                  0    // debug SunWeb Rx data from bus
#define SW_DEBUG_CHECKSUM              0    // dump checksum stats
#define SW_TX_ECHO                     0    // dump Tx SunWeb packets

// SunWeb Tx/Rx buffers
uint8_t tx_buf [TX_BUFLEN];                 // Tx packet buffer
uint8_t rx_buf [RX_BUFLEN];                 // Rx packet buffer

// Rx packet state
uint8_t rx_cnt = 0;                         // Rx packet buffer counter
uint8_t rx_new = 0;                         // new Rx packet valid flag

// Rx packet payload state
uint8_t  pcnt = 0;                          // Rx packet payload counter
uint32_t plen = 0;                          // Rx packet payload length

// poll SunWeb Bus USART1
void poll_bus (void)
{
  static uint8_t poll_state = SW_POLL_START;

  // get next character from SunWeb Bus
  int c = uart4_getc ();

  // bail when UART receive buffer empty
  if (c < 0)
    return;

  switch (poll_state)
  {
    case SW_POLL_START:
    {
      if (c == SW_SYNC)
      {
        rx_cnt = 0;          // reset Rx buffer counter
        pcnt   = 0;          // reset payload counter

        rx_buf[rx_cnt++] = c;

        debug (SW_DEBUG_POLL, "\n---\nsync found     : 0x%02x\n", rx_buf[rx_cnt-1]);

        poll_state = SW_POLL_GETID;
      }

      break;
    }

    case SW_POLL_GETID:
    {
      rx_buf[rx_cnt++] = c;

      debug (SW_DEBUG_POLL, "packet ID      : 0x%02x\n", rx_buf[rx_cnt-1]);

      poll_state = SW_POLL_GETCMD;
      break;
    }

    case SW_POLL_GETCMD:
    {
      rx_buf[rx_cnt++] = c;
 
      debug (SW_DEBUG_POLL, "packet CMD     : 0x%02x\n", rx_buf[rx_cnt-1]);

      poll_state = SW_POLL_GETLENGTH;
      break;
    }

    // get payload length
    case SW_POLL_GETLENGTH:
    {
      // get payload length, check for valid length
      if (c > SW_MAXLEN)
      {
        printf ("error: payload too long - %d (should be %d)\n", c, rx_buf[rx_cnt-1]);
        poll_state = SW_POLL_ERROR;
      }
      else
      {
        plen = c;
        rx_buf[rx_cnt++] = c;
        debug (SW_DEBUG_POLL, "payload length : %d\n", rx_buf[rx_cnt-1]);

        // check for payload
        if (plen)
          poll_state = SW_POLL_GETPAYLOAD;
        else
          poll_state = SW_POLL_GETCHECK;
      }     

      break;
    }

    // get payload & checksum
    case SW_POLL_GETPAYLOAD:
    {
      if (++pcnt <= plen)
      {
        // first, get payload
        rx_buf[rx_cnt++] = c;
        break;
      }

      poll_state = SW_POLL_GETCHECK;

      // fall thru
    }

    case SW_POLL_GETCHECK:
    {
      // get checksum
      rx_buf[rx_cnt] = c;    // get checksum

      debug (SW_DEBUG_POLL, "checksum       : 0x%02x\n", rx_buf[rx_cnt]);

      poll_state = SW_POLL_DONE;
      // fall thru
    }

    case SW_POLL_DONE:
    {
      rx_new = 1;            // set packet received flag

      debug (SW_DEBUG_POLL, "new packet found, ok\n---\n");

      poll_state = SW_POLL_START;
      break;
    }
     
    case SW_POLL_ERROR:
    {
      printf ("some bus parsing error, bail\n");

      poll_state = SW_POLL_START;
      break;
    }

    default:
      break;
  }
}

// assemble SunWeb Tx packet
void sw_txpack (uint8_t id, uint8_t cmd, uint8_t len, uint8_t *buf)
{
  uint8_t sum = 0;

  // assemble Tx header
  tx_buf[0] = SW_SYNC;
  tx_buf[1] = id;
  tx_buf[2] = cmd;
  tx_buf[3] = len;

  // copy payload to Tx buffer
  for (int i = 0; i < len; i++)
    tx_buf[SW_HDRLEN + i] = buf[i];

  // calculate checksum, skip SYNC byte
  for (int i = 1; i < SW_HDRLEN + len; i++)
    sum ^= tx_buf[i];

  tx_buf[SW_HDRLEN + len] = sum;  
}

// send SunWeb Tx packet
void sw_txsend (void)
{
  uint8_t len;

  // dump Tx packet
  if (SW_TX_ECHO)
    sw_txdump ();

  // get Tx packet length
  len = SW_HDRLEN + tx_buf[3] + 1;      

  // switch transceiver to transmit mode
  msleep (5);                 // give Devices time for bus turn-around
  rs485_dir (SW_SEND);
  usleep (100);               // relax a bit <> needed?

  // send SunWeb Packet
  for (int i = 0; i < len; i++)
  {
    uart4_putc (tx_buf[i]);
    // wait for UART transmit complete
    while (!(UART4->SR & USART_SR_TXE))
      ;
  }

  // wait for transmit done & switch transceiver to receive mode
  msleep (5);                 
  rs485_dir (SW_RECV);
}

// check Rx packet checksum
uint8_t sw_rxcheck (void)
{
  uint8_t sum = 0;
  uint8_t len = rx_buf[3];

  static uint32_t ok  = 0;
  static uint32_t err = 0;

  // calculate checksum, skip SYNC byte
  for (int i = 1; i < SW_HDRLEN + len; i++)
    sum ^= rx_buf[i];

  if (sum == rx_buf [SW_HDRLEN + len])
  {
    debug (SW_DEBUG_CHECKSUM, "Rx checksum ok   : %ld/%ld\n", ++ok, err);
    return 1;
  }
  else
  {
    printf ("Rx checksum ERROR: %ld/%ld\n", ok, ++err);
    return 0;
  }
}

// dump Tx packet buffer
void sw_txdump (void)
{
  printf ("Tx: ");
  dump_ascii (tx_buf, (uint16_t)(SW_HDRLEN + tx_buf[3] + 1));
}

// dump Rx packet buffer
void sw_rxdump (void)
{
  printf ("Rx: ");
  dump_ascii (rx_buf, (uint16_t)(SW_HDRLEN + rx_buf[3] + 1));
}

// make flag externally available
uint8_t sw_rxnew (void)
{
  return rx_new;
}

// clear flag from external routine
void sw_rxclear (void)
{
  rx_new = 0;
}
