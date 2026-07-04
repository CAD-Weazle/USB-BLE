// file    : spi.c
// author  : ao/rb
// purpose : SPI port routines
// date    : 170810
// last    : 190802
//
// note: Fpclk = 84Mc

#include "includes.h"

// set up SPI
void init_spi (void)
{
//spi1_init ();
//spi2_init ();
  spi3_init ();
}

// init SPI1
void spi1_init (void)
{
  // enable clock
  RCC->APB2ENR |= RCC_APB2Periph_SPI1;

  // set up SPI1
  SPI1->CR1 = SPI_CR1_SPE  |    // SPI enable
              SPI_CR1_MSTR |    // SPI master mode
              SPI_CR1_SSM  |    // must be set (??)
              SPI_CR1_SSI  |    // must be set (??)
            //SPI_CR1_BR_2 |    // baud rate control: 0b010 = Fpclk/8
              SPI_CR1_BR_1 |
            //SPI_CR1_BR_0 |
            //SPI_CR1_POL  |    // 0: SCK lo when idle / 1: SCK hi when idle
              SPI_CR1_CPHA;     // 0: first SCK edge captures / 1: second SCK edge captures
}

// init SPI2
void spi2_init (void)
{
  // enable clock
  RCC->APB1ENR |= RCC_APB1Periph_SPI2;

  // set up SPI2 
  SPI2->CR1 = SPI_CR1_SPE  |    // SPI enable
              SPI_CR1_MSTR |    // SPI master mode
              SPI_CR1_SSM  |    // must be set (??)
              SPI_CR1_SSI  |    // must be set (??)
            //SPI_CR1_BR_2 |    // baud rate control: 0b010 = Fpclk/8
              SPI_CR1_BR_1 |
            //SPI_CR1_BR_0 |
            //SPI_CR1_POL  |    // 0: SCK lo when idle / 1: SCK hi when idle
              SPI_CR1_CPHA;     // 0: first SCK edge captures / 1: second SCK edge captures
}

// init SPI3
void spi3_init (void)
{
  // enable clock
  RCC->APB1ENR |= RCC_APB1Periph_SPI3;

  // set up SPI3
  SPI3->CR1 = SPI_CR1_SPE  |    // SPI enable
              SPI_CR1_MSTR |    // SPI master mode
              SPI_CR1_SSM  |    // must be set (??)
              SPI_CR1_SSI  |    // must be set (??)
            //SPI_CR1_BR_2 |    // baud rate control: 0b111 = Fpclk/256 = 328kHz
              SPI_CR1_BR_1 ;
            //SPI_CR1_BR_0 ;    // baud rate control: 0b010 = Fpclk/8   = 10.5MHz
}

// SPI1 8-bit read/write  
uint8_t spi1_rw_byte (uint8_t data)
{
  SPI1->DR = data;

  while (!(SPI1->SR & SPI_SR_RXNE))
    continue;

  return (SPI1->DR);
}

// SPI1 8-bit write
void spi1_write_byte (uint8_t data)
{
  spi1_rw_byte (data);
}

// SPI1 8-bit ready
uint8_t spi1_read_byte  (void)
{
  return spi1_rw_byte (0x00);
}

// SPI2 8-bit read/write  
uint8_t spi2_rw_byte (uint8_t data)
{
  SPI2->DR = data;

  while (!(SPI2->SR & SPI_SR_RXNE))
    continue;

  return (SPI2->DR);
}

// SPI2 8-bit write
void spi2_write_byte (uint8_t data)
{
  spi2_rw_byte (data);
}

// SPI3 8-bit ready
uint8_t spi2_read_byte  (void)
{
  return spi2_rw_byte (0x00);
}

// SPI3 8-bit read/write  
uint8_t spi3_rw_byte (uint8_t data)
{
  SPI3->DR = data;

  while (!(SPI3->SR & SPI_SR_RXNE))
    continue;

  return (SPI3->DR);
}

// SPI3 8-bit write
void spi3_write_byte (uint8_t data)
{
  spi3_rw_byte (data);
}

// SPI3 8-bit ready
uint8_t spi3_read_byte  (void)
{
  return spi3_rw_byte (0x00);
}

// set SPI3 clock mode
void spi3_set_mode (uint8_t type)
{
  // change SPI clock mode
  switch (type)
  {
    // set up SPI3 for DS3234
    case SPI_DS3234_MODE:
    {
      // first disable SPI module
      SPI3->CR1 = 0x00;

      // set up SPI3
      SPI3->CR1 = SPI_CR1_SPE  |    // SPI enable
                  SPI_CR1_MSTR |    // SPI master mode
                  SPI_CR1_SSM  |    // must be set (??)
                  SPI_CR1_SSI  |    // must be set (??)
                  SPI_CR1_BR_2 |    // baud rate control: 0b100 = Fpclk/32 = 2.625Mc 
                //SPI_CR1_BR_1 |
                //SPI_CR1_BR_0 |
                  SPI_CR1_CPHA;     // 0: first SCK edge captures / 1: second SCK edge captures

      break;
    }

    // set up SPI3 for EVE TFT
    case SPI_TFT_MODE:
    {
      // first disable SPI module
      SPI3->CR1 = 0x00;
/*
      // set up SPI3
      SPI3->CR1 = SPI_CR1_SPE  |    // SPI enable
                  SPI_CR1_MSTR |    // SPI master mode
                  SPI_CR1_SSM  |    // must be set (??)
                  SPI_CR1_SSI  |    // must be set (??)
                  SPI_CR1_BR_2 |    // baud rate control: 0b111 = Fpclk/? <> increase this to < 30Mc
                  SPI_CR1_BR_1 |
                  SPI_CR1_BR_0 ;
*/
      // set up SPI3
      SPI3->CR1 = SPI_CR1_SPE  |    // SPI enable
                  SPI_CR1_MSTR |    // SPI master mode
                  SPI_CR1_SSM  |    // must be set (??)
                  SPI_CR1_SSI  |    // must be set (??)
                //SPI_CR1_BR_2 |    // baud rate control: 0b111 = Fpclk/256 = 328kHz
                  SPI_CR1_BR_1 ;
                //SPI_CR1_BR_0 ;    // baud rate control: 0b010 = Fpclk/8   = 10.5MHz
      break;
    }

    // set up SPI3 for 25AA040 EEPROM
    case SPI_EEPROM_MODE:
    {
      // first disable SPI module
      SPI3->CR1 = 0x00;

      // set up SPI3
      SPI3->CR1 = SPI_CR1_SPE  |    // SPI enable
                  SPI_CR1_MSTR |    // SPI master mode
                  SPI_CR1_SSM  |    // must be set (??)
                  SPI_CR1_SSI  |    // must be set (??)
                  SPI_CR1_BR_2 ;    // baud rate control: 0b100 = Fpclk/32 =  2.625Mc
                //SPI_CR1_BR_1 |
                //SPI_CR1_BR_0 ;

      break;
    }

    default:
      break;
  }
}

