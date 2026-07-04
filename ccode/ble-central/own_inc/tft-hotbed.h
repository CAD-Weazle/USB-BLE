// file    : tft-hotbed.h
// author  : rb
// purpose : header file for tft-hotbed.c
// date    : 191121
// last    : 191210
//

#ifndef _TFT_HOTBED_H_
#define _TFT_HOTBED_H_

// -- prototypes
void init_hb_tft (void);

void tft_hb_panel (void);
void tft_hb_hmi   (void);

void tft_hb_heaterpid (void);
void tft_hb_ledfans   (void);
void tft_hb_waterbars (void);

#endif


