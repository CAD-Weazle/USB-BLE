// file    : uart1.h
// author  : rb
// purpose : header file for uart4.c
// date    : 170813
// last    : 191210
//

#ifndef _UART4_H_
#define _UART4_H_

// -- defines
#define RX4_SIZE                                128  // Tx buffer size
#define TX4_SIZE                                128  // Rx buffer size

// -- prototypes
void UART4_irq (void);

void init_uart4 (int baud);

void uart4_putc (int c);
void uart4_puts (const char *s);
int  uart4_getc (void);

int uart4_flush (void *dev);

#endif

