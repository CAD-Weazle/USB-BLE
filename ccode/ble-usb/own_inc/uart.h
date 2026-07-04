/*
 * uart.h
 */

#define uart_puts(x)   uart2_puts(x)
#define uart_putc(x)   uart2_putc(x)
#define uart_flush(x)  uart2_flush(x)

int init_uart2( int baud );
int uart2_flush( void *dev );

void uart2_putc( int c );
void uart2_puts( const char *s );
int uart2_getc( void );
int uart2_getchar( void );
int uart2_flush( void *dev );
int uart2_read( void *dev, void *buf, int len );
int uart2_write( void *dev, const void *buf, int len );
int uart2_read_poll( char *buf, int max );
void uart2_wake_receive( void );
