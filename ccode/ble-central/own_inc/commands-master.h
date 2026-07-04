// file    : command-sunweb.h
// author  : rb
// purpose : header file for commands-sunweb.c
// date    : 190115
// last    : 190115
//

#ifndef _COMMANDS_SUNWEB_H_
#define _COMMANDS_SUNWEB_H_

// -- defines

// SunWeb commands
#define SW_CMD_SETBUSID      0xb0      // set SunWeb Device Bus ID
#define SW_CMD_SYNCRTC       0xb1      // sync ARM RTC at midnight 00:00:00
#define SW_CMD_SETRTC        0xb2      // set ARM RTC 

// -- prototypes
void sw_set_busid (uint8_t badr);

void sw_probe   (uint8_t badr);
void sw_release (uint8_t badr);

void sw_set_clock  (uint8_t badr);
void sw_sync_clock (uint8_t badr);

#endif


















