// file    : ds3234.h
// author  : rb
// purpose : header file for ds3234.c
// date    : 190920
// last    : 191210
//

#ifndef _DS3234_H_
#define _DS3234_H_

#define DAY_SECS                       (24*60*60)  // number of seconds in a day (86400)

// -- globals
extern uint8_t time_hrs;
extern uint8_t time_min;
extern uint8_t time_sec;

// -- prototypes
void init_ds3234 (void);

void ds3234_update (void);

void ds3234_set_time (uint8_t hrs, uint8_t min, uint8_t sec);
void ds3234_get_time (void);

void ds3234_set_date (uint8_t yrs, uint8_t mnt, uint8_t day);
void ds3234_get_date (void);

void ds3234_dump_time (uint8_t nl);
void ds3234_dump_date (void);

void ds3234_timestamp (void);

void ds3234_sync (void);

uint8_t ds3234_midnight (void);
uint8_t ds3234_newhour  (void);

void ds3234_start_log (void);
void ds3234_stop_log  (void);

void ds3234_state_update (void);

void ds3234_dump_regs (void);

#endif



