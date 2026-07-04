// file    : tft-hotbed.c
// author  : rb
// purpose : HotBed TFT interface
// date    : 191121
// last    : 191218
//

#include "includes.h"

// touch tags HotBed panel
uint16_t hbsel_tag[SW_NUM_GBOXES]    = {TAG_HBSEL1, TAG_HBSEL2, TAG_HBSEL3};               // touch tags for water bar select
uint16_t hbact_tag[SW_NUM_GBOXES]    = {TAG_HBACT1, TAG_HBACT2, TAG_HBACT3};               // touch tags for water bar active
uint16_t hbpos_tag[SW_NUM_GBOXES]    = {TAG_HBPOS1, TAG_HBPOS2, TAG_HBPOS3};               // touch tags for water slider
uint16_t hbpid_tag[SW_NUM_GBOXES]    = {TAG_HBPID1_ACT, TAG_HBPID2_ACT, TAG_HBPID3_ACT};   // touch tags for PID on/off control button
uint16_t hbset_tag[SW_NUM_GBOXES]    = {TAG_HBPID1_SET, TAG_HBPID2_SET, TAG_HBPID3_SET};   // touch tags for PID setpoint slider
uint16_t hbled_tag[SW_NUM_GBOXES]    = {TAG_HBLED1, TAG_HBLED2, TAG_HBLED3};               // touch tags for bottom LED control
uint16_t hbfan_tag[3][SW_NUM_GBOXES] = {{TAG_HBFAN1_MIN, TAG_HBFAN1_MID, TAG_HBFAN1_MAX}, 
                                        {TAG_HBFAN2_MIN, TAG_HBFAN2_MID, TAG_HBFAN2_MAX}, 
                                        {TAG_HBFAN3_MIN, TAG_HBFAN3_MID, TAG_HBFAN3_MAX}}; // touch tags for fan speed control


// initialize HotBed TFT panel
void init_hb_tft (void)
{
  // set pump time, data read from EEPROM
  pump_set_start (ts->wtime_pos);
}

// display HotBed control panel
void tft_hb_panel (void)
{
  // start writing to command FIFO
  eve_cmd_start (EVE_COL_GRY);      

  // display wall clock
  tft_display_time (TIME_XN, TIME_YN);

  // draw heater PID dials
  tft_hb_heaterpid ();

  // draw LED & fan buttons
  tft_hb_ledfans ();

  // draw water pump activation 
  tft_hb_waterbars ();

  // draw invisible panel switch buttons or EEPROM store menu
  tft_hb_hmi ();

  // draw watering busy state 
  tft_display_watering ();

  // stop writing to command FIFO & execute Display List
  eve_cmd_stop ();      
}

// display invisible panel switch buttons or EEPROM store menu
void tft_hb_hmi (void)
{
  if (!ts->store)
    tft_draw_panelswitch ();
  else
    tft_eestore ();
}

// display HotBed temperature control dials
void tft_hb_heaterpid (void)
{
  const uint16_t XP = 260;                  // x-pitch buttons
  const uint16_t SZ = 60;                   // button diameter
  const uint16_t SR = 10;                   // button rim
  const uint16_t XW = SZ*2;                 // x-width stencil block
  const uint16_t YH = SZ;                   // y-heigth stencil block
  const uint16_t YO = 40;                   // y-origin
  const uint16_t XO = (800 - 2*XP - XW)/2;  // x-origin

  const uint16_t XT = XO+32;                // text x-origin
  const uint16_t YT = YO+22;                // text y-origin
  const uint16_t PT = 50;                   // text pitch

  // begin drawing primitives
  eve_cmd_write (BEGIN | EVE_RECTS);

  // set stencil operation
  eve_cmd_write (STENCIL_OP (EVE_ZERO, EVE_INCR));

  // draw invisible blocks - note: 3 loops needed, single loop gives rendering errors
  eve_cmd_write (COLOR_A | 0x00);           

  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    eve_cmd_write (VERTEX2F ((XO+i*XP)*16,     YO*16));
    eve_cmd_write (VERTEX2F ((XO+i*XP+XW)*16, (YO+YH)*16));
  }

  // draw circle 
  eve_cmd_write (COLOR_A | 0xff);           

  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    eve_cmd_write (COLOR_RGB | EVE_COL_BG);
    eve_cmd_point ((XO+i*XP+XW/2), (YO+YH), SZ+SR);  // add rim
  }

  // enable stencil function
  eve_cmd_write (STENCIL_FUNC (EVE_EQUAL, 2, 0xff));

  // set PID button color
  eve_cmd_write (COLOR_RGB | EVE_COL_OLD);  

  // draw stencil: only circle/block overlap
  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    // set color PID active/inactive state
    if (ts->pid_active[i])
      eve_cmd_write (COLOR_A | 0xff);       // PID active color
    else  
      eve_cmd_write (COLOR_A | 0x3f);       // PID inactive color

    // draw button 
    eve_cmd_point ((XO+i*XP+XW/2), (YO+YH), SZ);
  }

  // disable stencil
  eve_cmd_write (STENCIL_FUNC (EVE_EQUAL, 2, 0x00));

  // draw temperatures
  eve_cmd_write (COLOR_RGB | EVE_COL_RED);
  eve_cmd_write (COLOR_A | 0xff);             

  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    uint16_t tmp = (220 + 5*((ts->pid_setpoint[i] - 8960)/2488));  

    // Tsetpoint
    sprintf (eve_buf, "%2d.%d", tmp/10, tmp%10); 
    eve_cmd_text (XT+i*XP, YT, 30, 0, eve_buf);

    // Tactual
    sprintf (eve_buf, "%2ld.%ld", (gb[i].hb_tact+5)/100, ((gb[i].hb_tact+5)%100)/10); // note: '+5' for rounding
    eve_cmd_text (XT+i*XP, YT+PT, 30, 0, eve_buf);
  }

  // draw temperature set slider
  const uint16_t XORG = XO+60;              // x-origin rotary slider 
  const uint16_t YORG = YO+60;              // y-origin rotary slider
  const uint16_t SS   = SZ+40;              // diameter rotary slider tracker area
  const uint16_t RHO  = 80;                 // diameter rotary slider radius

  float phi = 0.0;                          // polar coordinates angle for rotary slider

  // draw rotary slider for PID setpoint adjust
  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    // calculate angle (0..2*PI) from slider position (0..65635)
    phi = (float)(2*M_PI*ts->pid_setpoint[i])/65535;

    // draw invisible rotary slide/tracker
    eve_cmd_write (COLOR_A | 0x00);            
    eve_cmd_write (TAG | hbset_tag[i]);
    eve_cmd_point (XORG + i*XP, YORG, SS);
    eve_cmd_track (XORG + i*XP, YORG, 1, 1, hbset_tag[i]);  // rotary tracker
    eve_cmd_write (TAG | TAG0);     

    // draw marker dot
    eve_cmd_write (COLOR_A | 0xff);            
    eve_cmd_write (COLOR_RGB | EVE_COL_YEL);
    eve_cmd_point (XORG - (int16_t)(RHO*sin(phi)) + i*XP, YORG + (int16_t)(RHO*cos(phi)), 14);
  }

  // draw invisible PID on/off  buttons
  eve_cmd_write (COLOR_A | 0x00);           

  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    eve_cmd_write (TAG | hbpid_tag[i]);
    eve_cmd_point ((XO+i*XP+XW/2), (YO+YH), SZ);
    eve_cmd_write (TAG | TAG0);
  }

  // stop drawing primitives
  eve_cmd_write (END);
}

// display bottom LED & fan buttons
void tft_hb_ledfans (void)
{
  const uint16_t XO = 100;                  // x-origin
  const uint16_t YO = 205;                  // y-origin
  const uint16_t XP = 260;                  // x-pitch
  const uint16_t YP = 36;                   // y-pitch
  const uint16_t XW = 80;                   // button width
  const uint16_t YH = 14;                   // button height
  const uint16_t YD = (XW/2);               // y-distance fan buttons

  // start drawing primitives
  eve_cmd_write (BEGIN | EVE_LINES);
  eve_cmd_write (LINE_WIDTH | (YH-2)*16);

  // draw bottom LED buttons
  eve_cmd_write (COLOR_RGB | EVE_COL_LEDS);

  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    // set on/off color
    if (ts->bottom_leds[i])
      eve_cmd_write (COLOR_A | 0xff);           
    else
      eve_cmd_write (COLOR_A | 0x3f);           

    // draw buttons
    eve_cmd_write (TAG | hbled_tag[i]);
    eve_cmd_write (VERTEX2F ((XO+i*XP)*16,    YO*16));
    eve_cmd_write (VERTEX2F ((XO+i*XP+XW)*16, YO*16));
    eve_cmd_write (TAG | TAG0);
  }

  // draw fan control buttons
  eve_cmd_write (COLOR_RGB | EVE_COL_FANS);

  // set on/off color
  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    if (ts->fan_speed[i] == HB_FAN_OFF)
      eve_cmd_write (COLOR_A | 0x3f);           
    else
      eve_cmd_write (COLOR_A | 0xff);           

    eve_cmd_write (TAG | hbfan_tag[i][0]);
    eve_cmd_point ((XO+i*XP+0*YD), (YO+YP), YH);
    eve_cmd_write (TAG | TAG0);

    if (ts->fan_speed[i] >= HB_FAN_MID)
      eve_cmd_write (COLOR_A | 0xff);           
    else
      eve_cmd_write (COLOR_A | 0x3f);           

    eve_cmd_write (TAG | hbfan_tag[i][1]);
    eve_cmd_point ((XO+i*XP+1*YD), (YO+YP), YH);
    eve_cmd_write (TAG | TAG0);

    if (ts->fan_speed[i] == HB_FAN_MAX)
      eve_cmd_write (COLOR_A | 0xff);           
    else
      eve_cmd_write (COLOR_A | 0x3f);           

    eve_cmd_write (TAG | hbfan_tag[i][2]);
    eve_cmd_point ((XO+i*XP+2*YD), (YO+YP), YH);
    eve_cmd_write (TAG | TAG0);
  }

  // stop drawing primitives
  eve_cmd_write (END);
}

// display timing bars for watering pumps control
void tft_hb_waterbars (void)
{
  const uint16_t XO = 80;                   // x-origin first bar
  const uint16_t YO = 310;                  // y-origin first bar
  const uint16_t XL = (800 - 2*XO);         // bar length 
  const uint16_t YH = 20;                   // bar height
  const uint16_t YP = 65;                   // inter-bar pitch
  const uint16_t XB = 30;                   // background x length position
  const uint16_t YB = 16;                   // background y height position
  const uint16_t TF = DAY_SECS/(XL-2*XB);   // time bar division factor

  // begin drawing primitives
  eve_cmd_write (BEGIN | EVE_RECTS);

  // draw watering bars outline
  eve_cmd_write (COLOR_A | 0xff);          
  eve_cmd_write (LINE_WIDTH | YH*16);
  eve_cmd_write (COLOR_RGB | EVE_COL_BG);

  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    eve_cmd_write (VERTEX2F ( XO*16,     (YO + i*YP)*16));
    eve_cmd_write (VERTEX2F ((XO+XL)*16, (YO + i*YP)*16));
  }

  // draw watering bars background
  eve_cmd_write (LINE_WIDTH | 1*16);
  eve_cmd_write (COLOR_RGB | EVE_COL_BLK);

  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    eve_cmd_write (VERTEX2F ((XO+XB)*16,    (YO-YB + i*YP)*16));
    eve_cmd_write (VERTEX2F ((XO-XB+XL)*16, (YO+YB + i*YP)*16));
  }

  // draw dynamic progress bar (24*60*60 = 86400 [s] per day)
  eve_cmd_write (COLOR_RGB | EVE_COL_BG);

  // draw dynamic time bar ('ms->tsecs' = daytime in [s])
  eve_cmd_write (VERTEX2F ((XO+XB-1)*16,                (YO-YB-1 + ts->wtime_day*YP)*16));
  eve_cmd_write (VERTEX2F ((XO+XB+1+(ms->tsecs/TF))*16, (YO+YB+1 + ts->wtime_day*YP)*16));

  // draw full time bars for past days
  for (int i = 0; i < ts->wtime_day; i++)
  {
    eve_cmd_write (VERTEX2F ((XO+XB-1)*16,    (YO-YB-1 + i*YP)*16)); 
    eve_cmd_write (VERTEX2F ((XO-XB+1+XL)*16, (YO+YB+1 + i*YP)*16));
  }

  // draw invisible slider trackers
  eve_cmd_write (COLOR_A | 0x00);     

  for (int i = 0; i < SW_NUM_GBOXES; i++)
  {
    eve_cmd_write (TAG | hbpos_tag[i]);
    eve_cmd_write (VERTEX2F ((XO+XB)*16,    (YO-YB + i*YP)*16));
    eve_cmd_write (VERTEX2F ((XO-XB+XL)*16, (YO+YB + i*YP)*16));
    eve_cmd_track ((XO+XB), (YO-YB + i*YP), XL-2*XB, YH, hbpos_tag[i]);
    eve_cmd_write (TAG | TAG0);     
  }

  // draw watering time adjustment slider
  eve_cmd_write (COLOR_A | 0xff);    
  eve_cmd_write (COLOR_RGB | EVE_COL_YEL);
  eve_cmd_write (VERTEX2F ((XO+XB + (ts->wtime_pos/TF)-10)*16, (YO-20 + ts->wtime_sel*YP)*16));
  eve_cmd_write (VERTEX2F ((XO+XB + (ts->wtime_pos/TF)+10)*16, (YO+20 + ts->wtime_sel*YP)*16));

  // stop drawing primitives
  eve_cmd_write (END);

  // draw watering time just above slider
  uint16_t whrs = ts->wtime_pos/(60*60);
  uint16_t wmin = (ts->wtime_pos - whrs*60*60) / 60;
  
  eve_cmd_write (COLOR_RGB | EVE_COL_YEL);
  sprintf (eve_buf, "%02d:%02d", whrs, wmin);
  eve_cmd_text ((XO+6 + ts->wtime_pos/TF), (YO-44 + ts->wtime_sel*YP), 23, 0, eve_buf);

  // draw select buttons left side - day to water
  for (int i = 0; i < 3; i++)
  {
    // select active/inactive color
    if ((i == ts->wtime_sel) && (i <= ts->wtime_len)) // only active days can be selected
      eve_cmd_write (COLOR_RGB | EVE_COL_OLD);
    else
      eve_cmd_write (COLOR_RGB | EVE_COL_BLK);

    // draw button
    eve_cmd_write (TAG | (hbsel_tag[i]));
    eve_cmd_point (XO, YO + i*YP, YH-4);
    eve_cmd_write (TAG | TAG0);     
  }

  // draw activation buttons right side - length of watering cycle
  for (int i = 0; i < 3; i++)
  {
    // select active/inactive color
    if (i <= ts->wtime_len)
      eve_cmd_write (COLOR_RGB | EVE_COL_BLU);
    else
      eve_cmd_write (COLOR_RGB | EVE_COL_BLK);

    // draw button
    eve_cmd_write (TAG | (hbact_tag[i]));
    eve_cmd_point (XO+XL, YO + i*YP, YH-4);
    eve_cmd_write (TAG | TAG0);     
  }
}

