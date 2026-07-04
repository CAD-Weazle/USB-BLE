// file    : uart2.c
// author  : ao/rb
// purpose : STM32L1xx UART2 routines
// date    : 170811
// last    : 200313
//

#include "includes.h"

// input/output FIFOs 
//static char uart2_rx_buf[RX2_SIZE] __attribute__ ((section(".bss.ccm")));
//static char uart2_tx_buf[TX2_SIZE] __attribute__ ((section(".bss.ccm")));

static char uart2_rx_buf[RX2_SIZE];
static char uart2_tx_buf[TX2_SIZE];

// FIFO indexes 
volatile static uint32_t tx_tail = 0;
volatile static uint32_t rx_head = 0;
         static uint32_t tx_head = 0;
         static uint32_t rx_tail = 0;

// USART2 ISR
void USART2_IRQHandler (void)
{
  int sr = USART2->SR;

  // read character
  if (sr & USART_SR_RXNE)
  {
    int c = USART2->DR;

    if ((int)(rx_head - rx_tail) < RX2_SIZE) 
      uart2_rx_buf[rx_head++ % RX2_SIZE] = c;
  }

  // write character
  if (sr & USART_SR_TXE)
  {
    if (tx_head == tx_tail)
      USART2->CR1 &= ~USART_CR1_TXEIE; 
    else
      USART2->DR = uart2_tx_buf[tx_tail++ % TX2_SIZE];
  }
}

// init USART2
void init_uart2 (int baud)
{
  // enable USART2 clock
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

  USART2->CR3 = 0;
  USART2->CR2 = 0;                        // 1 stop bit
  USART2->BRR = (6 << 4) | 0b1101;        // 9600 Baud @ 1.024 Mc SYSCLK
  USART2->CR1 = USART_CR1_TE     | 
                USART_CR1_RE     | 
                USART_CR1_RXNEIE | 
                USART_CR1_UE;

  NVIC_EnableIRQ (USART2_IRQn);
}

// write character to USART2
void uart2_putc (int c)
{
  if (c == '\n')
    uart2_putc ('\r');

  while ((int)(tx_head - tx_tail) >= TX2_SIZE)
    ;

  uart2_tx_buf[tx_head++ % TX2_SIZE] = c;

  USART2->CR1 |= USART_CR1_TXEIE;
}

int uart2_flush (void *dev)
{
  while (tx_head != tx_tail)
    ;

  return 0;
}

// write string to USART2
void uart2_puts (const char *s)
{
  int c;

  while ((c = *s++) != 0)
    uart2_putc (c);
}

// get character from USART2
int uart2_getc (void)
{
  if (rx_head == rx_tail)
    return(-1);
  else
    return (uart2_rx_buf[rx_tail++ % RX2_SIZE]);
}

