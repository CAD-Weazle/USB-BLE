// file    : uart1.h
// author  : rb
// purpose : header file for uart1.c
// date    : 170813
// last    : 191210
//

#ifndef _UART1_H_
#define _UART1_H_

// -- defines
#define RX_SIZE                                 128  // Rx buffer size
#define TX_SIZE                                 128  // Tx buffer size
#define INBUF_LEN                           RX_SIZE  // for legacy 'cli.c' code

// -- prototypes
void USART1_irq (void);

void init_uart1 (int baud);

void uart1_putc (int c);
void uart1_puts (const char *s);
int  uart1_getc (void);

int uart1_flush (void);

#endif


