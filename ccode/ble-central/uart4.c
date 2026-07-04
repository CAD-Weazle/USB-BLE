// file    : uart4.c
// author  : ao/rb
// purpose : UART4 routines
// date    : 170811
// last    : 190712
//

#include "includes.h"

// input/output FIFOs 
//static char uart4_rx_buf[RX4_SIZE] __attribute__ ((section(".bss.ccm")));
//static char uart4_tx_buf[TX4_SIZE] __attribute__ ((section(".bss.ccm")));

static char uart4_rx_buf[RX4_SIZE];
static char uart4_tx_buf[TX4_SIZE];

// FIFO indexes 
volatile static uint32_t tx_tail = 0;
volatile static uint32_t rx_head = 0;
         static uint32_t tx_head = 0;
         static uint32_t rx_tail = 0;

#define rx_queued()  ( (int) (rx_head - rx_tail) )

// UART4 ISR
//void UART4_irq (void) __attribute__ ((section (".fast")));
void UART4_irq (void)
{
  int isr = UART4->SR;

  // read character
  if (isr & USART_SR_RXNE)
  {
    int c = UART4->DR;

    if ((int)(rx_head - rx_tail) < RX_SIZE) 
      uart4_rx_buf[rx_head++ % RX_SIZE] = c;
  }

  // write character
  if (isr & USART_SR_TXE)
  {
    if (tx_head == tx_tail)
      UART4->CR1 &= ~USART_CR1_TXEIE; 
    else
      UART4->DR = uart4_tx_buf[tx_tail++ % TX_SIZE];
  }
}

// init UART4
void init_uart4 (int baud)
{
  // enable UART4 clock
  RCC->APB1ENR |= RCC_APB1ENR_UART4EN;

  UART4->CR3 = 0;
  UART4->CR2 = 0;                                // 1 stop bit
//UART4->BRR = APB1CLK / baud;                   // note:  fraction ignored, seems to work ok
  UART4->BRR = ((APB1CLK << 4) / baud) >> 4;     // fixed-point that shit

  // note: oversampling by 16
  UART4->CR1 = USART_CR1_TE     | 
               USART_CR1_RE     | 
               USART_CR1_RXNEIE | 
               USART_CR1_UE;

  // enable UART4 interrupt
  NVIC_EnableIRQ (UART4_IRQn);
}

// write character to UART4
void uart4_putc (int c)
{
  if (c == '\n')
    uart4_putc ('\r');

  while ((int)(tx_head - tx_tail) >= TX_SIZE)
    ;

  uart4_tx_buf[tx_head++ % TX_SIZE] = c;

  UART4->CR1 |= USART_CR1_TXEIE;
}

// write string to USART1
void uart4_puts (const char *s)
{
  int c;

  while ((c = *s++) != 0)
    uart4_putc (c);
}

// get character from USART1
int uart4_getc (void)
{
/*
  while (rx_head == rx_tail)
    ;

  return uart4_rx_buf[rx_tail++ % RX_SIZE];
*/
  if (rx_head == rx_tail)
    return -1;
  else
    return uart4_rx_buf[rx_tail++ % INBUF_LEN];
}

// flush write buffers
int uart4_flush (void *dev)
{
  while (tx_head != tx_tail)
    ;

  return 0;
}

