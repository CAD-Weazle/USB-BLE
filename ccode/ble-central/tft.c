// file    : tft.c
// author  : rb
// purpose : ME813A-WH50C TFT module routines - FTDI FT813 EVE (Embedded Video Engine) - 
// date    : 190620
// last    : 191218
//

#include "includes.h"

//#define DUMP_DL_SIZE                 // dump percentage Display List used
//#define CALIBRATE_TOUCH              // run calibration 

#define TFT_TAGS_DEBUG                0    // dump touch position & tags

#define EVE_DEBUG_TRACKER             0    // EVE debug on/off
#define SWITCH_DELAY                  8    // panel switch/button delay in [s/10]

// TFT state
t_state tft_state;
t_state *ts = &tft_state;
uint8_t *ts_p = (uint8_t *)(&tft_state);

uint8_t blight = 1;

// set up TFT stuff
void init_tft (void)
{
  // get TFT state from EEPROM
  ee_tft_read ();

  // copy state from EEPROM to GroBox struct
  // - SimSun red/blu light mixture
  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    gb[i].ss_dred_flux = (ts->rlen[i] * ts->ilen[i]) / 100;  // total flux * %red
    gb[i].ss_rblu_flux =  ts->ilen[i] - gb[i].ss_dred_flux;  // total flux * %blu
  }
 
  // - HotBed temperature PID, fan en bottom LEDs
  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    gb[i].hb_tset = (220 + 5*((ts->pid_setpoint[i] - 8960)/2488)) * 10; // note: set HotBed PID [10*oC] / get PID [100*oC] !!1!1
    gb[i].hb_pid = ts->pid_active[i];
    gb[i].hb_led = ts->bottom_leds[i]; 
    gb[i].hb_fan = ts->fan_speed[i];
  }

  // overrule TFT state read from EEPROM
  ts->panel = TFT_MONITOR;  // start with Monitor

  // initialize TFT display panels
  init_mon_tft ();
  init_ss_tft  ();
  init_hb_tft  ();

  // calibrate touch screen
#ifdef CALIBRATE_TOUCH
  tft_touch_calibrate ();
#endif
}

// update TFT screens
void tft_update (void)
{
  uint8_t  tag;
  uint32_t tracker;

  // update watering program day 
  tft_next_day ();

  // update TFT display
  tft_display_screen ();

  // read touch tag 
  tag = (uint8_t)(eve_mem_rd32 (EVE_REG_CTOUCH_TAG) & 0x000000ffUL);

  // screen not touched, bail
  if (tag == 0x00)
    return;

  // screen is touched at undefined location, bail
  if (tag == 0xff)
    return;

  // get tracker state & extract tag number / track position
  tracker = eve_mem_rd32 (EVE_REG_TRACKER);

  uint16_t track_num = (uint16_t)(tracker & 0x0000ffff);
  uint16_t track_pos = (uint16_t)(tracker >> 16);           // range 0..65535

  debug (TFT_TAGS_DEBUG, "tag: %d tag_num: %d tag_pos: %d\n", tag, track_num, track_pos);
 
  // handle Monitor panel tags
  tft_mon_tags (tag, track_pos);

  // handle HotBed panel tags
  tft_hb_tags (tag, track_pos);

  // handle SimSun panel tags
  tft_ss_tags (tag, track_pos);

  // handle panel switch/EEPROM store tags
  tft_switch_store (tag);
}

// handle Monitor panel tags
void tft_mon_tags (uint8_t tag, uint16_t track_pos)
{
  static uint8_t dly = SWITCH_DELAY;

  if (dly--)
    return;

  switch (tag)
  {
    // pH sensor calibration
    case TAG_PH1:
      ph_cal_sensor (PH_SENSOR1);
      dly = SWITCH_DELAY;
      break;
  
    case TAG_PH2:
      ph_cal_sensor (PH_SENSOR2);
      dly = SWITCH_DELAY;
      break;

    case TAG_PH3:
      ph_cal_sensor (PH_SENSOR3);
      dly = SWITCH_DELAY;
      break;

    // start pH sensor calibration procedure
    case TAG_PHYES:
      ms->ph_cal_start = 1;
      dly = SWITCH_DELAY;
      break;

    // leave pH sensor calibration procedure    
    case TAG_PHNO:
      ms->ph_cal_busy = 0;
      dly = SWITCH_DELAY;
      break;

    // EC sensor calibration
    case TAG_EC1:
      ec_cal_sensor (EC_SENSOR1);
      dly = SWITCH_DELAY;
      break;

    case TAG_EC2:
      ec_cal_sensor (EC_SENSOR2);
      dly = SWITCH_DELAY;
      break;

    case TAG_EC3:
      ec_cal_sensor (EC_SENSOR3);
      dly = SWITCH_DELAY;
      break;

    // start pH sensor calibration procedure
    case TAG_ECYES:     
      ms->ec_cal_start = 1;
      dly = SWITCH_DELAY;
      break;

    // leave pH sensor calibration procedure
    case TAG_ECNO:
      ms->ec_cal_busy = 0;
      dly = SWITCH_DELAY;
      break;

    default:
      break;
  }
}

// handle SimSun panel tags
void tft_ss_tags  (uint8_t tag, uint16_t track_pos)
{
  // adjust to range to 0..100 [%] for LED bars
  uint16_t slider_lbar = track_pos / 655;

  // adjust to range 0..300 [PAR] for PAR bars
  uint16_t slider_ibar = (3*track_pos) / 655;

  // adjust to range 0..24*60*60 [s] for day time bar
  uint32_t slider_tbar = (10000*track_pos/7585); 

  switch (tag)
  {
    // LED RED/BLU mixture with increments of 5[%]
    case TAG_MIX1:
      ts->rlen[0] = slider_lbar - slider_lbar%5;
      ts->mix_new[0] = 1;
      break;

    case TAG_MIX2:
      ts->rlen[1] = slider_lbar - slider_lbar%5;
      ts->mix_new[1] = 1; 
      break;

    case TAG_MIX3:
      ts->rlen[2] = slider_lbar - slider_lbar%5;
      ts->mix_new[2] = 1;
      break;

    // PAR with increments of 10[PAR]
    case TAG_PAR1:
      ts->ilen[0] = slider_ibar - slider_ibar%10;
      ts->par_new[0] = 1;
      break;

    case TAG_PAR2:
      ts->ilen[1] = slider_ibar - slider_ibar%10;
      ts->par_new[1] = 1;
      break;

    case TAG_PAR3:
      ts->ilen[2] = slider_ibar - slider_ibar%10;
      ts->par_new[2] = 1;
      break;

    // day length slider
    case TAG_DAY:
      if (slider_tbar < ((ts->tlen[DAWN] + ts->tlen[DUSK])/2))
      {
        ts->tlen[DAWN] = slider_tbar - slider_tbar%900;  // dawn with increments of 15 [min]
        ts->dwn_new = 1;
      }
      else
      {
        ts->tlen[DUSK] = slider_tbar - slider_tbar%900;  // dusk with increments of 15 [min]
        ts->dsk_new = 1;
      }
      break;

    default:
      break;
  }
}

// handle HotBed panel tags
void tft_hb_tags (uint8_t tag, uint16_t track_pos)
{
  // select tag
  switch (tag)
  {
    // HotBed water time select buttons (note: selected day can not be > active days)
    case TAG_HBSEL1:
      ts->wtime_sel = 0;
      break;

    case TAG_HBSEL2:
      ts->wtime_sel = 1;
      break;

    case TAG_HBSEL3:      
      ts->wtime_sel = 2;
      break;

    // HotBed water program length buttons
    case TAG_HBACT1:
      ts->wtime_len = 0;
      break;

    case TAG_HBACT2:
      ts->wtime_len = 1;
      break;

    case TAG_HBACT3:
      ts->wtime_len = 2;
      break;

    // HotBed watering time sliders  (note: 65535 / 24*60*60 = 0.7585069 = 10000)/7585)
    case TAG_HBPOS1:
      if (ts->wtime_sel == 0)
      {
        ts->wtime_pos = (track_pos*10000)/7585;
        ts->wtime_pos = ts->wtime_pos - ts->wtime_pos%900;  // 15 minute increments
      }
      break;

    case TAG_HBPOS2:
      if (ts->wtime_sel == 1)
      {
        ts->wtime_pos = (track_pos*10000)/7585;
        ts->wtime_pos = ts->wtime_pos - ts->wtime_pos%900;  // 15 minute increments
      }
      break;

    case TAG_HBPOS3:
      if (ts->wtime_sel == 2)
      {
        ts->wtime_pos = (track_pos*10000)/7585;
        ts->wtime_pos = ts->wtime_pos - ts->wtime_pos%900;  // 15 minute increments
      }
      break;

    // HotBed PID control buttons
    case TAG_HBPID1_ACT:
      if (eve_pen_down ())
      {
        ts->pid_active[0] = (ts->pid_active[0] ? 0 : 1);
        ts->act_new[0] = 1;
      }
      break;

    case TAG_HBPID2_ACT:
      if (eve_pen_down ())
      {
        ts->pid_active[1] = (ts->pid_active[1] ? 0 : 1);
        ts->act_new[1] = 1;
      }
      break;

    case TAG_HBPID3_ACT:
      if (eve_pen_down ())
      {
        ts->pid_active[2] = (ts->pid_active[2] ? 0 : 1);
        ts->act_new[2] = 1;
      }
      break;

    // HotBed bottom LEDs control buttons
    case TAG_HBLED1:
      if (eve_pen_down ())
      {
        ts->bottom_leds[0] = (ts->bottom_leds[0] ? 0 : HB_LED_INTENSITY);
        ts->led_new[0] = 1;
      }
      break;

    case TAG_HBLED2:
      if (eve_pen_down ())
      {  
        ts->bottom_leds[1] = (ts->bottom_leds[1] ? 0 : HB_LED_INTENSITY);
        ts->led_new[1] = 1;
      }
      break;

    case TAG_HBLED3:
      if (eve_pen_down ())
      {
        ts->bottom_leds[2] = (ts->bottom_leds[2] ? 0 : HB_LED_INTENSITY);
        ts->led_new[2] = 1;
      }
      break;

    // HotBed fan speed control (note: setting already active speed == turn off fan)
    case TAG_HBFAN1_MIN:
      if (eve_pen_down ())
      {
        ts->fan_speed[0] = ((ts->fan_speed[0] == HB_FAN_MIN) ? HB_FAN_OFF : HB_FAN_MIN);
        ts->fan_new[0] = 1;
      }
      break;

    case TAG_HBFAN1_MID:
      if (eve_pen_down ())
      {
        ts->fan_speed[0] = ((ts->fan_speed[0] == HB_FAN_MID) ? HB_FAN_OFF : HB_FAN_MID);
        ts->fan_new[0] = 1;
      }
      break;

    case TAG_HBFAN1_MAX:
      if (eve_pen_down ())
      {
        ts->fan_speed[0] = ((ts->fan_speed[0] == HB_FAN_MAX) ? HB_FAN_OFF : HB_FAN_MAX);
        ts->fan_new[0] = 1;
      }
      break;

    case TAG_HBFAN2_MIN:
      if (eve_pen_down ())
      {
        ts->fan_speed[1] = ((ts->fan_speed[1] == HB_FAN_MIN) ? HB_FAN_OFF : HB_FAN_MIN);
        ts->fan_new[1] = 1;
      }
      break;

    case TAG_HBFAN2_MID:
      if (eve_pen_down ())
      {
        ts->fan_speed[1] = ((ts->fan_speed[1] == HB_FAN_MID) ? HB_FAN_OFF : HB_FAN_MID);
        ts->fan_new[1] = 1;
      }
      break;

    case TAG_HBFAN2_MAX:
      if (eve_pen_down ())
      {
        ts->fan_speed[1] = ((ts->fan_speed[1] == HB_FAN_MAX) ? HB_FAN_OFF : HB_FAN_MAX);
        ts->fan_new[1] = 1;
      }
      break;

    case TAG_HBFAN3_MIN:
      if (eve_pen_down ())
      {
        ts->fan_speed[2] = ((ts->fan_speed[2] == HB_FAN_MIN) ? HB_FAN_OFF : HB_FAN_MIN);
        ts->fan_new[2] = 1;
      }
      break;

    case TAG_HBFAN3_MID:
      if (eve_pen_down ())
      {
        ts->fan_speed[2] = ((ts->fan_speed[2] == HB_FAN_MID) ? HB_FAN_OFF : HB_FAN_MID);
        ts->fan_new[2] = 1;
      }
      break;

    case TAG_HBFAN3_MAX:
      if (eve_pen_down ())
      {
        ts->fan_speed[2] = ((ts->fan_speed[2] == HB_FAN_MAX) ? HB_FAN_OFF : HB_FAN_MAX);
        ts->fan_new[2] = 1;
      }
      break;

    // PID temperature setpoint rotary slider 
    case TAG_HBPID1_SET:
      ts->pid_setpoint[0] = track_pos;
      ts->pid_setpoint[0] = ts->pid_setpoint[0] - ts->pid_setpoint[0]%2488; // per 0.5 oC 'click'

      // set upper/lower bounds (22.0/32.0 oC)
      if (ts->pid_setpoint[0] < 8960)
        ts->pid_setpoint[0] = 8960;

      if (ts->pid_setpoint[0] > (65536 - (8960/2)))
        ts->pid_setpoint[0] = 65536 - (8960/2);

      ts->set_new[0] = 1;
      break;

    case TAG_HBPID2_SET:
      ts->pid_setpoint[1] = track_pos;
      ts->pid_setpoint[1] = ts->pid_setpoint[1] - ts->pid_setpoint[1]%2488; // per 0.5 oC 'click'

      // set upper/lower bounds     
      if (ts->pid_setpoint[1] < 8960)
        ts->pid_setpoint[1] = 8960;

      if (ts->pid_setpoint[1] > (65536 - (8960/2)))
        ts->pid_setpoint[1] = 65536 - (8960/2);

      ts->set_new[1] = 1;
      break;

    case TAG_HBPID3_SET:
      ts->pid_setpoint[2] = track_pos;
      ts->pid_setpoint[2] = ts->pid_setpoint[2] - ts->pid_setpoint[2]%2488; // per 0.5 oC 'click'

      // set upper/lower bounds     
      if (ts->pid_setpoint[2] < 8960)
        ts->pid_setpoint[2] = 8960;

      if (ts->pid_setpoint[2] > (65536 - (8960/2)))
        ts->pid_setpoint[2] = 65536 - (8960/2);

      ts->set_new[2] = 1;
      break;
  }

  // sanity check, selected day kan not be > active days
  if (ts->wtime_sel > ts->wtime_len)
    ts->wtime_sel = ts->wtime_len;

  // adjust current day if needed
  if (ts->wtime_day > ts->wtime_len)
    ts->wtime_day = ts->wtime_len;
}

// handle switch panel or store to EEPROM tags
void tft_switch_store (uint8_t tag)
{
  static uint8_t dly = SWITCH_DELAY;

  switch (tag)
  {
    // panel switching
    case TAG_SWL:
    {
      if (dly--)
        break;

      // switch to next panel
      if (ts->panel < 2)
        ts->panel++;
      else
        ts->panel = 0;

      // make sure EEPROM store flag is clear
      ts->store = 0; 

      dly = SWITCH_DELAY;
      break;
    }

    case TAG_SWR:
    {
      if (dly--)
        break;
   
      // switch to former panel
      if (ts->panel > 0)
        ts->panel--;
      else
        ts->panel = 2;

      // make sure EEPROM store flag is clear
      ts->store = 0; 

      dly = SWITCH_DELAY;
      break;
    }

    // display window 'store to EEPROM'
    case TAG_EESTORE:
      ts->store = 1; 
      break;

    // backlight on/off control
    case TAG_BLIGHT:
      if (blight)
      {
        eve_backlight (EVE_BLIGHT_OFF);
        blight = 0;
        msleep (500);
      }
      else
      {
        eve_backlight (EVE_BLIGHT_ON);
        blight = 1;
        msleep (500);
      }
      break;

    // store TFT state to EEPROM
    case TAG_EEWRITE:
      ee_tft_write ();
      
      ts->store = 0;  // clear flag, stop displaying window
      break;

    // cancel TFT state wtite to EEPROM
    case TAG_EECANCEL:
      ts->store = 0;  // clear flag, stop displaying window
      break;

    default:
      break;
  }
}

// display TFT screen
void tft_display_screen (void)
{
  // select panel to diplay
  switch (ts->panel)
  {
    case TFT_MONITOR:
    {
      tft_mon_panel ();
      break;
    }

    case TFT_SIMSUN:
    {
      tft_ss_panel ();
      break;
    }

    case TFT_HOTBED:
    {
      tft_hb_panel ();
      break;
    }

    default:
      break;
  }

  // dump size of Display List 
#ifdef DUMP_DL_SIZE
  uint32_t dl_ptr = eve_mem_rd32 (EVE_REG_CMD_DL);
  printf ("EVE_REG_CMD_DL   : 0x%03lx (DL memory %2.1f%% full)\n", dl_ptr, (100*(float)dl_ptr)/0x1fff);
#endif
}

// display wall clock
void tft_display_time (uint16_t x_pos, uint16_t y_pos)
{
  eve_cmd_write (COLOR_RGB | EVE_COL_WALLCLOCK);
  sprintf (eve_buf, "%02d:%02d:%02d", ms->hrs, ms->min, ms->sec);
  eve_cmd_text (x_pos, y_pos, 23, 0, eve_buf);
}

// displat watering is ongoing
void tft_display_watering (void)
{
  // bail whennog watering
  if (!ts->watering)
    return;

  eve_cmd_write (BEGIN | EVE_RECTS);  
  eve_cmd_write (COLOR_A | 0xff);
  eve_cmd_write (COLOR_RGB | EVE_COL_BLU);
  eve_cmd_write (LINE_WIDTH | 1*16); 

  // draw window outline           
  eve_cmd_write (VERTEX2F (100*16,  80*16));
  eve_cmd_write (VERTEX2F (700*16, 400*16));

  // draw text
  eve_cmd_write (COLOR_RGB | EVE_COL_OLD);
  sprintf (eve_buf, "%s", "- BUSY WATERING -");
  eve_cmd_text (210, 200, 31, 0, eve_buf);
}

// display invisible panel switch & store/retrieve buttons
void tft_draw_panelswitch (void)
{
  eve_cmd_write (BEGIN | EVE_RECTS);  
  eve_cmd_write (LINE_WIDTH | 1*16);        // reset line width just to be sure
  eve_cmd_write (COLOR_RGB | EVE_COL_WHT);
  eve_cmd_write (COLOR_A | 0x00); 

  // switch to left
  eve_cmd_write (TAG | TAG_SWL);
  eve_cmd_write (VERTEX2F ( 0*16, 150*16));
  eve_cmd_write (VERTEX2F (50*16, 399*16));

  // switch to right
  eve_cmd_write (TAG | TAG_SWR);
  eve_cmd_write (VERTEX2F (750*16, 150*16));  
  eve_cmd_write (VERTEX2F (800*16, 399*16));

  // EEPROM store button
  eve_cmd_write (TAG | TAG_BLIGHT);
  eve_cmd_write (VERTEX2F (  0*16, 400*16));  
  eve_cmd_write (VERTEX2F ( 50*16, 480*16));

  // TFT backlight on/off control button
  eve_cmd_write (TAG | TAG_EESTORE);
  eve_cmd_write (VERTEX2F (750*16, 400*16));  
  eve_cmd_write (VERTEX2F (800*16, 480*16));

  // end tagging & end drawing graphics primitive
  eve_cmd_write (TAG | TAG0);
  eve_cmd_write (END);  
}

// draw EEPROM storage dialog window
void tft_eestore (void)
{
  const uint16_t XP =  90;        // x-origin
  const uint16_t YP = 130;        // y-origin  
  const uint16_t XW = 610;        // x-width
  const uint16_t XM = (XP+XW/2);  // x middle
  const uint16_t YH = 240;        // y-heigth
  const uint16_t XO = 20;         // text x-offset 
  const uint16_t YO = 30;         // text y-offset 
  const uint16_t BW = 190;        // button width
  const uint16_t BH = 90;         // button height
  const uint16_t BS = 20;         // button spacing

  // begin drawing primitives
  eve_cmd_write (BEGIN | EVE_RECTS);  
  eve_cmd_write (COLOR_A | 0xff);
  eve_cmd_write (COLOR_RGB | EVE_COL_WINDOW);
  eve_cmd_write (LINE_WIDTH | 10*16); 

  // draw window outline           
  eve_cmd_write (VERTEX2F ( XP*16,      YP*16));
  eve_cmd_write (VERTEX2F ((XP+XW)*16, (YP+YH)*16));

  // draw text
  eve_cmd_write (COLOR_RGB | EVE_COL_OLD);
  sprintf (eve_buf, "%s", "write to non-volatile storage?");
  eve_cmd_text ((XP+XO), (YP+YO), 31, 0, eve_buf);
      
  // draw 'WRITE' button
  eve_cmd_fgcolor (EVE_COL_BUTTONYES);
  eve_cmd_bgcolor (EVE_COL_BLK);
  eve_cmd_write   (TAG | TAG_EEWRITE);
  eve_cmd_button  (XM - (BW + BS), YP + YH/2, BW, BH, 31, EVE_OPT_FLAT, "YES");
  eve_cmd_write   (TAG | TAG0);

  // draw 'CANCEL' button
  eve_cmd_fgcolor (EVE_COL_BUTTONNO);
  eve_cmd_bgcolor (EVE_COL_BLK);
  eve_cmd_write   (TAG | TAG_EECANCEL);
  eve_cmd_button  (XM + BS, YP + YH/2, BW, BH, 31, EVE_OPT_FLAT, "NO");
  eve_cmd_write   (TAG | TAG0);

  // clear tag & reset line width
  eve_cmd_write (LINE_WIDTH | 1*16); 

  // end drawing primitives
  eve_cmd_write   (END);
}

// calibrate touch screen
void tft_touch_calibrate (void)
{
  // start writing to command FIFO
  eve_cmd_start (EVE_COL_GRY);      

  // set background & foreground color
  eve_cmd_bgcolor (EVE_COL_GRY);
  eve_cmd_fgcolor (EVE_COL_RED);

  // display message
  eve_cmd_write (COLOR_RGB | EVE_COL_RED);
  eve_cmd_text ((EVE_HSIZE/2), (EVE_VSIZE/2), 30, EVE_OPT_CENTERX, "- please tap the dots -");

  // send calibrate command
  eve_cmd_write (CMD_CALIBRATE);
  eve_cmd_write (END);

  // mark end Display List & swap display buffers
  eve_cmd_write (DISPLAY);
  eve_cmd_write (CMD_SWAP);

  // start execution: update command FIFO write pointer
  eve_cmd_exec ();

  // wait for command completion, get offset first empty location command FIFO
  eve_cmd_wait ();

  // enter extended touch mode
  eve_mem_wr32 (EVE_REG_CTOUCH_EXTENDED, 0x00);

	uint32_t touch[6];

	touch[0] = eve_mem_rd32 (EVE_REG_TOUCH_TRANSFORM_A);
	touch[1] = eve_mem_rd32 (EVE_REG_TOUCH_TRANSFORM_B);
	touch[2] = eve_mem_rd32 (EVE_REG_TOUCH_TRANSFORM_C);
	touch[3] = eve_mem_rd32 (EVE_REG_TOUCH_TRANSFORM_D);
	touch[4] = eve_mem_rd32 (EVE_REG_TOUCH_TRANSFORM_E);
	touch[5] = eve_mem_rd32 (EVE_REG_TOUCH_TRANSFORM_F);

  // start writing to command FIFO
  eve_cmd_start (EVE_COL_BLK);      

  // set background & foreground color
  eve_cmd_bgcolor (EVE_COL_GRN);
  eve_cmd_fgcolor (EVE_COL_WHT);

  // display touch screen transform parameters
  eve_cmd_write (COLOR_RGB | EVE_COL_YEL);
  char bufje[64];

  for (int i = 0; i < 6; i++)
  {
    printf ("  eve_mem_wr32 (EVE_REG_TOUCH_TRANSFORM_%c, 0x%08lx\n);", 'A'+i, touch[i]);

    sprintf (bufje, "touch transform %c: 0x%08lx", 'A'+i, touch[i]);
  	eve_cmd_text (5, 20*i, 18, 0, bufje);
  }

  // mark end Display List & swap display buffers
	eve_cmd_write (DISPLAY);
	eve_cmd_write (CMD_SWAP); 

  // start execution: update command FIFO write pointer
  eve_cmd_exec ();
}

// dump snapshot to RAM_GP
void tft_get_snapshot (uint32_t adr)
{
  // turn PCLK off
  eve_mem_wr8  (EVE_REG_PCLK, 0);         

  // grab screen
  eve_cmd_start (EVE_COL_RED);
  eve_cmd_snapshot (adr);
  eve_cmd_exec ();

  // & back on again
  eve_mem_wr8  (EVE_REG_PCLK, EVE_PCLK);  
}

// dump snapshot to serial port in PPM format
void tft_ppm_snapshot (uint32_t adr, uint16_t width, uint16_t height)
{
  uint32_t i, dat;

  // generate ASCII PPM image 
  printf ("P3 \n");                    // magic 8-bit RGB ASCII image
  printf ("%d %d \n", width, height);  // image size
  printf ("255 \n");                   // max color value

  for (i = 0; i < width*height*2; i += 2)
  {
    dat = eve_mem_rd16 (adr + i);
 
    // ditch alfa channel
    uint8_t r = (uint8_t)((dat & 0x0f00) >> 8) << 4;
    uint8_t g = (uint8_t)((dat & 0x00f0) >> 4) << 4;
    uint8_t b = (uint8_t)((dat & 0x000f) >> 0) << 4;

    printf ("%d %d %d ", r, g, b);
  }
}

// day counter for watering system
#define NDAY_START      0
#define NDAY_MIDNIGHT   1
#define NDAY_IDLE       2

void tft_next_day (void)
{
  static uint8_t state = NDAY_START;
  static uint8_t cnt = 0;

  switch (state)
  {
    // test midnight with margin for clock jitter
    case NDAY_START:
    {
      if (ms->tsecs >= DAY_SECS-1)  
      {
        state = NDAY_MIDNIGHT;
        cnt = 0;
      }
      break;
    }

    // increment day counter & wrap
    case NDAY_MIDNIGHT:
    {
      ts->wtime_day++;
 
      if (ts->wtime_day > ts->wtime_len)
        ts->wtime_day = 0;

     state = NDAY_IDLE;
     break;
    }

    // idle some time (avoid races)
    case NDAY_IDLE:
    {
      if (cnt++ > 100)
        state = NDAY_START;
      break;
    }

    default:
      break;
  }
}

// dump 't_state' struct
void tft_tstate_dump (void)
{
  // global
  printf ("panel    : %d\n", ts->panel);                                    // current TFT panel being diplayed 
  
  // SimSun Display Panel
  printf ("rlen     : %d %d %d\n", ts->rlen[0], ts->rlen[1], ts->rlen[2]);  // LED light red/blu mixture [0..100%]
  printf ("ilen     : %d %d %d\n", ts->ilen[0], ts->ilen[1], ts->ilen[2]);  // lenght of intensity bar [0..300PAR]
  printf ("tlen     : %ld %ld\n",  ts->tlen[0], ts->tlen[1]);               // dawn & dusk in [s] (0..24*60*60)
  printf ("store    : %d\n"      , ts->store);                              // store/retreive button pressed

  // HotBed Display Panel 
  printf ("wtime_sel: %d\n",  ts->wtime_sel);                               // water timer select day to water
  printf ("wtime_len: %d\n",  ts->wtime_len);                               // water timer program lenght in [days]
  printf ("wtime_day: %d\n",  ts->wtime_day);                               // water timer current day
  printf ("wtime_pos: %ld\n", ts->wtime_pos);                               // water timer slider
  printf ("pid_setpoint: %d %d %d\n",  ts->pid_setpoint[0], ts->pid_setpoint[1], ts->pid_setpoint[2]); // PID heater setpoint rotary slider (0..65535)
  printf ("pid_active  : %d %d %d\n",  ts->pid_active[0], ts->pid_active[1], ts->pid_active[2]);       // PID heater control on/off         (note: Master state only, not SunWeb state)
  printf ("bottom_leds : %d %d %d\n",  ts->bottom_leds[0], ts->bottom_leds[1], ts->bottom_leds[2]);    // bottom LEDs control on/off        (note: Master state only, not SunWeb state)
  printf ("fan_speed   : %d %d %d\n",  ts->fan_speed[0], ts->fan_speed[1], ts->fan_speed[2]);          // fan speed control off/min/mid/max {note: Master state only, not SunWeb state)

  printf ("led_new     : %d %d %d\n",  ts->led_new[0], ts->led_new[1], ts->led_new[2]);                // HotBed bottom LED dirty bit
  printf ("fan_new     : %d %d %d\n",  ts->fan_new[0], ts->fan_new[1], ts->fan_new[2]);                // HotBed bottom LED dirty bit
  printf ("set_new     : %d %d %d\n",  ts->set_new[0], ts->set_new[1], ts->set_new[2]);                // HotBed bottom LED dirty bit
  printf ("\n");
}
