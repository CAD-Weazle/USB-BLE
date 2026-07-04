// file    : uart3.c
// author  : ao/rb
// purpose : STM32L1xx UART3 routines
// date    : 170811
// last    : 200507
//

#include "includes.h"

// input/output FIFOs 
//static char uart1_rx_buf[RX3_SIZE] __attribute__ ((section(".bss.ccm")));
//static char uart1_tx_buf[TX3_SIZE] __attribute__ ((section(".bss.ccm")));

static char uart3_rx_buf[RX3_SIZE];
static char uart3_tx_buf[TX3_SIZE];

// FIFO indexes 
volatile static uint32_t tx_tail = 0;
volatile static uint32_t rx_head = 0;
         static uint32_t tx_head = 0;
         static uint32_t rx_tail = 0;

// USART3 ISR
void USART3_IRQHandler (void)
{
  int sr = USART3->SR;

  // read character
  if (sr & USART_SR_RXNE)
  {
    int c = USART3->DR;

    if ((int)(rx_head - rx_tail) < RX3_SIZE) 
      uart3_rx_buf[rx_head++ % RX3_SIZE] = c;
  }

  // write character
  if (sr & USART_SR_TXE)
  {
    if (tx_head == tx_tail)
      USART3->CR1 &= ~USART_CR1_TXEIE; 
    else
      USART3->DR = uart3_tx_buf[tx_tail++ % TX3_SIZE];
  }
}

// init USART3
void init_uart3 (int baud)
{
  // enable USART3 clock
  RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

  USART3->CR3 = 0;
  USART3->CR2 = 0;                             // 1 stop bit
  USART3->BRR = ((APB1CLK << 4) / baud) >> 3;  // fixed-point that shit (note: OVER8 = 1, so '>> 3')
  USART3->CR1 = USART_CR1_OVER8  |             // 8x oversampling, for low 1.024Mc SYSCLK
                USART_CR1_TE     | 
                USART_CR1_RE     | 
                USART_CR1_RXNEIE | 
                USART_CR1_UE;

  NVIC_EnableIRQ (USART3_IRQn);
}

// write character to USART3
void uart3_putc (int c)
{
  if (c == '\n')
    uart3_putc( '\r' );

  while ((int)(tx_head - tx_tail) >= TX3_SIZE)
    ;

  uart3_tx_buf[tx_head++ % TX3_SIZE] = c;

  USART3->CR1 |= USART_CR1_TXEIE;
}

int uart3_flush (void *dev)
{
  while (tx_head != tx_tail)
    ;

  return 0;
}

// write string to USART3
void uart3_puts (const char *s)
{
  int c;

  while ((c = *s++) != 0)
    uart3_putc (c);
}

// get character from USART3
int uart3_getc (void)
{
/*
  while (rx_head == rx_tail)
    ;

  return uart3_rx_buf[rx_tail++ % RX3_SIZE];
*/

  if (rx_head == rx_tail)
    return(-1);
  else
    return (uart3_rx_buf[rx_tail++ % RX3_SIZE]);
}

