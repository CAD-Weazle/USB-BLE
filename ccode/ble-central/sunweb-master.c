// file    : sunweb-master.c
// author  : rb
// purpose : 
// date    : 190116
// last    : 191220
//

#include "includes.h"

#define SW_DEBUG_CMDS                  0   // debug command processing
#define SW_RX_ECHO                     0   // dump Rx SunWeb packets

// some flags
uint8_t do_probe = 0;                  // start probing SunWeb for Devices, invoked via command line only
uint8_t bus_probed = 0;                // SunWeb has been probed, start normal operation

// GroBox state
gbox grobox[SW_NUM_GBOXES];
gbox *gb = (gbox *)&grobox;

// init SunWeb state
void init_sunweb (void)
{
  // retrieve busmap from EEPROM
  ee_read_busmap ();
  
  // init SunWeb Devices
  sw_init_gboxes ();
}

// SunWeb statemachine, handle probe, program & logging SunWeb Devices
void sw_update (void)
{
  static uint8_t state = SX_DO_RELEASE;

  switch (state)
  {
    // release all SunWeb Devices
    case SX_DO_RELEASE:
    {
      sw_release_all ();

      // reset flag, after being set by command line (only)
      do_probe = 0;

      state = SX_DO_PROBE;
      break;
    }

    // probe Devices found in busmap
    case SX_DO_PROBE:
    {
      if (sw_probe_all ())
        state = SX_DO_PROGRAM;

      break;
    }

    // program SunWeb Devices
    case SX_DO_PROGRAM:
    {
      sw_program_all ();

      state = SX_DO_HANDLE;
      break;
    }

    // normal SunWeb bus operation, log Devices
    case SX_DO_HANDLE:
    {
      // parse received packets
      if (sw_rxnew ())
      {
        // check if Rx packet correct & then parse receive packet
        if (sw_rxcheck ())
          sw_parse_cmd ();
  
        // reset Rx state
        sw_rxclear ();
      }

      // handle TFT commands

      // check for new SimSun dawn setting
      if (ts->dwn_new)
      {
        // send over SunWeb
        ss_set_dawn (SW_ID_BCASTLEDS, ts->tlen[DAWN]);

        // reset flag
        ts->dwn_new = 0;
      }

      // check for new SimSun dusk setting
      if (ts->dsk_new)
      {
        // send over SunWeb
        ss_set_dusk (SW_ID_BCASTLEDS, ts->tlen[DUSK]);

        // reset flag
        ts->dsk_new = 0;
      }

      // check for new SimSun light mixture setting
      for (int i = 0; i < SW_NUM_GBOXES; i++)
      {
        if (ts->mix_new[i])
        {
          // copy TFT state tp GroBox struct
          gb[i].ss_dred_flux = (ts->rlen[i] * ts->ilen[i]) / 100;  // total flux * %red
          gb[i].ss_rblu_flux =  ts->ilen[i] - gb[i].ss_dred_flux;  // total flux * %blu

          // send new red/blu LED intensity over SunWeb, active next day cycle
          ss_max_dred (gb[i].ss_id, gb[i].ss_dred_flux);
          ss_max_rblu (gb[i].ss_id, gb[i].ss_rblu_flux);
 
          // reset flag
          ts->mix_new[i] = 0;  
        }
      }

      // check for new SimSun light intensity setting
      for (int i = 0; i < SW_NUM_GBOXES; i++)
      {
        if (ts->par_new[i])
        {
          // copy TFT state tp GroBox struct
          gb[i].ss_dred_flux = (ts->rlen[i] * ts->ilen[i]) / 100;  // total flux * %red
          gb[i].ss_rblu_flux =  ts->ilen[i] - gb[i].ss_dred_flux;  // total flux * %blu

          // send new red/blu LED intensity over SunWeb, active next day cycle
          ss_max_dred (gb[i].ss_id, gb[i].ss_dred_flux);
          ss_max_rblu (gb[i].ss_id, gb[i].ss_rblu_flux);
 
          // reset flag
          ts->par_new[i] = 0;
        }
      }

      // check for new HotBed bottom LED setting
      for (int i = 0; i < SW_NUM_GBOXES; i++)
      {
        if (ts->led_new[i])
        {
          // copy TFT state to GroBox struct & send over SunWeb
          gb[i].hb_led = ts->bottom_leds[i];
          hb_set_led (gb[i].hb_id, gb[i].hb_led);

          // reset flag
          ts->led_new[i] = 0;
        }
      }

      // check for new HotBed fan speed setting
      for (int i = 0; i < SW_NUM_GBOXES; i++)
      {
        if (ts->fan_new[i])
        {
          gb[i].hb_fan = ts->fan_speed[i];
          hb_set_fan (gb[i].hb_id, gb[i].hb_fan);

          // reset flag
          ts->fan_new[i] = 0;
        }
      }

      // check for new HotBed PID active/inactive setting
      for (int i = 0; i < SW_NUM_GBOXES; i++)
      {
        if (ts->act_new[i])
        {
          // copy TFT state to GroBox struct
          gb[i].hb_pid = ts->pid_active[i];

          // send PID start/stop over SunWeb
          if (gb[i].hb_pid)
            hb_start_pid (gb[i].hb_id);
          else            
            hb_stop_pid (gb[i].hb_id);

          // reset flag
          ts->act_new[i] = 0;
        }
      }

      // check for new HotBed PID temperature setpoint
      for (int i = 0; i < SW_NUM_GBOXES; i++)
      {
        if (ts->set_new[i])
        {
          // calculate temperature in GroBox format, set range to (22.0 .. 32.0oC) with 0.5oC steps
          gb[i].hb_tset = (220 + 5*((ts->pid_setpoint[i] - 8960)/2488)) * 10;  

          // send new setpoint over SunWeb
          hb_set_temp (gb[i].hb_id, gb[i].hb_tset/10);  // set PID target [10*oC]           

          // reset flag
          ts->set_new[i] = 0;
        }
      }

      // restart at PROBE stage (invoked by command line only)
      if (do_probe)
        state = SX_DO_RELEASE;

      break;
    }

    default:
      break;
  }
}

// release all SunWeb Devices
void sw_release_all (void)
{
  // release all possible SimSun Devices
  for (int i = 0; i < SW_NUM_DEVICES/2; i++)
  {
    sw_release (SW_ID_BCASTLEDS + i + 1);
    msleep (50);  // relax a bit
  }

  // release all possible HotBox Devices
  for (int i = 0; i < SW_NUM_DEVICES/2; i++)
  {
    sw_release (SW_ID_BCASTHEAT + i + 1);
    msleep (50);  // relax a bit
  }

  //relax a bit
  msleep (500);
}

// probe SunWeb for all Devices
uint8_t sw_probe_all (void)
{
         uint8_t id;
  static uint8_t cnt = 0;
  static uint8_t delay = 0;
  static uint8_t state = PROBE_SS;

  switch (state)
  {
    case PROBE_SS:
    {
      // slow down a bit
      if (delay++ < 5)
        break;
      else
        delay = 0;

      // read SimSun Device ID
      id = gb[cnt].ss_id;

      // send probe when valid ID present
      if (id != 0xff)
      {
        sw_probe (id);
        msleep (200);          // relax a bit for response from Device 

        state = PROBE_HANDLE_SS;
      }
      else
        state = PROBE_HB;

      break;
    }

    case PROBE_HANDLE_SS:
    {
      // parse received packets
      if (sw_rxnew ())
      {
        // check if correct & then parse receive packet
        if (sw_rxcheck ())
          sw_parse_cmd ();
  
        sw_rxclear ();      
      }

      state = PROBE_HB;
      break;
    }

    case PROBE_HB:
    {
      // slow down a bit
      if (delay++ < 5)
        break;
      else
        delay = 0;

      // read HotBed Device ID
      id = gb[cnt].hb_id;

      // send probe when valid ID present
      if (id != 0xff)
      {
        sw_probe (id);
        msleep (200);          // relax a bit for response from Device 

        state = PROBE_HANDLE_HB;
      }
      else
        state = PROBE_NEXT;

      break;
    }

    case PROBE_HANDLE_HB:
    {
      // parse received packets
      if (sw_rxnew ())
      {
        // check if correct & then parse receive packet
        if (sw_rxcheck ())
          sw_parse_cmd ();
  
        sw_rxclear ();      
      }

      state = PROBE_NEXT;
      break;
    }

    case PROBE_NEXT:
    {
      // adjust counter & test
      cnt++;

      if (cnt >= SW_NUM_GBOXES)
        state = PROBE_READY;
      else        
        state = PROBE_SS;

      break;
    }

    case PROBE_READY:
    {
      bus_probed = 1;

      printf ("#bus probe ready\n");  
    
      return 1;
    }

    default:
      break;
  }

  return 0;
}

// handle probed Device ID
void sw_probe_handle (uint8_t id)
{
  // search for ID in GroBox array
  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    if (gb[i].ss_id == id)
    {
      gb[i].ss_attc = 1;
      return;
    }
  
    if (gb[i].hb_id == id)
    {
      gb[i].hb_attc = 1;
      return;
    }
  }
}

// init SunWeb Devices <> get settings from EEPROM
void sw_program_all (void)
{
  // set local RTC SunWeb Devices to current time from DS3234
  sw_set_clock (SW_ID_BCASTLEDS);            
  sw_set_clock (SW_ID_BCASTHEAT);            

  // set up SimSun Devices
  ss_simsun_stop (SW_ID_BCASTLEDS);              // stop SimSun on all LED Devices

  // set dawn & dusk for all Devices
  ss_set_dawn (SW_ID_BCASTLEDS, ts->tlen[DAWN]);
  ss_set_dusk (SW_ID_BCASTLEDS, ts->tlen[DUSK]);

  // set up red/blu LED light mixture
  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    // only send to attached Devices
    if (gb[i].ss_attc)
    {
      ss_max_dred (gb[i].ss_id, gb[i].ss_dred_flux);
      ss_max_rblu (gb[i].ss_id, gb[i].ss_rblu_flux);
    }
  }

  // start SimSun program
  ss_simsun_start (SW_ID_BCASTLEDS);

  // setup HotBed Devices
  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    // only send to attached Devices
    if (gb[i].hb_attc)
    {
      hb_set_temp (gb[i].hb_id, gb[i].hb_tset/10);  // set PID target [10*oC]
   
      if (gb[i].hb_pid)                          // PID active/inactive
        hb_start_pid (gb[i].hb_id);
      else        
        hb_stop_pid (gb[i].hb_id);

      hb_set_led (gb[i].hb_id, gb[i].hb_led);    // bottom LEDs intensity
      hb_set_fan (gb[i].hb_id, gb[i].hb_fan);    // fan speed
    }
  }
}

// SunWeb Command parser - to Master/from Device
void sw_parse_cmd (void)
{
  // dump Rx packet
  if (SW_RX_ECHO)
    sw_rxdump ();

  // parse SunWeb command part
  switch (SW_CMD_BYTE)
  {
    // ----- probe response from SimSun/HotBed Device -----
    case SW_CMD_PROBE:
    {
      // ignore broadcast addresses sent back (patch for some error) & bail <> still needed?
      if ((SW_ID_BYTE == SW_ID_BCASTLEDS) || (SW_ID_BYTE == SW_ID_BCASTHEAT))
        break;

      // handle new Bus ID
      sw_probe_handle (SW_ID_BYTE);

      // & send Bus Attach command
      sw_txpack (SW_ID_BYTE, SW_CMD_ATTACH, 0, NULL);
      sw_txsend ();

      if ((SW_ID_BYTE & 0xf8) == SW_ID_BCASTLEDS)
        debug (SW_DEBUG_CMDS, "> ATTACH       SimSun 0x%02x\n", SW_ID_BYTE);
      else if ((SW_ID_BYTE & 0xf8) == SW_ID_BCASTHEAT)
        debug (SW_DEBUG_CMDS, "> ATTACH       HotBed 0x%02x\n", SW_ID_BYTE);
      else
        debug (SW_DEBUG_CMDS, "> ATTACH       Device 0x%02x\n", SW_ID_BYTE);

      break;
    }

    // ----- SimSun Device responses -----
    // SimSun temperature & LED intensity
    case SS_CMD_GETSTATE:
    {
      for (int i = 0; i < SW_NUM_GBOXES; i++)
      {
        if (gb[i].ss_id == SW_ID_BYTE)
        {
          gb[i].ss_tact = ((uint16_t)rx_buf[4] << 8) + rx_buf[5];

          gb[i].ss_dred = rx_buf[6];
          gb[i].ss_rblu = rx_buf[7];
          gb[i].ss_fred = rx_buf[8];

          debug (SW_DEBUG_CMDS, "< GETSTATE     SimSun 0x%02x, %d, %d, %d\n", SW_ID_BYTE, gb[i].ss_tact, gb[i].ss_dred, gb[i].ss_rblu, gb[i].ss_fred);
        }
      }

      break;
    }

    // ----- HotBed Device responses -----
    // HotBed setpoint & actual temperatures
    case HB_CMD_GETTEMP:
    {
      for (int i = 0; i < SW_NUM_GBOXES; i++)
      {
        if (gb[i].hb_id == SW_ID_BYTE)
        {
          gb[i].hb_tset = (uint16_t)(rx_buf[4] << 8) + rx_buf[5]; // note: set PID [10*oC] / get PID [100*oC] !!1!1
          gb[i].hb_tact = (uint16_t)(rx_buf[6] << 8) + rx_buf[7];
          gb[i].hb_tamb = (uint16_t)(rx_buf[8] << 8) + rx_buf[9];

          debug (SW_DEBUG_CMDS, "< GETTEMP      HotBed 0x%02x, %d, %d, %d,\n", SW_ID_BYTE, gb[i].hb_tset, gb[i].hb_tact, gb[i].hb_tamb);
        }
      }

      break;
    }

    // HotBed LED, fan & PID state
    case HB_CMD_GETSTATE:
    {
      for (int i = 0; i < SW_NUM_GBOXES; i++)
      {
        if (gb[i].hb_id == SW_ID_BYTE)
        {
          gb[i].hb_led = rx_buf[4];     // LED intensity
          gb[i].hb_fan = rx_buf[5];     // fan speed
          gb[i].hb_pid = rx_buf[6];     // PID active

          debug (SW_DEBUG_CMDS, "< GETTEMP      HotBed 0x%02x, %d, %d, %d,\n", SW_ID_BYTE, gb[i].hb_led, gb[i].hb_fan, gb[i].hb_pid);
        }
      }

      break;
    }

    // HotBed indiviual Pt100 temperatures, for offset error calibration only
    case HB_CMD_GETPT100:
    {
      for (int i = 0; i < SW_NUM_GBOXES; i++)
      {
        if (gb[i].hb_id == SW_ID_BYTE)
        {
          // get actual state HotBed
          uint16_t t_pt1 = (uint16_t)(rx_buf[4] << 8) + rx_buf[5];  // HotBed temperature sensor 1
          uint16_t t_pt2 = (uint16_t)(rx_buf[6] << 8) + rx_buf[7];  // HotBed temperature sensor 2
          uint16_t t_amb = (uint16_t)(rx_buf[8] << 8) + rx_buf[9];  // HotBed ambient temperature

          debug (SW_DEBUG_CMDS, "< GETPT100   HotBed 0x%02x, %d, %d, %d\n", SW_ID_BYTE, t_pt1, t_pt2, t_amb);

          printf ("T1: %02d.%02d, T2: %02d.%02d, Tamb: %02d.%02d\n", \
                              t_pt1/100, t_pt1%100,                  \
                              t_pt2/100, t_pt2%100,                  \
                              t_amb/100, t_amb%100);
        }
      }

      break;
    }
  }
}

// update state attached SunWeb Devices
void sw_log_all (void)
{
  static uint8_t state = SW_LOG_HBTEMP;
  static uint8_t num = 0;

  if (!bus_probed)
    return;

  // poll Devices round robin
  switch (state)
  {
    // get HotBed setpoint & actual temperature 
    case SW_LOG_HBTEMP:
    {
      // get temperature HotBed Device
      if (gb[num].hb_attc)
        hb_get_temp (gb[num].hb_id);

      if (++num >= SW_NUM_GBOXES)
      {
        num   = 0;
        state = SW_LOG_HBSTATE;
      }

      break;
    }
    
    // get HotBed state 
    case SW_LOG_HBSTATE:
    {
      // get state HotBed Device
      if (gb[num].hb_attc)
        hb_get_state (gb[num].hb_id);

      if (++num >= SW_NUM_GBOXES)
      {
        num   = 0;
        state = SW_LOG_SSSTATE;
      }

      break;
    }
    
    // get SimSun state
    case SW_LOG_SSSTATE:
    {
      // get state SimSun Device
      if (gb[num].ss_attc)
        ss_get_state (gb[num].ss_id);

      if (++num >= SW_NUM_GBOXES)
      {
        num   = 0;
        state = SW_LOG_HBTEMP;
      }

      break;
    }  

    default:
      break;
  }
}

// release all Devices & restart network
void sw_start_probe (void)
{
  do_probe = 1;
}

// init GroBoxes
void sw_init_gboxes (void)
{
  // init GroBoxes
  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    // get SimSun Device ID from busmap from EEPROM & set default values
    gb[i].ss_id   = ee_get_busmap (i, SW_ID_BCASTLEDS);
    gb[i].ss_attc = 0;
    gb[i].ss_dred = 0;
    gb[i].ss_fred = 0;
    gb[i].ss_rblu = 0;
    gb[i].ss_tact = 999;

    // get HotBed Device ID from busmap from EEPROM & set default values
    gb[i].hb_id   = ee_get_busmap (i, SW_ID_BCASTHEAT);
    gb[i].hb_attc = 0;
//  gb[i].hb_tset = 9990;
    gb[i].hb_tact = 9990;
    gb[i].hb_tamb = 9990;
//  gb[i].hb_pid  = 0;
//  gb[i].hb_fan  = 0;
  }
}

// dump GroBoxes
void sw_dump_gboxes (void)
{
  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    printf ("-- GroBox %d --\n", i+1);
    printf ("SimSun ID           : 0x%02x\n", gb[i].ss_id);
    printf (" SimSun attached    : %d\n",     gb[i].ss_attc);
    printf (" DRed LED intensity : %d\n",     gb[i].ss_dred);
    printf (" FRed LED intensity : %d\n",     gb[i].ss_fred);
    printf (" RBlu LED intensity : %d\n",     gb[i].ss_rblu);
    printf (" DRed LED flux      : %d\n",     gb[i].ss_dred_flux);
    printf (" RBlu LED flux      : %d\n",     gb[i].ss_rblu_flux);
    printf (" temperature        : %d\n",     gb[i].ss_tact);

    printf ("HotBed ID           : 0x%02x\n", gb[i].hb_id);
    printf (" HotBed attached    : %d\n",     gb[i].hb_attc);
    printf (" Tsetpoint          : %ld\n",    gb[i].hb_tset);  // [10*oC]  - beware!
    printf (" Tactual            : %ld\n",    gb[i].hb_tact);  // [100*oC] - beware!
    printf (" Tambient           : %ld\n",    gb[i].hb_tamb);  // [108*oC] - beware!
    printf (" LED intensity      : %d\n",     gb[i].hb_led);
    printf (" fan speed          : %d\n",     gb[i].hb_fan);
    printf (" PID active         : %d\n",     gb[i].hb_pid);
  }
}

// log GroBoxes
void sunweb_dump (void)
{
  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    // SimSun state
    printf ("%d "     , gb[i].ss_dred);
    printf ("%d "     , gb[i].ss_rblu);
    printf ("%d "     , gb[i].ss_fred);
    printf ("%d.%d "  , gb[i].ss_tact/10, gb[i].ss_tact%10);

    // HotBed state
    // nb: convert temperature to single decimal precision (xx.xx -> xx.x)
    printf ("%ld.%ld ", (gb[i].hb_tset+5)/100, (gb[i].hb_tset+5)/10%10);
    printf ("%ld.%ld ", (gb[i].hb_tact+5)/100, (gb[i].hb_tact+5)/10%10);
    printf ("%ld.%ld ", (gb[i].hb_tamb+5)/100, (gb[i].hb_tamb+5)/10%10);

    printf ("%d "     , gb[i].hb_led);
    printf ("%d "     , gb[i].hb_fan);
    printf ("%d  "    , gb[i].hb_pid);
  }

  printf ("\n");
}
