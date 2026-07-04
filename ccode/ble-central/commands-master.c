// file    : commands-master.c
// author  : rb
// purpose : SunWeb Master commands
// date    : 190115
// last    : 190115
//

#include "includes.h"

#define SX_DEBUG_CMDS                  0    // debug command processing

// set SunWeb Device Bus ID
void sw_set_busid (uint8_t badr)
{
  // note: only needed during install of netwerk, ID is stored locally in FLASH
  sw_txpack (SW_ID_BROADCAST, SW_CMD_SETBUSID, 1, &badr);
  sw_txsend ();

  debug (SX_DEBUG_CMDS, "> SETBUSID 0x%02x broadcast\n", badr);
}

// probe SunWeb Device
void sw_probe (uint8_t badr)
{
  sw_txpack (badr, SW_CMD_PROBE, 0, NULL);
  sw_txsend ();

  if ((badr & 0xf8) == SW_ID_BCASTLEDS)
    debug (SX_DEBUG_CMDS, "> PROBE        SimSun 0x%02x\n", badr);
  else if ((badr & 0xf8) == SW_ID_BCASTHEAT)
    debug (SX_DEBUG_CMDS, "> PROBE        HotBed 0x%02x\n", badr);
  else
    debug (SX_DEBUG_CMDS, "> PROBE        Device 0x%02x\n", badr);
}

// release SunWeb Device
void sw_release (uint8_t badr)
{
  sw_txpack (badr, SW_CMD_RELEASE, 0, NULL);
  sw_txsend ();

  if ((badr & 0xf8) == SW_ID_BCASTLEDS)
    debug (SX_DEBUG_CMDS, "> RELEASE      SimSun 0x%02x\n", badr);
  else if ((badr & 0xf8) == SW_ID_BCASTHEAT)
    debug (SX_DEBUG_CMDS, "> RELEASE      HotBed 0x%02x\n", badr);
  else
    debug (SX_DEBUG_CMDS, "> RELEASE      Device 0x%02x\n", badr);
}

// set RTC SunWeb Devices
void sw_set_clock (uint8_t badr)
{
  uint8_t buf[3];

  // get current time
  ds3234_get_time ();

  // time stored in globals
  buf[0] = time_hrs;
  buf[1] = time_min;
  buf[2] = time_sec;

  // broadcast time to all devices
  sw_txpack (badr, SW_CMD_SETRTC, 3, buf);
  sw_txsend ();

  // note: time is in BCD format, print as hex, not decimal
  if ((badr & 0xf8) == SW_ID_BCASTLEDS)
    debug (SX_DEBUG_CMDS, "> SETTIME      SimSun 0x%02x, %02x:%02x:%02x\n", badr, buf[0], buf[1], buf[2]);
  else if ((badr & 0xf8) == SW_ID_BCASTHEAT)
    debug (SX_DEBUG_CMDS, "> SETTIME      HotBed 0x%02x, %02x:%02x:%02x\n", badr, buf[0], buf[1], buf[2]);
  else
    debug (SX_DEBUG_CMDS, "> SETTIME      Device 0x%02x, %02d:%02x:%02x\n", badr, buf[0], buf[1], buf[2]);
}

// sync RTC SunWeb devices at midnight
void sw_sync_clock (uint8_t badr)
{
  sw_txpack (badr, SW_CMD_SYNCRTC, 0, NULL);
  sw_txsend ();

  if ((badr & 0xf8) == SW_ID_BCASTLEDS)
    debug (SX_DEBUG_CMDS, "> SYNCRTC      SimSun 0x%02x\n", badr);
  else if ((badr & 0xf8) == SW_ID_BCASTHEAT)
    debug (SX_DEBUG_CMDS, "> SYNCRTC      HotBed 0x%02x\n", badr);
  else
    debug (SX_DEBUG_CMDS, "> SYNCRTC      Device 0x%02x\n", badr);
}
