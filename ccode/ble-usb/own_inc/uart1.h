// file    : uart1.h
// author  : rb
// purpose : header file for uart1.c
// date    : 170813
// last    : 170813
//

// defines
#define RX1_SIZE           128
#define TX1_SIZE           128

//#define uart_puts(x)   uart1_puts(x)
//#define uart_putc(x)   uart1_putc(x)
//#define uart_flush(x)  uart1_flush(x)
//#define uart_getc()    uart1_getc()

// -- prototypes
void init_uart1 (int baud);

void uart1_putc (int c);
void uart1_puts (const char *s);
int  uart1_getc (void);

int uart1_flush (void *dev);


