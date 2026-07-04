// file    : ports.h
// author  : rb
// purpose : header file for ports.c
// date    : 180505
// last    : 191210
//

#ifndef _PORTS_H_
#define _PORTS_H_

// port A
#define DIR_485         (1 <<  2)

// port B
#define LIGHT_SD        (1 <<  2)
#define LED_RED         (1 <<  6)
#define LED_GRN         (1 <<  7)
#define LEVEL2          (1 <<  8)
#define PUMP2           (1 <<  9)
#define CO2_RDY         (1 << 12)
#define EE_SEL_L        (1 << 13)

// port C
#define BLE_RESET_L     (1 <<  9)  // test only

// port D
#define TFT_SEL_L       (1 <<  0)
#define TFT_PD_L        (1 <<  1)
#define TFT_INT_L       (1 <<  3)
  
// port E
#define PH2_OFF_L       (1 <<  0)
#define LEVEL1          (1 <<  1)
#define PUMP1           (1 <<  2)
#define PH1_OFF_L       (1 <<  3)
#define LEVEL0          (1 <<  4)
#define PUMP0           (1 <<  5)
#define PH0_OFF_L       (1 <<  6)
#define REF_EC          (1 <<  7)
#define REF_PH          (1 <<  8)
#define RTC_SEL_L       (1 <<  9)

// port H
#define EC_SEL0         (1 <<  0)
#define EC_SEL1         (1 <<  1)

// water pump control 
#define PUMP_0           (1 << 0) 
#define PUMP_1           (1 << 1)
#define PUMP_2           (1 << 2)
#define PUMP_ALL         (PUMP_2 | PUMP_1 | PUMP_0)

// -- prototypes
void init_ports (void);

void led_grn_on     (void);
void led_grn_off    (void);
void led_grn_toggle (void);
void led_grn_flash  (uint16_t delay);

void led_red_on     (void);
void led_red_off    (void);
void led_red_toggle (void);
void led_red_flash  (uint16_t delay);

void rs485_dir (uint16_t dir);

void ds3234_sel   (void);
void ds3234_unsel (void);

void ec_power_on  (void);
void ec_power_off (void);

void ph_power_on  (void);
void ph_power_off (void);

void tft_power_on  (void);
void tft_power_off (void);
void tft_sel       (void);
void tft_unsel     (void);

void eeprom_sel   (void);
void eeprom_unsel (void);

void photo_sensor_on  (void);
void photo_sensor_off (void);

#endif


