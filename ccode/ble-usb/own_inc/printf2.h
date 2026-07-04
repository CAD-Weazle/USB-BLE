#ifndef printf2_h
#define printf2_h

#include <stdarg.h>

//int parsenum( const char **end );
//int vsprintf( char *s, const char *fmt, va_list args );
int sprintf2( char *buf, const char *fmt, ... );
int printf2( const char *fmt, ... );

//int debug (uint8_t do_print, const char *fmt, ...);

#endif




