// file    : tft-simsun.c
// author  : rb
// purpose : SimSun TFT interface
// date    : 191120
// last    : 191216
//

#include "includes.h"

// touch tags
uint16_t mix_tag[SW_NUM_GBOXES] = {TAG_MIX1, TAG_MIX2, TAG_MIX3};  // touch tags for color mix slider
uint16_t par_tag[SW_NUM_GBOXES] = {TAG_PAR1, TAG_PAR2, TAG_PAR3};  // touch tags for intensity slider

// initialize SimSun TFT panel
void init_ss_tft (void)
{
}

// display SimSun control panel
void tft_ss_panel (void)
{
  // start writing to command FIFO
  eve_cmd_start (EVE_COL_GRY);      

  // display wall clock
  tft_display_time (TIME_XN, TIME_YN);

  // display light mixture & light intensity slider bars
  tft_ss_mixsliders ();

  // display day length slider
  tft_ss_daysliders ();

  // draw invisible panel switch buttons or EEPROM store menu
  tft_ss_hmi ();

  // draw watering busy state 
  tft_display_watering ();

  // mark end of Display List & swap display buffers
  eve_cmd_stop ();
}

// display invisible panel switch buttons or EEPROM store menu
void tft_ss_hmi (void)
{
  if (!ts->store)
    tft_draw_panelswitch ();
  else
    tft_eestore ();
}

// draw SimSun light mixture slider bars
void tft_ss_mixsliders (void)
{
  // start drawing graphics primitive
  eve_cmd_write (BEGIN | EVE_RECTS);  

  // draw SimSun LED color mixture & intensity slider bars
  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    // LED color mix: blue background
    eve_cmd_write (TAG | mix_tag[i]);
    eve_cmd_write (COLOR_RGB | EVE_COL_BLU);
    eve_cmd_write (VERTEX2F (SS_LBAR_XF             , SS_LBAR_YF + i*SS_LBAR_PF));  
    eve_cmd_write (VERTEX2F (SS_LBAR_XF + SS_LBAR_WF, SS_LBAR_YF + i*SS_LBAR_PF + SS_LBAR_HF));  
    eve_cmd_track (SS_LBAR_XN, SS_LBAR_YN + i*SS_LBAR_PN, SS_LBAR_WN, SS_LBAR_HN, mix_tag[i]);
    
    // LED color mix: draw red slider
    eve_cmd_write (COLOR_RGB | EVE_COL_RED);
    eve_cmd_write (VERTEX2F (SS_LBAR_XF                               , SS_LBAR_YF + i*SS_LBAR_PF));  
    eve_cmd_write (VERTEX2F (SS_LBAR_XF + (ts->rlen[i]*SS_LBAR_WF)/100, SS_LBAR_YF + i*SS_LBAR_PF + SS_LBAR_HF));  

    // light intensity: draw background
    eve_cmd_write (TAG | par_tag[i]);
    eve_cmd_write (COLOR_RGB | EVE_COL_BG);
    eve_cmd_write (VERTEX2F (SS_IBAR_XF             , SS_IBAR_YF + i*SS_IBAR_PF + SS_LBAR_HF));  
    eve_cmd_write (VERTEX2F (SS_IBAR_XF + SS_LBAR_WF, SS_IBAR_YF + i*SS_IBAR_PF + SS_LBAR_HF + SS_IBAR_HF));  
    eve_cmd_track (SS_IBAR_XN, SS_IBAR_YN + i*SS_IBAR_PN + SS_LBAR_HN, SS_IBAR_WN, SS_IBAR_HN, par_tag[i]);

    // light intensity: draw slider
    eve_cmd_write (COLOR_RGB | EVE_COL_OLD);
    eve_cmd_write (VERTEX2F (SS_LBAR_XF                               , SS_IBAR_YF + i*SS_LBAR_PF + SS_LBAR_HF));  
    eve_cmd_write (VERTEX2F (SS_LBAR_XF + (ts->ilen[i]*SS_IBAR_WF)/300, SS_IBAR_YF + i*SS_LBAR_PF + SS_LBAR_HF + SS_IBAR_HF));  
  }

  // clear tag
  eve_cmd_write (TAG | TAG0);

  // ligth mixture slider bar text
  eve_cmd_write (COLOR_RGB | EVE_COL_OLD);

  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    sprintf (eve_buf, "%d%%", ts->rlen[i]); 
    eve_cmd_text (SS_LBAR_XN               , SS_LBAR_YN + i*SS_LBAR_PN - SS_LBAR_TN, 24, 0, eve_buf);

    sprintf (eve_buf, "%d%%", 100 - ts->rlen[i]); 
    eve_cmd_text (SS_LBAR_XN+SS_LBAR_WN-50 , SS_LBAR_YN + i*SS_LBAR_PN - SS_LBAR_TN, 24, 0, eve_buf);

    sprintf (eve_buf, "PAR %d", ts->ilen[i]); 
    eve_cmd_text (SS_LBAR_XN+SS_LBAR_WN-350, SS_LBAR_YN + i*SS_LBAR_PN - SS_LBAR_TN, 24, 0, eve_buf);
  }

  // end drawing graphics primitive
  eve_cmd_write (END);  
}

// draw SimSun day length slider
void tft_ss_daysliders (void)
{
  uint16_t hrs =  ts->tlen[DAWN] / (60*60);           // calculate hours from seconds
  uint16_t min = (ts->tlen[DAWN] - hrs*60*60) / 60;   // calculate minutes from seconds

  // start drawing graphics primitive
  eve_cmd_write (BEGIN | EVE_RECTS);  

  // draw background bar
  eve_cmd_write (TAG | TAG_DAY);
  eve_cmd_write (COLOR_RGB | EVE_COL_BG);
  eve_cmd_write (VERTEX2F (SS_TBAR_XF             , SS_TBAR_YF + SW_NUM_GBOXES*SS_IBAR_PF));  
  eve_cmd_write (VERTEX2F (SS_TBAR_XF + SS_IBAR_WF, SS_TBAR_YF + SW_NUM_GBOXES*SS_IBAR_PF + SS_TBAR_HF));  
  eve_cmd_track (SS_TBAR_XN, SS_TBAR_YN + SW_NUM_GBOXES*SS_IBAR_PN + SS_TBAR_HN, SS_TBAR_WN, SS_TBAR_HN, TAG_DAY);

  // draw dual side slider bar
  eve_cmd_write (COLOR_RGB | EVE_COL_GLD);
  eve_cmd_write (VERTEX2F (SS_TBAR_XF + (ts->tlen[DAWN]*SS_LBAR_WF)/(24*60*60), SS_TBAR_YF + SW_NUM_GBOXES*SS_IBAR_PF));  
  eve_cmd_write (VERTEX2F (SS_TBAR_XF + (ts->tlen[DUSK]*SS_LBAR_WF)/(24*60*60), SS_TBAR_YF + SW_NUM_GBOXES*SS_IBAR_PF + SS_TBAR_HF));  

  // clear tag 
  eve_cmd_write (TAG | TAG0);

  // set text color & display day time slider bar text
  eve_cmd_write (COLOR_RGB | EVE_COL_OLD);
  sprintf (eve_buf, "DAWN %02d:%02d", hrs, min); 
  eve_cmd_text (SS_IBAR_XN, SS_IBAR_YN + SW_NUM_GBOXES*SS_LBAR_PN - SS_IBAR_TN - 20, 24, 0, eve_buf);

  // nasty hack: don't allow 24:00 as end time (due to SimSun bug)
  if (ts->tlen[DUSK] == 24*60*60)
    ts->tlen[DUSK] -= 15*60;  // set dawn back 15 minutes

  hrs =  ts->tlen[DUSK] / (60*60);
  min = (ts->tlen[DUSK] - hrs*60*60) / 60;

  sprintf (eve_buf, "%02d:%02d DUSK", hrs, min); 
  eve_cmd_text  (SS_IBAR_XN + 460, SS_IBAR_YN + 3*SS_LBAR_PN - SS_IBAR_TN - 20, 24, 0, eve_buf);

  hrs = (ts->tlen[DUSK] - ts->tlen[DAWN]) / (60*60);
  min = (ts->tlen[DUSK] - ts->tlen[DAWN] - hrs*60*60) / 60;

  sprintf (eve_buf, "DAY %02d:%02d", hrs, min); 
  eve_cmd_text  (SS_IBAR_XN + 245, SS_IBAR_YN + 3*SS_LBAR_PN - SS_IBAR_TN - 20, 24, 0, eve_buf);

  // end drawing graphics primitive
  eve_cmd_write (END);  
}
