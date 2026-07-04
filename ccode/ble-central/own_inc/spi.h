// file    : spi.h
// author  : rb
// purpose : header file for spi.c
// date    : 170820
// last    : 191210
//

#ifndef _SPI_H_
#define _SPI_H_

// -- defines
#define SPI_DS3234_MODE                           0  // for DS3234 RTC
#define SPI_TFT_MODE                              1  // for BT81x TFT display driver
#define SPI_EEPROM_MODE                           2  // for 25aa040 EEPROM

// -- prototypes
void init_spi (void);

void spi1_init (void);
void spi2_init (void);
void spi3_init (void);

uint8_t spi1_rw_byte    (uint8_t data);
void    spi1_write_byte (uint8_t data);
uint8_t spi1_read_byte  (void);

uint8_t spi2_rw_byte    (uint8_t data);
void    spi2_write_byte (uint8_t data);
uint8_t spi2_read_byte  (void);

uint8_t spi3_rw_byte    (uint8_t data);
void    spi3_write_byte (uint8_t data);
uint8_t spi3_read_byte  (void);
void    spi3_set_mode   (uint8_t type);

#endif


