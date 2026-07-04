// file    : ports.h
// author  : rb
// purpose : header file for ports.c
// date    : 170810
// last    : 200319
//

#ifndef _PORTS_H_
#define _PORTS_H_

// -- defines

// GPIO output set/clear macros
#define gpio_set(port, pin)       do {port->BSRR = (0x00000001 << pin);} while(0) // set outputs: lo-word BSRR
#define gpio_clr(port, pin)       do {port->BSRR = (0x00010000 << pin);} while(0) // clr outputs: hi-word BSRR

// GPIO port mode defines
#define GPIO_MODE_IN                (0x00000000u)  // GPIO set to input
#define GPIO_MODE_OUT               (0x00000001u)  // GPIO set to output
#define GPIO_MODE_AF                (0x00000002u)  // GPIO set to alternate function
#define GPIO_MODE_AN                (0x00000003u)  // GPIO set to analog mode

#define GPIO_OTYPE_PP               (0x00000000u)  // output push-pull
#define GPIO_OTYPE_OD               (0x00000001u)  // output open-drain

// port A
#define LED_RED                                 9 
#define LED_GRN                                10
#define VGND                            (1 << 11)
#define BAT_SINK                        (1 << 15)

// port B
#define BLE_MODE                        (1 <<  0)
#define BLE_RESET_L                     (1 <<  1)
#define SHT_DAT                                 7
#define SHT_CLK                                 8
#define SENSOR_PWR_L                    (1 <<  9)
#define BLE_RXI_L                       (1 << 12)

// -- prototypes
void init_ports (void);

void ports_stop   (void);
void ports_resume (void); 

void led_red_on     (void);
void led_red_off    (void);
void led_red_toggle (void);
void led_red_flash  (uint16_t delay);

void led_grn_on     (void);
void led_grn_off    (void);
void led_grn_toggle (void);
void led_grn_flash  (uint16_t delay);

void vgnd_enable  (void);
void vgnd_disable (void);

void sensor_power_enable  (void);
void sensor_power_disable (void);

#endif



