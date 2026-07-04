// file    : commands-hotbed.h
// author  : rb
// purpose : header file for commands-hotbed.c
// date    : 180111
// last    : 190114
//

#ifndef _COMMANDS_HOTBED_H_
#define _COMMANDS_HOTBED_H_

// --defines

// HotBed commands
#define HB_CMD_SETTEMP       0x20      // set setpoint HotBed heater [100*oC]
#define HB_CMD_GETTEMP       0x21      // get setpoint & actual HotBed temperature [100*oC]
#define HB_CMD_SETLEDS       0x22      // set bottom LEDs light intensity [0..100]
#define HB_CMD_SETFAN        0x23      // set fan speed [0..100]
#define HB_CMD_STARTPID      0x24      // start PID temperature control
#define HB_CMD_STOPPID       0x25      // stop PID temperature control
#define HB_CMD_STARTSHOW     0x26      // start bottom LEDs 'show'
#define HB_CMD_STOPSHOW      0x27      // stop bottom LEDs 'show'
#define HB_CMD_RTDERR        0x28      // write Pt100 calibration parameter
#define HB_CMD_GETSTATE      0x29      // get LEDs, fan, PID state
#define HB_CMD_GETPT100      0x2a      // get all Pt100 sensors, for error calibration only

// -- prototypes
void hb_set_temp (uint8_t badr, uint16_t val);
void hb_get_temp (uint8_t badr);

void hb_set_led (uint8_t badr, uint8_t val);
void hb_set_fan (uint8_t badr, uint8_t val);

void hb_start_pid (uint8_t badr);
void hb_stop_pid  (uint8_t badr);

void hb_start_show (uint8_t badr);
void hb_stop_show  (uint8_t badr);

void hb_set_rtd (uint8_t badr, uint8_t num, int16_t err);
void hb_get_rtd (uint8_t badr);

void hb_get_state (uint8_t badr);

#endif

