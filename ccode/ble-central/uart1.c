// file    : uart1.c
// author  : ao/rb
// purpose : UART1 routines
// date    : 170811
// last    : 191130
//

#include "includes.h"

// input/output FIFOs 
//static char uart1_rx_buf[RX_SIZE] __attribute__ ((section(".bss.ccm")));
//static char uart1_tx_buf[TX_SIZE] __attribute__ ((section(".bss.ccm")));

static char uart1_rx_buf[RX_SIZE];
static char uart1_tx_buf[TX_SIZE];

// FIFO indexes 
volatile static uint32_t tx_tail = 0;
volatile static uint32_t rx_head = 0;
         static uint32_t tx_head = 0;
         static uint32_t rx_tail = 0;

#define rx_queued()  ( (int) (rx_head - rx_tail) )

// USART1 ISR
//void USART1_irq (void) __attribute__ ((section (".fast")));
void USART1_irq (void)
{
  int isr = USART1->SR;

  // read character
  if (isr & USART_SR_RXNE)
  {
    int c = USART1->DR;

    if ((int)(rx_head - rx_tail) < RX_SIZE) 
      uart1_rx_buf[rx_head++ % RX_SIZE] = c;
  }

  // write character
  if (isr & USART_SR_TXE)
  {
    if (tx_head == tx_tail)
      USART1->CR1 &= ~USART_CR1_TXEIE; 
    else
      USART1->DR = uart1_tx_buf[tx_tail++ % TX_SIZE];
  }
}

// init USART1
void init_uart1 (int baud)
{
  // enable USART1 clock
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

  USART1->CR3 = 0;
  USART1->CR2 = 0;                               // 1 stop bit
//USART1->BRR = APB2CLK / baud;                  // note:  fraction ignored, seems to work ok
  USART1->BRR = ((APB2CLK << 4) / baud) >> 4;    // fixed-point that shit
    
  // note: oversampling by 16
  USART1->CR1 = USART_CR1_TE     | 
                USART_CR1_RE     | 
                USART_CR1_RXNEIE | 
                USART_CR1_UE;

  // enable USART1 interrupt
  NVIC_EnableIRQ (USART1_IRQn);
}

// write character to USART1
void uart1_putc (int c)
{
  if (c == '\n')
    uart1_putc ('\r');

  while ((int)(tx_head - tx_tail) >= TX_SIZE)
    ;

  uart1_tx_buf[tx_head++ % TX_SIZE] = c;

  USART1->CR1 |= USART_CR1_TXEIE;
}

// write string to USART1
void uart1_puts (const char *s)
{
  int c;

  while ((c = *s++) != 0)
    uart1_putc (c);
}

// get character from USART1
int uart1_getc (void)
{
/*
  while (rx_head == rx_tail)
    ;

  return uart1_rx_buf[rx_tail++ % RX_SIZE];
*/
  if (rx_head == rx_tail)
    return -1;
  else
    return uart1_rx_buf[rx_tail++ % RX_SIZE];
}

// flush write buffers
int uart1_flush (void)
{
  while (tx_head != tx_tail)
    ;

  return 0;
}

/*
static int uart1_write (void *dev, const void *buf, int len)
{
  int retval = len;
  const char *s = buf;

  while (--len >= 0)
    uart1_putc (*s++);

  return retval;
}

static int uart1_read (void *dev, void *buf, int len)
{
  int retval = len;
  char *s = buf;

  while (--len >= 0)
    *s++ = uart1_getc ();

  return retval;
}
*/

