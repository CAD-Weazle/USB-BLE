// file    : commands-hotbed.c
// author  : rb
// purpose : SunWeb HotBed commands
// date    : 190114
// last    : 190904
//

#include "includes.h"

#define HB_DEBUG_CMDS                  0    // debug command processing

// set HotBed target temperatures [10*oC]
void hb_set_temp (uint8_t badr, uint16_t val)
{
  uint8_t buf[2];

  // convert two double decimal & adjust endianness 
  buf[0] = (uint8_t)(((val*10) & 0xff00) >> 8);
  buf[1] = (uint8_t) ((val*10) & 0x00ff);

  // send SunWeb packet
  sw_txpack (badr, HB_CMD_SETTEMP, 2, buf);
  sw_txsend ();

  debug (HB_DEBUG_CMDS, "> SETTEMP      HotBed 0x%02x, %d\n", badr, val);
}

// get HotBed actual temperatures [100*oC]
void hb_get_temp (uint8_t badr)
{
  sw_txpack (badr, HB_CMD_GETTEMP, 0, NULL);
  sw_txsend ();

  debug (HB_DEBUG_CMDS, "> GETTEMP      HotBed 0x%02x\n", badr);
}

// set HotBed bottom LEDs intensity [0..100% PWM]
void hb_set_led (uint8_t badr, uint8_t val)
{
  sw_txpack (badr, HB_CMD_SETLEDS, 1, &val);
  sw_txsend ();

  debug (HB_DEBUG_CMDS, "> SETLED       HotBed 0x%02x, %d\n", badr, val);
}

// set HotBed fan speed [0..100% PWM]
void hb_set_fan (uint8_t badr, uint8_t val)
{
  sw_txpack (badr, HB_CMD_SETFAN, 1, &val);
  sw_txsend ();

  debug (HB_DEBUG_CMDS, "> SETFAN       HotBed 0x%02x, %d\n", badr, val);
}

// start HotBed PID temperature control
void hb_start_pid (uint8_t badr)
{
  sw_txpack (badr, HB_CMD_STARTPID, 0, NULL);
  sw_txsend ();

  debug (HB_DEBUG_CMDS, "> STARTPID     HotBed 0x%02x\n", badr);
}

// stop HotBed PID temperature control
void hb_stop_pid (uint8_t badr)
{
  sw_txpack (badr, HB_CMD_STOPPID, 0, NULL);
  sw_txsend ();

  debug (HB_DEBUG_CMDS, "> STOPPID      HotBed 0x%02x\n", badr);
}

// start HotBed bottom LEDs 'show'
void hb_start_show (uint8_t badr)
{
  sw_txpack (badr, HB_CMD_STARTSHOW, 0, NULL);
  sw_txsend ();

  debug (HB_DEBUG_CMDS, "> STARTSHOW    HotBed 0x%02x\n", badr);
}

// stop HotBed bottom LEDs 'show'
void hb_stop_show (uint8_t badr)
{
  sw_txpack (badr, HB_CMD_STOPSHOW, 0, NULL);
  sw_txsend ();

  debug (HB_DEBUG_CMDS, "> STOPSHOW     HotBed 0x%02x\n", badr);
}

// set HotBed Pt100 offset errors [10*oC]
void hb_set_rtd (uint8_t badr, uint8_t num, int16_t err)
{
  uint8_t buf[3];

  buf[0] = num;
  buf[1] = (int8_t)((err*10 & 0xff00) >> 8);
  buf[2] = (int8_t) (err*10 & 0x00ff);

  // broadcast SETTIME packet to all devices
  sw_txpack (badr, HB_CMD_RTDERR, 3, buf);
  sw_txsend ();

  debug (HB_DEBUG_CMDS, "> SETRTD       HotBed 0x%02x, %d %d \n", badr, num, err);
}

// get HotBed Pt100 temperatures (for offset error calibration only)
void hb_get_rtd (uint8_t id)
{
  sw_txpack (id, HB_CMD_GETPT100, 0, NULL);
  sw_txsend ();

  debug (HB_DEBUG_CMDS, "> GETRTD       HotBed 0x%02x\n", id);
}

// get HotBed state: LED intensity, fan speed & PID active state
void hb_get_state (uint8_t badr)
{
  sw_txpack (badr, HB_CMD_GETSTATE, 0, NULL);
  sw_txsend ();

  debug (HB_DEBUG_CMDS, "> GETSTATE     HotBed 0x%02x\n", badr);
}


