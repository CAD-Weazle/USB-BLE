// file    : dump.c
// author  : rb
// purpose : dump ASCII in pretty hexdump format
// date    : 150714
// last    : 191213
//

#include "includes.h"

// formatted hex dump / ASCII dump
void dump_ascii (uint8_t *buf, uint16_t len)
{
  uint8_t  i;
  uint8_t  ready = 0;
  int16_t  pbuf[HEXLINE_LEN];          // print buffertje
  uint16_t tmp_len, tmp_ofs = 0;
  uint8_t  line_cnt = 0;

  tmp_len = len;

  while (!ready) 
  {
    // fill print buffer with 'empty' character
    for (i = 0; i < HEXLINE_LEN; i++)
      pbuf[i] = -1;

    // adjust length
    if (tmp_len > HEXLINE_LEN)
    {
      // copy one line of data to print buffer
      for (i = 0; i < HEXLINE_LEN; i++)
        pbuf[i] = (int16_t)buf[tmp_ofs + i];

      tmp_len -= HEXLINE_LEN;
      tmp_ofs += HEXLINE_LEN;
    }
    else
    {
      // copy remaining data to print buffer
      for (i = 0; i < tmp_len; i++)
        pbuf[i] = (int16_t)buf[tmp_ofs + i];
 
      ready = 1;
    }

    // print line number (leading # for gnuplot output)
  //printf ("#0x%04x - ", buf + line_cnt++ * HEXLINE_LEN);   // absolute addresses
  //printf ("#0x%04x - ", line_cnt++ * HEXLINE_LEN);         // relative addresses

    // print line of data in hex
  //for (i = 0; i < HEXLINE_LEN; i++)
    for (i = 0; i < len; i++)
    {
      if (i == (HEXLINE_LEN/2))
        printf ("- ");

      (pbuf[i] != -1) ? printf ("%02x ", (uint8_t)pbuf[i]) : printf ("-- ");
    }

    printf (": ");

    // print line of data in ASCII
    for (i = 0; i < HEXLINE_LEN; i++)
    {
      uint8_t ch = pbuf[i];

      if (ch == '*')  
        printf ("-");
      else if (isprint (ch))
        printf ("%c", ch);
      else
        printf (".");
    }

    printf ("\n");
  }
}
