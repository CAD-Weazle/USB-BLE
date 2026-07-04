// file    : i2c.h
// author  : rb
// purpose : header file i2c.c
// date    : 181010
// last    : 191210

#ifndef __I2C_H__
#define __I2C_H__

// -- prototypes
void init_i2c  (void);

void init_i2c1 (void);
void init_i2c2 (void);
void init_i2c3 (void);

void i2c1_read  (uint8_t adr, uint8_t buf[], uint8_t len);
void i2c1_write (uint8_t adr, uint8_t buf[], uint8_t len);

void i2c2_read  (uint8_t adr, uint8_t buf[], uint8_t len);
void i2c2_write (uint8_t adr, uint8_t buf[], uint8_t len);

void i2c3_read  (uint8_t adr, uint8_t buf[], uint8_t len);
void i2c3_write (uint8_t adr, uint8_t buf[], uint8_t len);

void i2c1_dump_registers (void);
void i2c2_dump_registers (void);
void i2c3_dump_registers (void);

#endif


