// file    : bus-handler.h
// author  : rb
// purpose : header file for bus-handler.c
// date    : 180916
// last    : 190116
//

#ifndef _BUS_HANDLER_H_
#define _BUS_HANDLER_H_

// -- globals
extern uint8_t rx_buf [16];            // Rx packet buffer

// -- defines
//#define SW_NUM_DEVICES          6      // max number SunWeb Devices connected
//#define SW_NUM_GBOXES (SW_NUM_DEVICES/2) // max number GroBoxes

#define SW_MAXLEN              16      // SunWeb maximum packet length
#define TX_BUFLEN       SW_MAXLEN      // length SunWeb transmit buffer
#define RX_BUFLEN       SW_MAXLEN      // length SunWeb receive buffer

#define SW_RECV                 0      // RS-485 direction: receive
#define SW_SEND                 1      // RS-485 direction: send

#define SW_HDRLEN               4      // SunWeb Packet header length (SYNC+ID+CMD+LEN)

#define SW_ID_BYTE      rx_buf[1]      // Rx packet byte holding Device ID
#define SW_CMD_BYTE     rx_buf[2]      // Rx packet byte holding command
#define SW_LEN_BYTE     rx_buf[3]      // Rx packet byte holding packet length

// SunWeb Packet parsing statemachine
#define SW_POLL_START           0
#define SW_POLL_GETID           1
#define SW_POLL_GETCMD          2
#define SW_POLL_GETLENGTH       3
#define SW_POLL_GETPAYLOAD      4
#define SW_POLL_GETCHECK        5
#define SW_POLL_ERROR          98
#define SW_POLL_DONE           99

// SunWeb Packet defines
// -- SYNC byte
#define SW_SYNC              0x55      // sync byte 

// -- Device IDs
#define SW_ID_BROADCAST      0x00      // broadcast address all Devices
#define SW_ID_BCASTLEDS      0x10      // multicast address LED Devices
#define SW_ID_BCASTHEAT      0x20      // multicast address Heater Devices

// -- prototypes
void poll_bus (void);

void    sw_txpack  (uint8_t id, uint8_t cmd, uint8_t len, uint8_t *buf);
void    sw_txsend  (void);
uint8_t sw_rxcheck (void);
void    sw_txdump  (void);
void    sw_rxdump  (void);

uint8_t sw_rxnew   (void);
void    sw_rxclear (void);

#endif

