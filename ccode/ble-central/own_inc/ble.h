// file    : ble.h
// author  : rb
// purpose : header file for ble.c
// date    : 191014
// last    : 200528

#ifndef _BLE_H_
#define _BLE_H_

// BLE module state
typedef struct
{ 
  // BLE Jig statics
  char     name[16];                        // BLE Jig Module name
  uint16_t num;                             // BLE Jig Module number
  uint32_t cnt;                             // BLE Jig response counter
  char     mac[16];                         // string holding MAC address

  // BLEJIG measurement data
  // (example: T: 16.18 RH: 40.6 VDDA: 2.82 Vsol: 1.26 Vbat: 3.88 Vamb: 2.10)
  float T;
  float RH;
  float Vdda;
  float Vsol;
  float Vbat;
  float Vamb;
} b_state;

extern b_state ble_state[3];                       // <> remove later

// BLE Jig sampling 
#define BLE_NUMJIGS                             3  // number of BLE Jigs

#define BLE_DELAY                             600  // BLE Jig sample interval                 
#define BLE_MAXRETRIES                         16  // number of Peripheral connection retries

// BLE Jig polling FSM
#define SCAN_START                              0
#define SCAN_WAIT                               1

// input buffer
#define BLE_INBUF_LEN                         255  // Rx buffer size (data from BLE unit to ARM)

// -- prototypes
void init_ble (void);

void ble_init_state (b_state bs[]);

void ble_do (void);

int8_t ble_get_data (char *mac);

void ble_poll  (void);
void ble_parse (void);

void ble_reset   (void);
void ble_connect (char *mac);

void ble_dump_state (b_state bs[]);
void ble_dump_stats (void);

#endif













