// file    : ltc4150.h
// author  : rb
// purpose : header file for ltc4150.c
// date    : 200426
// last    : 200502
//

#ifndef _LTC4150_H_
#define _LTC4150_H_

// -- defines 
#define Rs                                    1.0  // value sense resistor [R]
#define dQ                           1/(Rs*32.55)  // charge increment per INT_L pulse [C]

#define LTC4150_CFILTERDEPTH                   16  // number samples points charge filter
#define LTC4150_IFILTERDEPTH                   16  // number samples points current filter
//#define LTC4150_CFILTERDEPTH                  128  // number samples points charge filter
//#define LTC4150_IFILTERDEPTH                  128  // number samples points current filter

// -- prototypes
void init_ltc4150 (void);

void ltc4150_handle (void);

float ltc4150_cfilter (float dat);
float ltc4150_ifilter (float dat);

void ltc4150_enable  (void);
void ltc4150_disable (void);
void ltc4150_clear   (void);

void ltc4150_dump (void);

#endif



