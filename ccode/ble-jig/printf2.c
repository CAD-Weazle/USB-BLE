/*
 * printf implementation
 * Arlet Ottens, 2000.
 */

#define FLOAT

#include "includes.h"

#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include "printf2.h"

#ifdef FLOAT
#include "./cmath/dtoa.h"
#endif

// -- static prototypes
static int parsenum( const char **end );
static int vsprintf( char *s, const char *fmt, va_list args );
//static int sprintf2( char *buf, const char *fmt, ... );

enum
{
   ZEROPAD = 0x01
};

static int parsenum( const char **end )
{
   const char *s = *end;
   int c;
   int val = 0;

   while( (c = *s - '0') >= 0 && c <= 9 )
   {
      val = val * 10 + c;
      s++;
   }
   *end = s;
   return( val );
}

static int vsprintf( char *s, const char *fmt, va_list args )
{
   char c;
   const char *prefix, *p = s;
   char *arg;
   char buf[32];
   int flags, precision, len, width, zeropad, max;
   unsigned long val, base;

#ifdef FLOAT 
   double fval;  // rb
#endif

   while( (c = *fmt++) != '\0' )
   {
      if( c != '%' )
      {
         *s++ =  c;
         continue;
      }
      flags = zeropad = width = 0;
      precision = -1;
      max = INT_MAX;
      prefix = "";
      while( 1 )
      {
         c = *fmt;
         if( c == '0' )
            flags = ZEROPAD;
         else
            break;
         fmt++;
      }
      width = parsenum( &fmt );
      if( *fmt == '.' )
      {
         fmt++;
         precision = parsenum( &fmt );
      }
      if( *fmt == 'h' || *fmt == 'l' )
         fmt++;
      switch( (c = *fmt++) )
      {
         case 'u':
         case 'd':
            base = 10;
            break;

#ifdef FLOAT 
         case 'e':
         case 'f':
         case 'g':
            fval = va_arg( args, double );
            dtoa( buf, c, sizeof(buf), precision, fval );
            arg = buf;
            goto string;
#endif

         case 'p':
            precision = 2 * sizeof( void * );
         case 'X':
         case 'x':
            base = 16;
            break;

         case 'i':
            val = va_arg( args, unsigned long );
            sprintf2( buf, "%d.%d.%d.%d",  
                    (unsigned char) val, 
                    (unsigned char) (val >> 8), 
                    (unsigned char) (val >> 16), 
                    (unsigned char) (val >> 24) );
            arg = buf;
            goto string;

         case 's':
            arg = va_arg( args, char * );
            if( precision >= 0 )
               max = precision;
            goto string;

         case 'c':
            buf[0] = (char) va_arg( args, int );
            arg = buf;
            max = 1;
            goto string;

         default:
            *s++ = c;
            continue;
      }

      /*
       * turn number into string, without padding
       */

      val = (unsigned long) va_arg( args, int );
      if( c == 'd' && (long) val < 0 )
      {
         val = -val;
         prefix = "-";
      }

     arg = buf + sizeof( buf );
     *--arg = 0;
     while( val != 0 )
     {
        *--arg = "0123456789abcdef"[val % base];
        val /= base; 
     }
     if( precision >= 0 )
        zeropad = precision;
     else if( flags & ZEROPAD )
        zeropad = width - strlen( prefix );
     else
        zeropad = 1;

string:
      len = strnlen( arg, max );
      if( (zeropad -= len) < 0 )
         zeropad = 0;
      width -= len + zeropad + strlen( prefix );
      while( *prefix )
         *s++ = *prefix++;
      while( --zeropad >= 0 )
         *s++ = '0';
      while( --len >= 0 )
         *s++ = *arg++;
      while( --width >= 0 )
         *s++ = ' ';
   }
   *s = 0;
   return( s - p );
}

int sprintf2 (char *buf, const char *fmt, ...)
//static int sprintf2( char *buf, const char *fmt, ... )
{
  va_list args;
  int n;

  va_start (args, fmt);
  n = vsprintf (buf, fmt, args);
  va_end (args);

  return (n);
}

int printf2 (const char *fmt, ...)
{
  va_list args;
  static char buf[256];

  va_start (args, fmt);
  vsprintf (buf, fmt, args);
  va_end (args);

  uart2_puts (buf);
  
  return strlen (buf);
}

int debug (uint8_t do_print, const char *fmt, ...)
{
  if (do_print)
  {
    va_list args;
    static char buf[128];

    va_start (args, fmt);
    vsprintf (buf, fmt, args);
    va_end (args);
    uart2_puts (buf);

    return strlen (buf);
  }
  else
    return -1;
}

