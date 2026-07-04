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


void parse_line( int argc, char **argv )
{

  if (argc)
  {
    // ** BLE commands
    if (!strcmp (argv[0], "b_reset"))
      ble_reset ();

    if (!strcmp (argv[0], "b_cmd"))
      ble_cmd_mode ();

    if (!strcmp (argv[0], "b_dat"))
      ble_data_mode ();

    if (!strcmp (argv[0], "b_pd"))
      ble_power_down ();

    if (!strcmp (argv[0], "b_pu"))
      ble_power_up ();

    if (!strcmp (argv[0], "b_cl"))
      ble_connection_close ();

    if (!strcmp (argv[0], "b_adv"))
      ble_advertizing_start ();

    if (!strcmp (argv[0], "b_advo"))
      ble_advertizing_stop ();

    if (!strcmp (argv[0], "poll_on"))
      ble_poll_on ();

    if (!strcmp (argv[0], "poll_off"))
      ble_poll_off ();


    // ** misc. commands

    // <> test: power down ARM for current measurement
    if (!strcmp (argv[0], "pd"))
      power_down ();

    // start logging to UART
    if (!strcmp (argv[0], "start"))
      do_send = 1;

    // start logging to UART
    if (!strcmp (argv[0], "log"))
      do_log = 1;

    // stop logging to UART / pass-thru mode
    if (!strcmp (argv[0], "q"))
    {
      do_log    = 0;
      do_send   = 0;
    }

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
  printf2 ("- Solar BLE Jig - " VERSION " -\n");
}

// poll command line interface USART1
void poll_cli (void)
{
  int c = uart2_getc ();

  // test for Rx buffer empty
  if (c < 0)
    return;

  if (c == '~')
  {
    if (pass_thru == 0)
    {
      pass_thru = 1;

      printf2 ("\E[35m%s\E[30m\n", "enable pass-thru");
    }
    else
    {
      pass_thru = 0;

      printf2 ("\E[35m%s\E[30m\n", "leave pass-thru");
    }
  }

  if (pass_thru)
  {
    uart3_putc (c);
    uart2_putc (c); // <> dev only
  }
  else
  {
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
}


