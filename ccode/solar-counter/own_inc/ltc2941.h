// file    : ltc2941.h
// author  : rb
// purpose : header file for ltc2941.c
// date    : 200430
// last    : 200612
//

#ifndef _LTC2941_H_
#define _LTC2941_H_

// -- defines 
#define LTC2941_IFILTERDEPTH                   64  // number samples points current filter

#define LTC2941_I2C_ADR                0b11001000  // I2C address
#define LTC2941_I2C_WADR               0b11001000  // I2C write address
#define LTC2941_I2C_RADR               0b11001001  // I2C read address

#define LTC2941_REG_STATUS                      0
#define LTC2941_REG_CONTROL                     1
#define LTC2941_REG_CHARGE                      2
#define LTC2941_REG_THRESHOLD_HI                4
#define LTC2941_REG_THRESHOLD_LO                6

#define LTC2941_REG_CHARGE_MSB                  2
#define LTC2941_REG_CHARGE_LSB                  3
#define LTC2941_REG_THRESHI_MSB                 4
#define LTC2941_REG_THRESHI_LSB                 5
#define LTC2941_REG_THRESLO_MSB                 6
#define LTC2941_REG_THRESLO_LSB                 7

#define LTC2941_PRESCALE_1           (0b000 << 3)
#define LTC2941_PRESCALE_2           (0b001 << 3)
#define LTC2941_PRESCALE_4           (0b010 << 3)
#define LTC2941_PRESCALE_8           (0b011 << 3)
#define LTC2941_PRESCALE_16          (0b100 << 3)
#define LTC2941_PRESCALE_32          (0b101 << 3)
#define LTC2941_PRESCALE_64          (0b110 << 3)
#define LTC2941_PRESCALE_128         (0b111 << 3)

#define LTC2941_CHARGE_COMPLETE           (0b010)

// -- prototypes
void init_ltc2941 (void);

void ltc2941_update (void);

float ltc2941_ifilter (float dat);

uint8_t  ltc2941_rd_reg8  (uint8_t reg);
uint16_t ltc2941_rd_reg16 (uint8_t reg);

void ltc2941_wr_reg8  (uint8_t reg, uint8_t  dat);
void ltc2941_wr_reg16 (uint8_t reg, uint16_t dat);

void ltc2941_charge_complete (void);

void ltc2941_dump     (void);
void ltc2941_reg_dump (void);

#endif



