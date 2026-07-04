// file    : ble.h
// author  : rb
// purpose : header file for ble.c
// date    : 191014
// last    : 200408

#ifndef _BLE_H_
#define _BLE_H_

// input buffer
#define BLE_INBUF_LEN                         255  // Rx buffer size (data from BLE unit to ARM)
#define SCAN_TIMEOUT                           50  // BLE module response timeout [dsec counts]

// BLE traffic handler
#define BLE_POWER_DOWN                          0
#define BLE_POWER_UP                            1
#define BLE_READ_SENSORS                        2
#define BLE_SEND_DATA                           3
#define BLE_CONNECTION_CLOSE                    4
#define BLE_RESET                               5

// BLE response parser
#define BLE_RESPONSE_REBOOT                     0
#define BLE_RESPONSE_CMD                        1
#define BLE_RESPONSE_SCAN_START                 2
#define BLE_RESPONSE_SCAN_STOP                  4
#define BLE_RESPONSE_CONNECT                    5
#define BLE_RESPONSE_DISCONNECT                 6
#define BLE_RESPONSE_DATA_MODE                  7
#define BLE_RESPONSE_STREAM_OPEN                8
#define BLE_RESPONSE_AOK                        9

// -- prototypes
void init_ble (void);

void ble_do (void);

void ble_reset (void);

void ble_cmd_mode  (void);
void ble_data_mode (void);

void ble_power_down (void);
void ble_power_up   (void);

void ble_advertizing_start (void);
void ble_advertizing_stop  (void);

void ble_connection_close (void);

void poll_ble    (void);
void ble_poll_on (void);
void ble_poll_on (void);

uint8_t ble_scan_response (uint8_t resp);

#endif


