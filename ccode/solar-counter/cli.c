// file    : cli.c
// author  : rb
// purpose : command line interface 
// date    : 081104
// last    : 200308
//

#include "includes.h"
#include <ctype.h>
#include <strings.h>

// globals
uint8_t do_log    = 0;    // logging flag
uint8_t do_send   = 0;    // <> remove later 
uint8_t pass_thru = 0;    // <> remove later 

uchar buf_len;
char  buf[INBUF_LEN];

int   argc;
char *argv[10];

void prompt (void)
{ 
  // normal prompt
  printf2 (">");
}

void first_prompt (void)
{
  printf2 ("\n>");
}

int rb_atoi( char *s )
{
  int i = 0, n, sign = 1;
  
  if( s[0] == '-' )
  {
    sign = -1;
    i++;
  }
  for( n = 0; s[i] >= '0' && s[i] <= '9'; i++ )
    n = 10 * n + s[i] - '0';
  
  return (sign * n);
}


long rb_atol( char *s )
{
  int i = 0, sign = 1;
  long n;
  
  if( s[0] == '-' )
  {
    sign = -1;
    i++;
  }
  
  for( n = 0; s[i] >= '0' && s[i] <= '9'; i++ )
    n = 10 * n + s[i] - '0';
  
  return( sign * n );
}


char *skipws( char *line )
{
  if( line != NULL )
  {
    while( *line == ' ' )
      line++;
  }
  return( line );
}

int split_line( char *line, char **words )
{
  int n = 0;
  char *sep = line;
  
  while( (line = skipws(sep)) != NULL && *line )
  {
    sep = strchr( line, ' ' );
    if (sep)
      *sep++ = 0;
    words[n++] = line;
  }
  return( n );
}

// parse CLI data from serial port
void parse_line( int argc, char **argv )
{
  if (argc)
  {
    // set dummy load PWM
    if (!strcmp (argv[0], "pwm"))
    {
      if (argc == 2)
      {
        uint8_t dat = rb_atoi (argv[1]);

        pwm_set (dat);
      }
    }

    // LTC2941 full charge
    if (!strcmp (argv[0], "full"))
      ltc2941_charge_complete ();
      
    // LTC2941 dump registers
    if (!strcmp (argv[0], "regs"))
      ltc2941_reg_dump  ();


    // clear LTC4150
    if (!strcmp (argv[0], "clr"))
      ltc4150_clear  ();

    // dump help data
    if (!strcmp (argv[0], "help"))
      help ();

    // dump version information
    if (!strcmp (argv[0], "version"))
      version ();
  }   
} 

// dump help
void help (void)
{
  printf2 ("log               : start logging to UART & sending data\n");
  printf2 ("q                 : stop logging to UART & sending data\n");
  printf2 ("help              : this help\n");
  printf2 ("version           : version information\n");
}
     
// dump version info
void version (void)
{
  printf2 ("#- Solar Counter - " VERSION " -\n");
}

// poll command line interface USART1
void poll_cli (void)
{
  int c = uart2_getc ();

  // test for Rx buffer empty
  if (c < 0)
    return;

  // add byte to buffer & parse
  if ((c == '\r') | (c == '\n'))       // check for enter to end string
  {
    printf2 ("\n");                    // print newline
  
    buf[buf_len] = '\0';               // /0 terminate buffer
    buf_len      = 0;                  // reset pointer
  
    argc = split_line (buf, argv);     // parse command
    parse_line (argc, argv);          
  
    prompt ();                         // display new prompt
  }
  else if (c == '\b' || c == 0x7f)     // check for backspace and delete
  {
    if (buf_len > 0)
      buf_len--;
  }
  else if (isprint(c) && buf_len < INBUF_LEN)
    buf[buf_len++] = c;
}


