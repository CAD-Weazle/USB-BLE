// file    : ports.h
// author  : rb
// purpose : header file for ports.c
// date    : 170810
// last    : 220214
//

#ifndef _PORTS_H_
#define _PORTS_H_

#include "stm.h"

// GPIO output set/clear macros
#define gpio_set(port, pin)       do {port->BSRR = (0x00000001 << pin);} while(0) // set outputs: lo-word BSRR
#define gpio_clr(port, pin)       do {port->BSRR = (0x00010000 << pin);} while(0) // clr outputs: hi-word BSRR


// port A
#define BLE_TRAN                4
#define BLE_RESET_L             5

// port B
#define BLE_LED                 8
#define BLE_STATUS              9

// port C
#define LED_RED                14


// -- prototypes
void init_ports (void) ;

void led_red_on     (void);
void led_red_off    (void);
void led_red_toggle (void);
void led_red_flash  (uint16_t delay);

#endif