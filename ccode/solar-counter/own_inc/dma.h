// file    : dma.h
// author  : rb
// purpose : header file for dma.c
// date    : 200229
// last    : 200303
//

#ifndef _DMA_H_
#define _DMA_H_

// -- prototypes
void init_dma_adc (uint32_t *adc_dat, uint8_t adc_num);

void dma_enable  (void);
void dma_disable (void);

void dma_dump_registers (void);

#endif
