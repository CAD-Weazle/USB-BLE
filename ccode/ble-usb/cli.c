// file    : cli.c
// author  : rb
// purpose : command line interface 
// date    : 081104
// last    : 200523
//

#include "includes.h"
#include <ctype.h>
#include <strings.h>

#define INBUF_LEN RX1_SIZE

// globals
uint8_t buf_len;
char    buf[INBUF_LEN];

int   argc;
char *argv[10];

uint8_t do_log = 0;

// print first prompt
void first_prompt (void)
{
  printu ("\n>");
}

// print prompt
void prompt (void)
{ 
  // normal prompt
  printu ("\r>");
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
    // ** Feasycom BLE commands
    if (!strcmp (argv[0], "at"))
    {
      printf  ("AT\n");
      printf2 ("AT\n");
    }

    if (!strcmp (argv[0], "rst"))
      printf2 ("AT+REBOOT\n");

    if (!strcmp (argv[0], "hrst"))
      ble_reset ();

    if (!strcmp (argv[0], "disc"))
      printf2 ("AT+DISC\n");

    if (!strcmp (argv[0], "ver"))
      printf2 ("AT+VER\n");

    if (!strcmp (argv[0], "adr"))
      printf2 ("AT+ADDR\n");

    if (!strcmp (argv[0], "name"))
      printf2 ("AT+NAME\n");

    if (!strcmp (argv[0], "baud"))
      printf2 ("AT+BAUD\n");

    if (!strcmp (argv[0], "scan"))
      printf2 ("AT+SCAN=1,48\n");

    if (!strcmp (argv[0], "role"))
      printf2 ("AT+ROLE\n");

    if (!strcmp (argv[0], "stat"))
      ble_dump_stats ();

    if (!strcmp (argv[0], "con"))
      ble_connect ("DAC8F9C0EEB9");

  
    // start logging to Teraterm
    if (!strcmp (argv[0], "log"))
      do_log = 1;

    // stop logging to Teraterm
    if (!strcmp (argv[0], "q"))
      do_log = 0;

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
  printu ("help              : this help\n\r");
  printu ("version           : version information\n\r");
}
     
// dump version info
void version (void)
{
  printu ("#- BLE-USB - " VERSION " -\n");
}

// poll command line interface 
void poll_cli (void)
{
  // read next char
  int c = uart1_getc ();

  // test for Rx buffer empty
  if (c < 0)
    return;

  // add byte to buffer & parse
  if ((c == '\r') | (c == '\n'))       // check for enter to end string
  {
    printf ("\n");                     // print newline
  
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


// poll command line interface via USB
void poll_usb (void)
{
  if (!cli_new)
   return;

  printf ("%s\n\r", cli_buf); // <> debug

  printu ("\n\r");                     // print newline
									   
  argc = split_line (cli_buf, argv);   // parse command
  parse_line (argc, argv);             
									   
  prompt ();                           // display new prompt

  cli_cnt = 0;
  cli_new = 0;
}







