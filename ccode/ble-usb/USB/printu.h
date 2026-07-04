// file    : printf.h
// author  : ao/rb
// purpose : header file for printu.c
// date    : 000101
// last    : 220815
//

#ifndef _PRINTU_H_
#define _PRINTU_H_

int printu  (const char *fmt, ...);
int sprintu (char *buf, const char *fmt, ...);

int usb_debug (uint8_t do_print, const char *fmt, ...);

//int debug  (uint8_t do_print, const char *fmt, ...);

int putcharu (int c);
int putsu    (const char *s);

#endif




