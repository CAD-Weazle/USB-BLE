// file    : ble.h
// author  : rb
// purpose : header file for ble.c
// date    : 191014
// last    : 220907

#ifndef _BLE_H_
#define _BLE_H_

// BLE Jig state
typedef struct
{ 
  // BLE Jig statics
  char     name[16];                        // BLE Jig Module name
  uint16_t num;                             // BLE Jig Module number
  uint32_t cnt;                             // BLE Jig response counter
  char     mac[16];                         // string holding MAC address

  // BLEJIG measurement data
  // (example: JIG01: 1234 T: 16.18 RH: 40.6 VDDA: 2.82 Vsol: 1.26 Vbat: 3.88 Vamb: 2.10)
  float T;
  float RH;
  float Vdda;
  float Vsol;
  float Vbat;
  float Vamb;
} b_state;

// BLE MOISTER state
typedef struct
{ 
  // BLE MOISTER statics
  char     name[16];                        // BLE MOISTER Module name
  uint16_t num;                             // BLE MOISTER Module number
  uint32_t cnt;                             // BLE MOISTER response counter
  char     mac[16];                         // string holding MAC address

  // MOISTER measurement data
  // (example: MOISTER: 1234 VDDA: 2.79 Vsol: 0.30 Vbat: 3.10  NTC: 24.7 24.7 24.4 24.5  SOIL: 2636.0 1778.0 1358.0)
  float Vdda;
  float Vsol;
  float Vbat;
  float NTC0;
  float NTC1;
  float NTC2;
  float NTC3;
  float Soil0;
  float Soil1;
  float Soil2;
} m_state;

// BLE Jig sampling 
#define BLE_NUMJIGS                             1  // number of BLE Jigs
#define BLE_NUMMOIST                            1  // number of MOISTERs

//#define BLE_DELAY                           600  // BLE Jig sample interval                 
#define BLE_DELAY                              60  // BLE Jig sample interval  <> dev              


#define BLE_MAXRETRIES                         32  // number of Peripheral connection retries

// BLE Jig polling FSM
#define SCAN_START                              0
#define SCAN_WAIT                               1

// input buffer
#define BLE_INBUF_LEN                         255  // Rx buffer size (data from BLE unit to ARM)

// -- prototypes
void init_ble (void);

void ble_init_state (m_state bs[]);

void ble_do (void);

int8_t ble_get_data (char *mac);

void ble_poll  (void);
void ble_parse (void);

void ble_reset   (void);

void ble_connect    (char *mac);
void ble_disconnect (void);

void ble_dump_state (m_state bs[]);
void ble_dump_stats (void);

#endif













