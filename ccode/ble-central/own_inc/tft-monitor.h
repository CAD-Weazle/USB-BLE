// file    : tft-monitor.h
// author  : rb
// purpose : header file for tft-monitor.c
// date    : 191121
// last    : 191214
//

#ifndef _TFT_MONITOR_H_
#define _TFT_MONITOR_H_

// -- prototypes
void init_mon_tft (void);

void tft_mon_panel (void);

void tft_mon_ambient (void);
void tft_mon_grobox  (void);
void tft_mon_water   (void);

void tft_mon_hmi (void);

void tft_mon_phack      (uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height);
void tft_mon_phprogress (uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height);

void tft_mon_ecack      (uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height);
void tft_mon_ecprogress (uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height);

#endif


