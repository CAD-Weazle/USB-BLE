// file    : i2c.c
// author  : rb
// purpose : I2C routines STM32F4xx
// date    : 190701
// last    : 191210
//
// note    : during POR both SDA and SCL are pulled down for a brief moment,
//           this gives gezak in the I2C FSM, adding delay & clearing state is needed

#include "includes.h"

// init I2C 
void init_i2c (void)
{
  // relax a bit to let bus lines settle, will prevent hanging after POR
  msleep (200);

  // init I2C modules
//init_i2c1 ();
  init_i2c2 ();
//init_i2c3 ();
}

// init I2C2 module
void init_i2c2 (void)
{
  // enable I2C2 clock
  RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

  // reset I2C block to resolve errors during POR
  I2C2->CR1 |=  I2C_CR1_SWRST;
  I2C2->CR1 &= ~I2C_CR1_SWRST;

  // disable I2C2
  I2C2->CR1 &= ~I2C_CR1_PE;

  // define APB clock frequency, no DMA, no interrupts
  I2C2->CR2 = 42;       // APB1CLK = 42Mc (from 'pll.h')

  // select Slow mode, duty cycle & set SCL frequency to 100kHz (T = 1/100kHz)
  // note: 50% dutycycle -> T = (2*CCR)/APB1CLK -> CCR = 42Mc / 2*100kHz
  I2C2->CCR = 210;      // 100 kHz SCL

  // set SCL rise time 
  I2C2->TRISE = 42 + 1; // <> WTF???

  // enable I2C2
  I2C2->CR1 |= I2C_CR1_PE;
}

// I2C2 read
void i2c2_read (uint8_t adr, uint8_t buf[], uint8_t len)
{
  // generate I2C START condition
  I2C2->CR1 |= I2C_CR1_START;

  // wait for START condition generated
  while ((I2C2->SR1 & I2C_SR1_SB) == 0)
    ;

  // write I2C address
  I2C2->DR = adr;

  // wait for I2C address sent
  while ((I2C2->SR1 & I2C_SR1_ADDR) == 0)
    ;

  // read SR2 to clear ADDR bit
  (void)I2C2->SR2;

  // read bytes
  for (int i = 0; i < len; i++)
  {
    // enable ACK for first reads but disable for last read
    if (i < len-1)
      I2C2->CR1 |= I2C_CR1_ACK;  // enable ACK
    else
	    I2C2->CR1 &= ~I2C_CR1_ACK; // disable ACK
 
    // wait while busy
    while ((I2C2->SR1 & I2C_SR1_RXNE) == 0)
      ;

    // read data    
    buf[i] = I2C2->DR;
  }
}

// I2C2 write 
void i2c2_write (uint8_t adr, uint8_t buf[], uint8_t len)
{
  // generate I2C START condition
  I2C2->CR1 |= I2C_CR1_START;

  // wait for START condition generated
  while ((I2C2->SR1 & I2C_SR1_SB) == 0)
    ;

  // write I2C address
  I2C2->DR = adr;

  // wait for I2C address sent
  while ((I2C2->SR1 & I2C_SR1_ADDR) == 0)
    ;

  // read SR2 to clear ADDR bit
  (void)I2C2->SR2;

  // write bytes
  for (int i = 0; i < len; i++)
  {
    // write data
    I2C2->DR = buf[i];

    // wait while busy
    while ((I2C2->SR1 & I2C_SR1_TXE) == 0)
      ;
  }

  // wait for byte transmit finished
  while ((I2C2->SR1 & I2C_SR1_BTF) == 0) 
    ;

  // generate I2C STOP condition
  I2C2->CR1 |= I2C_CR1_STOP;
}

// dump I2C2 registers
void i2c2_dump_registers (void)
{
  printf ("I2C2_CR1    : 0x%08x\n", I2C2->CR1);
  printf ("I2C2_CR2    : 0x%08x\n", I2C2->CR2);
//printf ("I2C2_OAR1   : 0x%08x\n", I2C2->OAR1);
//printf ("I2C2_OAR2   : 0x%08x\n", I2C2->OAR2);
  printf ("I2C2_DR     : 0x%08x\n", I2C2->DR);
  printf ("I2C2_SR1    : 0x%08x\n", I2C2->SR1);
  printf ("I2C2_SR2    : 0x%08x\n", I2C2->SR2);
//printf ("I2C2_CCR    : 0x%08x\n", I2C2->CCR);
//printf ("I2C2_TRISE  : 0x%08x\n", I2C2->TRISE);
  printf ("\n");
}


///////////////////////////////////////

// init I2C1 module
void init_i2c1 (void)
{
  // enable I2C1 clock
  RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

  // disable I2C1
  I2C1->CR1 &= ~I2C_CR1_PE;

  // select Slow mode, duty cycle & set SCL frequency to 100kHz (T = 1/100kHz)
  // note: 50% dutycycle -> T = (2*CCR)/APB1CLK -> CCR = 42Mc / 2*100kHz
  I2C1->CCR = 210;      // 100 kHz SCL

  // set SCL rise time 
  I2C1->TRISE = 42 + 1; // <> WTF???

  // enable I2C1
  I2C1->CR1 |= I2C_CR1_PE;
}

// init I2C3 module
void init_i2c3 (void)
{
  // enable I2C3 clock
  RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;

  // disable I2C3
  I2C3->CR1 &= ~I2C_CR1_PE;

  // define APB clock frequency, no DMA, no interrupts
  I2C3->CR2 = 42;       // APB1CLK = 42 Mc (from 'pll.h')
  
  // select Slow mode, duty cycle & set SCL frequency to 100kHz (T = 1/100kHz)
  // note: 50% dutycycle -> T = (2*CCR)/APB1CLK -> CCR = 42Mc / 2*100kHz
  I2C3->CCR = 210;      // 100 kHz SCL

  // set SCL rise time 
  I2C3->TRISE = 42 + 1; // <> WTF???

  // enable I2C3
  I2C3->CR1 |= I2C_CR1_PE;
}

// I2C1 read
void i2c1_read (uint8_t adr, uint8_t buf[], uint8_t len)
{
}

// I2C1 write 
void i2c1_write (uint8_t adr, uint8_t buf[], uint8_t len)
{
}

// I2C3 read
void i2c3_read (uint8_t adr, uint8_t buf[], uint8_t len)
{
}

// I2C3 write 
void i2c3_write (uint8_t adr, uint8_t buf[], uint8_t len)
{
}

// dump I2C1 registers
void i2c1_dump_registers (void)
{
  printf ("I2C1_CR1    : 0x%08x\n", I2C1->CR1);
  printf ("I2C1_CR2    : 0x%08x\n", I2C1->CR2);
  printf ("I2C1_OAR1   : 0x%08x\n", I2C1->OAR1);
  printf ("I2C1_OAR2   : 0x%08x\n", I2C1->OAR2);
  printf ("I2C1_DR     : 0x%08x\n", I2C1->DR);
  printf ("I2C1_SR1    : 0x%08x\n", I2C1->SR1);
  printf ("I2C1_SR2    : 0x%08x\n", I2C1->SR2);
  printf ("I2C1_CCR    : 0x%08x\n", I2C1->CCR);
  printf ("I2C1_TRISE  : 0x%08x\n", I2C1->TRISE);
  printf ("\n");
}

// dump I2C3 registers
void i2c3_dump_registers (void)
{
  printf ("I2C3_CR1    : 0x%08x\n", I2C3->CR1);
  printf ("I2C3_CR2    : 0x%08x\n", I2C3->CR2);
  printf ("I2C3_OAR1   : 0x%08x\n", I2C3->OAR1);
  printf ("I2C3_OAR2   : 0x%08x\n", I2C3->OAR2);
  printf ("I2C3_DR     : 0x%08x\n", I2C3->DR);
  printf ("I2C3_SR1    : 0x%08x\n", I2C3->SR1);
  printf ("I2C3_SR2    : 0x%08x\n", I2C3->SR2);
  printf ("I2C3_CCR    : 0x%08x\n", I2C3->CCR);
  printf ("I2C3_TRISE  : 0x%08x\n", I2C3->TRISE);
  printf ("\n");
}


