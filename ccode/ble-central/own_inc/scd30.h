// file    : scd30.h
// author  : rb
// purpose : header file scd30.c
// date    : 190702
// last    : 191210

#ifndef __SCD30_H__
#define __SCD30_H__

//#include "../includes.h"

#define SCD30_FILTERDEPTH             32      // moving average filter depth

// -- defines
#define SCD30_RADR                  0xc3      // 0b1100.0011
#define SCD30_WADR                  0xc2      // 0b1100.0010

// CRC8 polynomial
#define SCD30_CRC8_POLYNOMIAL       0x31
#define SCD30_CRC8_INIT             0xff
#define SCD30_CRC8_LEN                 1

// SCD30 commands
#define SCD30_GETID               0xd100
#define SCD30_RESET               0xd304

#define SCD30_STARTMEASURE        0x0010
#define SCD30_STOPMEASURE         0x0104
#define SCD30_GETMEASURE          0x0300

#define SCD30_SETINTERVAL         0x4600
#define SCD30_SETOFFSET           0x5403
#define SCD30_SETALTITUDE         0x5102

#define SCD30_GETREADY            0x0202

#define SCD30_SETSELFCAL          0x5306
#define SCD30_SETFACTOR           0x5204


// -- prototypes
void init_scd30 (void);

void scd30_update (void);

void scd30_reset (void);

uint16_t scd30_getid    (void);
uint16_t scd30_getready (void);

void    scd30_startmeasure (void);
void    scd30_stopmeasure  (void);
uint8_t scd30_getmeasure   (void);

float scd30_filter (float dat);

void scd30_setinterval (uint16_t interval);
void scd30_setoffset   (uint16_t offset);
void scd30_getoffset   (void);
void scd30_setaltitude (uint16_t altitude);

uint8_t scd30_command (uint16_t cmd, uint8_t sdat[], uint8_t slen, uint8_t rdat[], uint8_t rlen);

uint8_t scd30_generate_crc (uint8_t dat[], uint8_t num);

void scd30_log_all (void);

void scd30_state_update (void);

#endif


