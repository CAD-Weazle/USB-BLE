// file    : dump.h
// author  : rb
// purpose : header file for dump.c
// date    : 150714
// last    : 150720

#ifndef _DUMP_H_
#define _DUMP_H_

#define HEXLINE_LEN          16        // width of hexdump 

void dump_ascii   (uint8_t *buf, uint16_t len);

void dump_buffer  (uint8_t *buf, uint8_t len);
void dump_buffers (void);

#endif



