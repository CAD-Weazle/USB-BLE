#ifndef printf4_h
#define printf4_h

#include <stdarg.h>

//int parsenum( const char **end );
//int vsprintf( char *s, const char *fmt, va_list args );
//int sprintf( char *buf, const char *fmt, ... );

int printf4 (const char *fmt, ...);
int debug4  (uint8_t do_print, const char *fmt, ...);

#endif




