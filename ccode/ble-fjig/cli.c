// file    : cli.c
// author  : rb
// purpose : command line interface 
// date    : 081104
// last    : 200523
//

#include "includes.h"
#include <ctype.h>
#include <strings.h>

// globals
uint8_t buf_len;
char    buf[INBUF_LEN];

int   argc;
char *argv[10];

// print first prompt
void first_prompt (void)
{
  printf2 ("\n>");
}

// print prompt
void prompt (void)
{ 
  // normal prompt
  printf2 (">");
}

// convert string to integer
int rb_atoi (char *s)
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

// convert string to long
long rb_atol (char *s)
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

// skip white space
char *skipws (char *line)
{
  if( line != NULL )
  {
    while( *line == ' ' )
      line++;
  }
  return( line );
}

// break up line in words
int split_line (char *line, char **words)
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

// command parser
void parse_line (int argc, char **argv)
{
  if (argc)
  {
    // * FSC-BT630 Feasycom BLE Module commands
    if (!strcmp (argv[0], "at"))
      printf3 ("AT\n");

    if (!strcmp (argv[0], "sreset"))
    {
      printf2 ("#software reset BLE Module\n");
      printf3 ("AT+REBOOT\n");
    }

    if (!strcmp (argv[0], "hreset"))
    {  
      printf2 ("#hardware reset BLE Module\n");
      ble_reset ();
    }

    // get/set Advertising Interval
    if (!strcmp (argv[0], "advin"))
    {
      if (argc == 2)
      {
        uint16_t dat = rb_atoi (argv[1]);
        printf3 ("AT+ADVIN=%d\n", dat);
      }
      else
        printf3 ("AT+ADVIN\n");
    }

    if (!strcmp (argv[0], "disc"))
      printf3 ("AT+DISC\n");

    if (!strcmp (argv[0], "ver"))
      printf3 ("AT+VER\n");

    if (!strcmp (argv[0], "adr"))
      printf3 ("AT+ADDR\n");

    if (!strcmp (argv[0], "bname"))
      ble_dump_name ();

    if (!strcmp (argv[0], "name"))
    {
      // then send command
      if (argc == 2)
      {
        // make sure BLE Module is awake
        ble_at ();
        msleep (10);

        // set name
        printf3 ("AT+NAME=%s\n", argv[1]);
      }
      else
      {
        // get name
        printf3 ("AT+NAME\n");
      }
    }

    if (!strcmp (argv[0], "baud"))
      printf3 ("AT+BAUD\n");

    if (!strcmp (argv[0], "scan"))
    {
      printf2 ("#scan for BLE Devices\n");
      printf3 ("AT+SCAN=1,48\n");
    }

    if (!strcmp (argv[0], "role"))
      printf3 ("AT+ROLE\n");

    if (!strcmp (argv[0], "lpm_on"))
    {
      printf2 ("#enter Low Power Mode\n");
      ble_enter_lpm ();
      msleep (100);
      ble_reset ();
    }

    if (!strcmp (argv[0], "lpm_off"))
    {
      printf2 ("#leave Low Power Mode\n");
      ble_leave_lpm ();
      msleep (100);
      ble_reset ();
    }

    // ** misc. commands

    // power down ARM for current measurement
    if (!strcmp (argv[0], "pd"))
      power_down ();

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
  printf2 ("help              : this help\n");
  printf2 ("version           : version information\n");
}
     
// dump version info
void version (void)
{
  printf2 ("#- Feasycom BLE Jig - " VERSION " -\n");
}

// poll command line interface 
void poll_cli (void)
{
  // read next char
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


