// file    : cli.h
// author  : rb
// purpose : header file for cli.c
// date    : 081104
// last    : 190130

#ifndef _CLI_H_
#define _CLI_H_

// -- globals
extern uint8_t co2_log;

// -- prototypes
void parse_line (int argc, char **argv);
int  split_line (char *line, char **words);

char *skipws (char *line);

int      rb_atoi (char *s);
long     rb_atol (char *s);
uint32_t rb_atoh (char *s);

void poll_cli     (void);
void first_prompt (void);
void prompt       (void);

void version (void);
void help    (void);

#endif






