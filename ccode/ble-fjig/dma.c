// file    : dma.c
// author  : rb
// purpose : ST32L1xx ARM DMA routines
// date    : 200229
// last    : 200303
//

#include "includes.h"

// setup DMA for ADC1 transfers
void init_dma_adc (uint32_t *adc_dat, uint8_t adc_num)
{ 
  // enable clock for DMA1
  RCC->AHBENR |= RCC_AHBENR_DMA1EN;
  (void) RCC->AHBENR;                             // dummy read, make sure register is written

  // disable DMA1 before setup
  DMA1_Channel1->CCR = 0;
 
  // setup DMA channel
  DMA1_Channel1->CPAR  = (uint32_t) &ADC1->DR;   // src address: ADC1 - from peripheral
  DMA1_Channel1->CMAR  = (uint32_t) adc_dat;     // dst address: SRAM - to memory
  DMA1_Channel1->CNDTR = adc_num;                // number of items
  DMA1_Channel1->CCR   = DMA_CCR_MSIZE_1 |       // 32-bit memory 
                         DMA_CCR_PSIZE_1 |       // 32-bit peripheral
                         DMA_CCR_MINC    |       // enable memory increment mode
                         DMA_CCR_CIRC;           // enable circular mode
 
//NVIC_EnableIRQ (DMA1_Channel1_IRQn);           // no interrupts

  DMA1_Channel1->CCR  |= DMA_CCR_EN;             // enable DMA
}

// enable DMA <> needed?
void dma_enable (void)
{
  DMA1_Channel1->CCR  |= DMA_CCR_EN;             
}

// disable DMA, save power during Stop mode <> needed?
void dma_disable (void)
{
  DMA1_Channel1->CCR  &= ~DMA_CCR_EN;       
  RCC->AHBENR &= ~RCC_AHBENR_DMA1EN;
}


// dump DMA registers
void dma_dump_registers (void)
{
  printf2 ("-- common DMA registers --\n");
  printf2 ("DMA_ISR        : %08lx\n", DMA1->ISR);
  printf2 ("DMA_IFCR       : %08lx\n", DMA1->IFCR);
  printf2 ("-- DMA1 Channel1 registers --\n");
  printf2 ("DMA1_CH1_CCR   : %08lx\n", DMA1_Channel1->CCR);
  printf2 ("DMA1_CH1_CNDTR : %08lx\n", DMA1_Channel1->CNDTR);
  printf2 ("DMA1_CH1_CPAR  : %08lx\n", DMA1_Channel1->CPAR);
  printf2 ("DMA1_CH1_CMAR  : %08lx\n", DMA1_Channel1->CMAR);
}


