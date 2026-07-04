// file    : i2c.c
// author  : rb
// purpose : I2C routines STM32L1xx
// date    : 190701
// last    : 200429
//
// note    : during POR both SDA and SCL are pulled down for a brief moment,
//           this gives gezak in the I2C FSM, adding delay & clearing state is needed

#include "includes.h"

#define I2C_ERR             1  // dump I2C timeout errors

uint16_t to_cnt = 0;

// init I2C 
void init_i2c (void)
{
  // relax a bit to let bus lines settle, will prevent hanging after POR
  msleep (200);

  // setup I2C module
  init_i2c1 ();
}

// init I2C1 module
void init_i2c1 (void)
{
  // enable I2C1 clock
  RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

  // reset I2C block to resolve errors during POR
  I2C1->CR1 |=  I2C_CR1_SWRST;
  I2C1->CR1 &= ~I2C_CR1_SWRST;

  // disable I2C1
  I2C1->CR1 &= ~I2C_CR1_PE;

  // define APB clock frequency, no DMA, no interrupts
  I2C1->CR2 = 2;       // APB1CLK = 2Mc (is really 1.024Mc <> let's see if this works)

  // select Slow mode, duty cycle & set SCL frequency
  // note: 50% dutycycle -> T = (2*CCR)/APB1CLK -> CCR = 1.024Mc / 2*10kHz  = 50
//I2C1->CCR = 5;       // 100 kHz SCL @ 1.024Mc clock - hangs
//I2C1->CCR = 10;      // 50 kHz SCL @ 1.024Mc clock  - hangs
  I2C1->CCR = 50;      // 10 kHz SCL @ 1.024Mc clock  - slow clock seems ok

  // set SCL rise time 
  I2C1->TRISE = 10;    // seems ok @ 1.024Mc clock

  // enable I2C1
  I2C1->CR1 |= I2C_CR1_PE;
}

// I2C1 read
void i2c1_read (uint8_t adr, uint8_t buf[], uint8_t len)
{
  // generate I2C START condition
  I2C1->CR1 |= I2C_CR1_START;

  // wait for START condition generated
  while ((I2C1->SR1 & I2C_SR1_SB) == 0)
    ;

  // write I2C address
  I2C1->DR = adr;

  // wait for I2C address sent
  while ((I2C1->SR1 & I2C_SR1_ADDR) == 0)
    ;

  // read SR2 to clear ADDR bit
  (void)I2C1->SR2;

  // read bytes
  for (int i = 0; i < len; i++)
  {
    // enable ACK for first reads but disable for last read
    if (i < len-1)
      I2C1->CR1 |= I2C_CR1_ACK;  // enable ACK
    else
	    I2C1->CR1 &= ~I2C_CR1_ACK; // disable ACK
 
    // wait while busy
    while ((I2C1->SR1 & I2C_SR1_RXNE) == 0)
      ;

    // read data    
    buf[i] = I2C1->DR;
  }
}

// I2C1 read register
uint8_t i2c1_read_reg (uint8_t adr, uint8_t reg)
{
  uint8_t ret = 0;

  // generate I2C START condition
  I2C1->CR1 |= I2C_CR1_START;

  // wait for START condition generated
  to_cnt = 0;

  while ((I2C1->SR1 & I2C_SR1_SB) == 0)
  {
    if (to_cnt++ > I2C_TIMEOUT)
    {
      led_grn_flash (1);
      debug (I2C_ERR, "\E[35m#i2c1_read_reg: I2C ERROR\n\E[30m");
      return 0;
    }
    else
      usleep (I2C_WAITDELAY);
  }

  // -- first, write register address to be read --

  // write I2C write address (= first byte after START condition)
  I2C1->DR = adr | 0x00; 

  // wait for I2C address sent
  to_cnt = 0;

  while ((I2C1->SR1 & I2C_SR1_ADDR) == 0)
  {
    if (to_cnt++ > I2C_TIMEOUT)
    {
      led_grn_flash (2);
      debug (I2C_ERR, "\E[35m#i2c1_read_reg: I2C ERROR\n\E[30m");
      return 0;
    }
    else
      usleep (I2C_WAITDELAY);
  }

  // read SR2 to clear ADDR bit
  (void)I2C1->SR2;

  // write register number
  I2C1->DR = reg;

  // wait while busy
  to_cnt = 0;

  while ((I2C1->SR1 & I2C_SR1_TXE) == 0)
  {
    if (to_cnt++ > I2C_TIMEOUT)
    {
      led_grn_flash (4);
      debug (I2C_ERR, "\E[35m#i2c1_read_reg: I2C ERROR\n\E[30m");
      return 0;
    }
    else
      usleep (I2C_WAITDELAY);
  }

  // -- then, read register contents --

  // generate I2C RESTART condition
  I2C1->CR1 |= I2C_CR1_START;

  // wait for START condition generated
  to_cnt = 0;

  while ((I2C1->SR1 & I2C_SR1_SB) == 0)
  {
    if (to_cnt++ > I2C_TIMEOUT)
    {
      led_grn_flash (8);
      debug (I2C_ERR, "\E[35m#i2c1_read_reg: I2C ERROR\n\E[30m");
      return 0;
    }
    else
      usleep (I2C_WAITDELAY);
  }

  // write I2C read address (= first byte after START condition)
  I2C1->DR = adr | 0x01;

  // wait for I2C address sent
  to_cnt = 0;

  while ((I2C1->SR1 & I2C_SR1_ADDR) == 0)
  {
    if (to_cnt++ > I2C_TIMEOUT)
    {
      led_grn_flash (16);
      debug (I2C_ERR, "\E[35m#i2c1_read_reg: I2C ERROR\n\E[30m");
      return 0;
    }
    else
      usleep (I2C_WAITDELAY);
  }

  // read SR2 to clear ADDR bit
  (void)I2C1->SR2;

  // read single byte, so disable ACK
  I2C1->CR1 &= ~I2C_CR1_ACK; 
 
  // wait while busy
  to_cnt = 0;

  while ((I2C1->SR1 & I2C_SR1_RXNE) == 0)
  {
    if (to_cnt++ > I2C_TIMEOUT)
    {
      led_grn_flash (32);
      debug (I2C_ERR, "\E[35m#i2c1_read_reg: I2C ERROR\n\E[30m");
      return 0;
    }
    else
      usleep (I2C_WAITDELAY);
  }

  // read data    
  ret = I2C1->DR;

  // generate I2C STOP condition
  I2C1->CR1 |= I2C_CR1_STOP;

  // return data    
  return (ret);
}

// I2C1 write 
void i2c1_write (uint8_t adr, uint8_t buf[], uint8_t len)
{
  // generate I2C START condition
  I2C1->CR1 |= I2C_CR1_START;

  // wait for START condition generated
  while ((I2C1->SR1 & I2C_SR1_SB) == 0)
    ;

  // write I2C address
  I2C1->DR = adr;

  // wait for I2C address sent
  while ((I2C1->SR1 & I2C_SR1_ADDR) == 0)
    ;

  // read SR2 to clear ADDR bit
  (void)I2C1->SR2;

  // write bytes
  for (int i = 0; i < len; i++)
  {
    // write data
    I2C1->DR = buf[i];

    // wait while busy
    while ((I2C1->SR1 & I2C_SR1_TXE) == 0)
      ;
  }

  // wait for byte transmit finished
  while ((I2C1->SR1 & I2C_SR1_BTF) == 0) 
    ;

  // generate I2C STOP condition
  I2C1->CR1 |= I2C_CR1_STOP;
}

// I2C1 write register
void i2c1_write_reg (uint8_t adr, uint8_t reg, uint8_t dat)
{
  // generate I2C START condition
  I2C1->CR1 |= I2C_CR1_START;

  // wait for START condition generated
  to_cnt = 0;

  while ((I2C1->SR1 & I2C_SR1_SB) == 0)
  {
    if (to_cnt++ > I2C_TIMEOUT)
    {
      printf2 ("#i2c1_write_reg: I2C ERROR\n");
      return;
    }
    else
      usleep (50);
  }

  // -- first, write register address to be written --

  // write I2C write address (= first byte after START condition)
  I2C1->DR = adr | 0x00; 

  // wait for I2C address sent
  to_cnt = 0;

  while ((I2C1->SR1 & I2C_SR1_ADDR) == 0)
  {
    if (to_cnt++ > I2C_TIMEOUT)
    {
      printf2 ("#i2c1_write_reg: I2C ERROR\n");
      return;
    }
    else
      usleep (50);
  }

  // read SR2 to clear ADDR bit
  (void)I2C1->SR2;

  // write register number
  I2C1->DR = reg;

  // wait while busy
  to_cnt = 0;

  while ((I2C1->SR1 & I2C_SR1_TXE) == 0)
  {
    if (to_cnt++ > I2C_TIMEOUT)
    {
      printf2 ("#i2c1_write_reg: I2C ERROR\n");
      return;
    }
    else
      usleep (50);
  }

  // -- then, write register contents --

  // write data
  I2C1->DR = dat;

  // wait while busy
  to_cnt = 0;

  while ((I2C1->SR1 & I2C_SR1_TXE) == 0)
  {
    if (to_cnt++ > I2C_TIMEOUT)
    {
      printf2 ("#i2c1_write_reg: I2C ERROR\n");
      return;
    }
    else
      usleep (50);
  }

  // wait for byte transmit finished
  to_cnt = 0;

  while ((I2C1->SR1 & I2C_SR1_BTF) == 0) 
  {
    if (to_cnt++ > I2C_TIMEOUT)
    {
      printf2 ("#i2c1_write_reg: I2C ERROR\n");
      return;
    }
    else
      usleep (50);
  }

  // generate I2C STOP condition
  I2C1->CR1 |= I2C_CR1_STOP;
}

// dump I2C1 registers
void i2c1_dump_registers (void)
{
  printf2 ("I2C1_CR1    : 0x%08x\n", I2C1->CR1);
  printf2 ("I2C1_CR2    : 0x%08x\n", I2C1->CR2);
//printf2 ("I2C1_OAR1   : 0x%08x\n", I2C1->OAR1);
//printf2 ("I2C1_OAR2   : 0x%08x\n", I2C1->OAR2);
  printf2 ("I2C1_DR     : 0x%08x\n", I2C1->DR);
  printf2 ("I2C1_SR1    : 0x%08x\n", I2C1->SR1);
  printf2 ("I2C1_SR2    : 0x%08x\n", I2C1->SR2);
  printf2 ("I2C1_CCR    : 0x%08x\n", I2C1->CCR);
  printf2 ("I2C1_TRISE  : 0x%08x\n", I2C1->TRISE);
  printf2 ("\n");
}










