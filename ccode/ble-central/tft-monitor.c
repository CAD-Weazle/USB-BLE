// file    : tft-monitor.c
// author  : rb
// purpose : Monitor TFT interface
// date    : 191121
// last    : 191218
//

#include "includes.h"

// touch tags
uint16_t phcal_tag[SW_NUM_GBOXES] = {TAG_PH1, TAG_PH2, TAG_PH3};  // touch tags for pH calibration 'buttons'
uint16_t eccal_tag[SW_NUM_GBOXES] = {TAG_EC1, TAG_EC2, TAG_EC3};  // touch tags for EC calibration 'buttons'

// initialize Monitor TFT panel
void init_mon_tft (void)
{
}

// display Monitor control panel
void tft_mon_panel (void)
{
  // start writing to command FIFO
  eve_cmd_start (EVE_COL_GRY);      

  // display wall clock
  tft_display_time (TIME_XN, TIME_YN);

  // display ambient condition (temp/humidity/pressure/CO2 level)
  tft_mon_ambient ();

  // display GroBox conditions
  tft_mon_grobox ();

  // display feed water conditions
  tft_mon_water (); 

  // display panel switch buttons or pH/EC calibration menu <> todo: EC
  tft_mon_hmi ();  

  // draw watering busy state 
  tft_display_watering ();

  // mark end of Display List & swap display buffers
  eve_cmd_stop ();
}

// display ambient condition
void tft_mon_ambient (void)
{
  eve_cmd_write (COLOR_RGB | EVE_COL_GLD);
  sprintf (eve_buf, "%2.1f C  %2.1f%%RH  %dmbar  %3.0fppm", ms->temp, ms->humi, ms->prs, ms->co2); 
  eve_cmd_text  (MON_AMBIENT_XN, MON_AMBIENT_YN, 31, 0, eve_buf);
  sprintf (eve_buf, "o"); 
  eve_cmd_text  (MON_AMBIENT_XN+83, MON_AMBIENT_YN-5, 29, 0, eve_buf); // <> nasty hack, look for new font

}

// display GroBox conditions
void tft_mon_grobox (void)
{
  // bar over HotBed data
  eve_cmd_write (COLOR_RGB | EVE_COL_YEL);
  eve_cmd_write (COLOR_A | 0x10);
  eve_cmd_write (LINE_WIDTH | 20*16);
  eve_cmd_write (BEGIN | EVE_RECTS);
  eve_cmd_write (VERTEX2F ( 50*16,  50*16));
  eve_cmd_write (VERTEX2F (750*16, 180*16));

  // set text color
  eve_cmd_write (COLOR_A | 0xff);
  eve_cmd_write (COLOR_RGB | EVE_COL_OLD);

  // label setpoint temperature
  sprintf (eve_buf, "%s", "SET"); 
  eve_cmd_text (50,   50, 30, 0, eve_buf);
  sprintf (eve_buf, "%s", " C"); 
  eve_cmd_text (690,  50, 30, 0, eve_buf);
  sprintf (eve_buf, "%s", "o"); 
  eve_cmd_text (685,  45, 29, 0, eve_buf);  // <> nasty hack, look for new font


  // label actual temperature
  sprintf (eve_buf, "%s", "ACT"); 
  eve_cmd_text (50,  100, 30, 0, eve_buf);
  sprintf (eve_buf, "%s", " C"); 
  eve_cmd_text (690, 100, 30, 0, eve_buf);
  sprintf (eve_buf, "%s", "o"); 
  eve_cmd_text (685,  95, 29, 0, eve_buf);  // <> nasty hack, look for new font

  // label ambient temperature
  sprintf (eve_buf, "%s", "AIR"); 
  eve_cmd_text (50,  150, 30, 0, eve_buf);
  sprintf (eve_buf, "%s", " C"); 
  eve_cmd_text (690, 150, 30, 0, eve_buf);
  sprintf (eve_buf, "%s", "o"); 
  eve_cmd_text (685, 145, 29, 0, eve_buf);  // <> nasty hack, look for new font

  // GroBox temperatures
  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    // mark PID temperature control active/inactive & HotBed Device attached
    if (gb[i].hb_attc)  // check Device attached
    {
      if (gb[i].hb_pid) // check PID active 
      {
        eve_cmd_write (COLOR_A | 0xff);
        eve_cmd_write (COLOR_RGB | EVE_COL_OLD);
      }
      else
      {
        eve_cmd_write (COLOR_A | 0x80);
        eve_cmd_write (COLOR_RGB | EVE_COL_BLU);
      }
    }
    else
    {
      eve_cmd_write (COLOR_A | 0x80);
      eve_cmd_write (COLOR_RGB | EVE_COL_RED);
    }

    // Tsetpoint
    sprintf (eve_buf, "%2ld.%ld", gb[i].hb_tset/100, (gb[i].hb_tset%100)/10); 
    eve_cmd_text (170+(i*200), 40, 31, 0, eve_buf);

    // set color text
    if (gb[i].hb_attc)  // check Device attached
    {
      eve_cmd_write (COLOR_A | 0xff);
      eve_cmd_write (COLOR_RGB | EVE_COL_OLD);
    }
    else
    {
      eve_cmd_write (COLOR_A | 0x80);
      eve_cmd_write (COLOR_RGB | EVE_COL_RED);
    }

    // Tactual & Tambient
    sprintf (eve_buf, "%2ld.%ld", (gb[i].hb_tact+5)/100, ((gb[i].hb_tact+5)%100)/10); // note: '+5' for rounding
    eve_cmd_text (170+(i*200), 90, 31, 0, eve_buf);
    sprintf (eve_buf, "%2ld.%ld", (gb[i].hb_tamb+5)/100, ((gb[i].hb_tamb+5)%100)/10); // note: '+5' for rounding
    eve_cmd_text (170+(i*200), 140, 31, 0, eve_buf);
  }

  // end drawing graphics primitive
  eve_cmd_write (END);
}

// display feed water conditions
void tft_mon_water (void)
{
  // bar over feed water data
  eve_cmd_write (COLOR_RGB | EVE_COL_YEL);
  eve_cmd_write (COLOR_A | 0x10);
  eve_cmd_write (LINE_WIDTH | 20*16);
  eve_cmd_write (BEGIN | EVE_RECTS);
  eve_cmd_write (VERTEX2F ( 50*16,  250*16));
  eve_cmd_write (VERTEX2F (750*16, (250+130)*16));

  // set text color
  eve_cmd_write (COLOR_A | 0xff);
  eve_cmd_write (COLOR_RGB | EVE_COL_OLD);

  // label pH
  sprintf (eve_buf, "%s", "pH"); 
  eve_cmd_text (50,  250, 30, 0, eve_buf);
  sprintf (eve_buf, "%s", " "); 
  eve_cmd_text (690, 250, 30, 0, eve_buf);

  // label EC
  sprintf (eve_buf, "%s", "EC"); 
  eve_cmd_text (50,  300, 30, 0, eve_buf);
  sprintf (eve_buf, "%s", "mS"); 
  eve_cmd_text (690, 300, 30, 0, eve_buf);

  // label water temperature
  sprintf (eve_buf, "%s", "H 0"); 
  eve_cmd_text (50,  350, 30, 0, eve_buf);
  sprintf (eve_buf, "%s", "2"); 
  eve_cmd_text (70,  365, 29, 0, eve_buf);  // <> nasty hack, look for new font
  sprintf (eve_buf, "%s", " C"); 
  eve_cmd_text (690, 350, 30, 0, eve_buf);
  sprintf (eve_buf, "%s", "o"); 
  eve_cmd_text (685, 345, 29, 0, eve_buf);  // <> nasty hack, look for new font

  // feed water conditions
  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    // pH
    sprintf (eve_buf, "%d.%02d", ms->ph[i]/100, (ms->ph[i]%100)/1);
    eve_cmd_write (TAG | (phcal_tag[i]));   // tag pH calibration 'button'
    eve_cmd_text (170+(i*200), 240, 31, 0, eve_buf);
    eve_cmd_write (TAG | TAG0);     

    // EC
    sprintf (eve_buf, "%d.%02d", ms->ec[i]/1000, (ms->ec[i]%1000)/10);
    eve_cmd_write (TAG | (eccal_tag[i]));   // tag EC calibration 'button'
    eve_cmd_text (170+(i*200), 290, 31, 0, eve_buf);
    eve_cmd_write (TAG | TAG0);     

    // water temperature
    sprintf (eve_buf, "%d.%01d", ms->ntc[i]/100, ((ms->ntc[i]+5)%100)/10);  // '+5' for rounding
    eve_cmd_text (170+(i*200), 340, 31, 0, eve_buf);
  }

  // magic void command (tags won't work without it, WTF???)
  eve_cmd_track (0, 0, 0, 0, TAG0);

  // end drawing graphics primitive
  eve_cmd_write (END);
}

// display panel switch buttons or pH/EC calibration menu
void tft_mon_hmi (void)
{
  if ((!ms->ph_cal_busy) && (!ms->ec_cal_busy))
  {
    tft_draw_panelswitch ();
  }
  else if (ms->ph_cal_busy)
  {
    // pH calibration busy
    if (!ms->ph_cal_start)
    {
      // display pH calibration ack window
      tft_mon_phack (100, 100, 600, 200);     
    }
    else
    {
      // display pH calibration progress window
      tft_mon_phprogress (100, 100, 600, 200);     
    }
  }
  else if (ms->ec_cal_busy)
  {
    // EC calibration busy
    if (!ms->ec_cal_start)
    {
      // display EC calibration ack window
      tft_mon_ecack (100, 100, 600, 200);     
    }
    else
    {
      // display EC calibration progress window
      tft_mon_ecprogress (100, 100, 600, 200);     
    }
  }
}

// display pH calibration ack window 
void tft_mon_phack (uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height)
{
  const uint16_t XO = 20;   // text x-offset 
  const uint16_t YO = 28;   // text y-offset 
  const uint16_t BW = 190;  // button width
  const uint16_t BH = 90;   // button height
  const uint16_t BS = 20;   // button spacing

  // draw window
  eve_cmd_write (BEGIN | EVE_RECTS);  
  eve_cmd_write (COLOR_RGB | EVE_COL_WINDOW);
  eve_cmd_write (COLOR_A | 0xff); 
  eve_cmd_write (LINE_WIDTH | 10*16); 
  eve_cmd_write (VERTEX2F ( x_pos*16,         y_pos*16));
  eve_cmd_write (VERTEX2F ((x_pos+width)*16, (y_pos+height)*16));

  // draw text
  eve_cmd_write (COLOR_RGB | EVE_COL_OLD);
  sprintf (eve_buf, "sure to calibrate pH sensor %d?", ms->ph_cal_probe+1);
  eve_cmd_text (x_pos+XO, y_pos+YO, 31, 0, eve_buf);
  eve_cmd_write (END);
      
  // draw 'YES' button
  eve_cmd_write   (BEGIN | EVE_RECTS);  
  eve_cmd_write   (TAG | TAG_PHYES);
  eve_cmd_fgcolor (EVE_COL_BUTTONYES);
  eve_cmd_bgcolor (EVE_COL_BLK);
  eve_cmd_button  (x_pos + (width-BS)/2 - BW , y_pos + height/2, BW, BH, 31, EVE_OPT_FLAT, "YES");
  eve_cmd_write   (END);

  // draw 'NO' button
  eve_cmd_write   (BEGIN | EVE_RECTS);  
  eve_cmd_write   (TAG | TAG_PHNO);
  eve_cmd_fgcolor (EVE_COL_BUTTONNO);
  eve_cmd_bgcolor (EVE_COL_BLK);
  eve_cmd_button  (x_pos + (width+BS)/2, y_pos + height/2, BW, BH, 31, EVE_OPT_FLAT, "NO");
  eve_cmd_write   (END);
  
  // clear tag
  eve_cmd_write (TAG | TAG0);
}

// display pH calibration progress window 
void tft_mon_phprogress (uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height)
{
  const uint16_t XO = 40;  // text x-offset
  const uint16_t YO = 50;  // text y-offset
  const uint16_t XS = 40;  // progress bar x spacing
  const uint16_t YS = 150; // progress bar y spacing

  // draw window
  eve_cmd_write (BEGIN | EVE_RECTS);  

  // set window color
  if (ms->ph_cal_stage == PH_CAL_ERR)
    eve_cmd_write (COLOR_RGB | EVE_COL_BUTTONNO);
  else
    eve_cmd_write (COLOR_RGB | EVE_COL_OLD);

  // draw window area
  eve_cmd_write (COLOR_A | 0xf0);           
  eve_cmd_write (VERTEX2F ( x_pos*16,         y_pos*16));
  eve_cmd_write (VERTEX2F ((x_pos+width)*16, (y_pos+height)*16));

  // set normal text color
  eve_cmd_write (COLOR_RGB | EVE_COL_BLK);

  // display window text
  if (ms->ph_cal_stage == PH_CAL_4_WAIT)
    sprintf (eve_buf, "put probe in buffer pH 4");
  else if (ms->ph_cal_stage == PH_CAL_CLEAN)
    sprintf (eve_buf, "clean probe in water");                          
  else if (ms->ph_cal_stage == PH_CAL_7_WAIT)
    sprintf (eve_buf, "put probe in buffer pH 7");
  else if (ms->ph_cal_stage == PH_CAL_OK)
    sprintf (eve_buf, "calibration succeeded");                         
  else if (ms->ph_cal_stage == PH_CAL_ERR)   
    sprintf (eve_buf, "failed, check pH probe %d!", ms->ph_cal_probe+1);
  else
    sprintf (eve_buf, "%s", "");  // suppress random text output

  eve_cmd_text (x_pos+XO, y_pos+YO, 31, 0, eve_buf);

  // only display progress bar when needed
  if ((ms->ph_cal_stage == PH_CAL_4_WAIT) ||
      (ms->ph_cal_stage == PH_CAL_CLEAN)  ||
      (ms->ph_cal_stage == PH_CAL_7_WAIT))
  {
    eve_cmd_fgcolor (EVE_COL_GRY1);
    eve_cmd_bgcolor (EVE_COL_OLD);
    eve_cmd_progress (x_pos+XS, y_pos+YS, width-(2*XS), 30, 0, ms->ph_cal_progress, ms->ph_cal_waittime);
  }

  // end drawing graphics primitive
  eve_cmd_write (END);
}

// display EC calibration ack window 
void tft_mon_ecack (uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height)
{
  const uint16_t XO = 20;   // text x-offset 
  const uint16_t YO = 28;   // text y-offset 
  const uint16_t BW = 190;  // button width
  const uint16_t BH = 90;   // button height
  const uint16_t BS = 20;   // button spacing

  // draw window
  eve_cmd_write (BEGIN | EVE_RECTS);  
  eve_cmd_write (COLOR_RGB | EVE_COL_WINDOW);
  eve_cmd_write (COLOR_A | 0xff); 
  eve_cmd_write (LINE_WIDTH | 10*16); 
  eve_cmd_write (VERTEX2F ( x_pos*16,         y_pos*16));
  eve_cmd_write (VERTEX2F ((x_pos+width)*16, (y_pos+height)*16));

  // draw text
  eve_cmd_write (COLOR_RGB | EVE_COL_OLD);
  sprintf (eve_buf, "sure to calibrate EC sensor %d?", ms->ec_cal_probe+1);
  eve_cmd_text (x_pos+XO, y_pos+YO, 31, 0, eve_buf);
  eve_cmd_write (END);
      
  // draw 'YES' button
  eve_cmd_write   (BEGIN | EVE_RECTS);  
  eve_cmd_write   (TAG | TAG_ECYES);
  eve_cmd_fgcolor (EVE_COL_BUTTONYES);
  eve_cmd_bgcolor (EVE_COL_BLK);
  eve_cmd_button  (x_pos + (width-BS)/2 - BW , y_pos + height/2, BW, BH, 31, EVE_OPT_FLAT, "YES");
  eve_cmd_write   (END);

  // draw 'NO' button
  eve_cmd_write   (BEGIN | EVE_RECTS);  
  eve_cmd_write   (TAG | TAG_ECNO);
  eve_cmd_fgcolor (EVE_COL_BUTTONNO);
  eve_cmd_bgcolor (EVE_COL_BLK);
  eve_cmd_button  (x_pos + (width+BS)/2, y_pos + height/2, BW, BH, 31, EVE_OPT_FLAT, "NO");
  eve_cmd_write   (END);
  
  // clear tag
  eve_cmd_write (TAG | TAG0);
}

// display EC calibration progress window 
void tft_mon_ecprogress (uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height)
{
  const uint16_t XO = 40;  // text x-offset
  const uint16_t YO = 50;  // text y-offset
  const uint16_t XS = 40;  // progress bar x spacing
  const uint16_t YS = 150; // progress bar y spacing

  // draw window
  eve_cmd_write (BEGIN | EVE_RECTS);  

  // set window color
  eve_cmd_write (COLOR_RGB | EVE_COL_OLD);

  // draw window area
  eve_cmd_write (COLOR_A | 0xf0);           
  eve_cmd_write (VERTEX2F ( x_pos*16,         y_pos*16));
  eve_cmd_write (VERTEX2F ((x_pos+width)*16, (y_pos+height)*16));

  // set normal text color
  eve_cmd_write (COLOR_RGB | EVE_COL_BLK);

  // display window text
  if (ms->ec_cal_stage == EC_CAL_BUSY)
    sprintf (eve_buf, "put probe in solution 3 mS");
  else
    sprintf (eve_buf, "%s", "");  // suppress random text output

  eve_cmd_text (x_pos+XO, y_pos+YO, 31, 0, eve_buf);

  // only display progress bar when needed
  if (ms->ec_cal_stage == EC_CAL_BUSY)
  {
    eve_cmd_fgcolor (EVE_COL_GRY1);
    eve_cmd_bgcolor (EVE_COL_OLD);
    eve_cmd_progress (x_pos+XS, y_pos+YS, width-(2*XS), 30, 0, ms->ec_cal_progress, ms->ec_cal_waittime);
  }

  // end drawing graphics primitive
  eve_cmd_write (END);
}
