// file    : uart3.h
// author  : rb
// purpose : header file for uart3.c
// date    : 170813
// last    : 200307
//

// -- prototypes
void init_uart3 (int baud);

void uart3_putc (int c);
void uart3_puts (const char *s);
int  uart3_getc (void);

int uart3_flush (void *dev);


