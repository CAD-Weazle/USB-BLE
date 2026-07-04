// file    : sht15.h
// author  : rb
// purpose : header file for sht15.c
// date    : 150205
// last    : 200320
//

#ifndef __SHT15_H__
#define __SHT15_H__


// -- defines
#define SHT15_HI_RES                         0x00  // 12-bit %RH / 14-bit T resolution
#define SHT15_LO_RES                         0x01  //  8-bit %RH / 12-bit T resolution

#define SHT15_ACK                               1

// commands
#define SHT15_TEMP                           0x03
#define SHT15_RH                             0x05
#define SHT15_STAT_WR                        0x06
#define SHT15_STAT_RD                        0x07
#define SHT15_RESET                          0x1e

// -- prototypes
void init_sht15 (void);

void sht15_update (void);

float sht15_get_t (void);
float sht15_get_h (void);

uint16_t sht15_measure (uint8_t cmd);

void    sht15_reset (void);
void    sht15_start (void);
void    sht15_stop  (void);
uint8_t sht15_read  (uint8_t ack);
uint8_t sht15_write (uint8_t dat);

void HCLK_0 (void);
void HCLK_1 (void);
void HDAT_0 (void);
void HDAT_1 (void);

void sht15_dump     (void);
void sht15_dump_ble (void);




#endif





