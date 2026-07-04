// file    : i2c.h
// author  : rb
// purpose : header file i2c.c
// date    : 181010
// last    : 200501

#ifndef __I2C_H__
#define __I2C_H__

#define I2C_TIMEOUT                           100  // I2C timeout
#define I2C_WAITDELAY                         100  // I2C timeout wait delay [us]

// -- prototypes
void init_i2c  (void);

void init_i2c1 (void);
void init_i2c2 (void);
void init_i2c3 (void);

void i2c1_read  (uint8_t adr, uint8_t buf[], uint8_t len);
void i2c1_write (uint8_t adr, uint8_t buf[], uint8_t len);

uint8_t i2c1_read_reg  (uint8_t adr, uint8_t reg);
void    i2c1_write_reg (uint8_t adr, uint8_t reg, uint8_t dat);

void i2c2_read  (uint8_t adr, uint8_t buf[], uint8_t len);
void i2c2_write (uint8_t adr, uint8_t buf[], uint8_t len);

void i2c3_read  (uint8_t adr, uint8_t buf[], uint8_t len);
void i2c3_write (uint8_t adr, uint8_t buf[], uint8_t len);

void i2c1_dump_registers (void);
void i2c2_dump_registers (void);
void i2c3_dump_registers (void);

#endif


