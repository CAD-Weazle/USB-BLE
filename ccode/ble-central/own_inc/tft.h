// file    : tft.h
// author  : rb
// purpose : header file for tft.c
// date    : 170620
// last    : 191216
//

#ifndef _TFT_H_
#define _TFT_H_

// -- defines
#define DAWN                                    0  // SimSun LEDs on
#define DUSK                                    1  // SimSUn LEDs off

// color defines
#define EVE_COL_RED                    0xff0000UL
#define EVE_COL_ORG                    0xffa500UL
#define EVE_COL_GRN                    0x00ff00UL
#define EVE_COL_BLU                    0x0000ffUL
#define EVE_COL_BL1                    0x5dade2UL
#define EVE_COL_YEL                    0xffff00UL
#define EVE_COL_PNK                    0xff00ffUL
#define EVE_COL_PUR                    0x800080UL
#define EVE_COL_WHT                    0xffffffUL
#define EVE_COL_BLK                    0x000000UL 
#define EVE_COL_GRY                    0x202020UL 
#define EVE_COL_GRY1                   0x101010UL 
#define EVE_COL_GLD                    0xc4ae00UL  // golden
#define EVE_COL_OLD                    0xede5a6UL  // old lace, rb-edit
#define EVE_COL_MENU                   0x738678UL  // Xanadu
#define EVE_COL_BG                     0x2f3720UL  // Olive Drab
#define EVE_COL_OLDLACE_COMP           0xa6aeedUL  // Old Lace complement
#define EVE_COL_WALLCLOCK              0xbbb351UL  // Vegas Gold
#define EVE_COL_BUTTONYES              0x3d9140UL  // 'YES' button        - Cobalt Green
#define EVE_COL_BUTTONNO               0xaf002aUL  // 'NO' button         - Alabama Grimson
#define EVE_COL_WINDOW                 0x3b3434UL  // menu background     - Dark Lava
#define EVE_COL_LEDS                   0x116100UL  // bottom LEDs buttons - Lincoln Green 
#define EVE_COL_FANS                   0x1251b0UL  // fan control buttons - Sapphire

// TAG defines
#define TAG0                                    0
#define TAG_MIX1                               10
#define TAG_MIX2                               11
#define TAG_MIX3                               12
#define TAG_PAR1                               13 
#define TAG_PAR2                               14
#define TAG_PAR3                               15
#define TAG_DAY                                20
#define TAG_SWL                                21
#define TAG_SWR                                22
#define TAG_EESTORE                            30
#define TAG_BLIGHT                             99
#define TAG_EEWRITE                            31
#define TAG_EECANCEL                           32
#define TAG_PH1                                40
#define TAG_PH2                                41
#define TAG_PH3                                42
#define TAG_PHYES                              43
#define TAG_PHNO                               44
#define TAG_EC1                                50
#define TAG_EC2                                51
#define TAG_EC3                                52
#define TAG_ECYES                              53
#define TAG_ECNO                               54
#define TAG_HBSEL1                             60
#define TAG_HBSEL2                             61
#define TAG_HBSEL3                             62
#define TAG_HBACT1                             63
#define TAG_HBACT2                             64
#define TAG_HBACT3                             65
#define TAG_HBPOS1                             67
#define TAG_HBPOS2                             68
#define TAG_HBPOS3                             69
#define TAG_HBPID1_ACT                         70
#define TAG_HBPID2_ACT                         71
#define TAG_HBPID3_ACT                         72
#define TAG_HBPID1_SET                         73
#define TAG_HBPID2_SET                         74
#define TAG_HBPID3_SET                         75
#define TAG_HBLED1                             80
#define TAG_HBLED2                             81
#define TAG_HBLED3                             82
#define TAG_HBFAN1_MIN                         90
#define TAG_HBFAN1_MID                         91
#define TAG_HBFAN1_MAX                         92
#define TAG_HBFAN2_MIN                         93
#define TAG_HBFAN2_MID                         94
#define TAG_HBFAN2_MAX                         95
#define TAG_HBFAN3_MIN                         96
#define TAG_HBFAN3_MID                         97
#define TAG_HBFAN3_MAX                         98

// TFT display panel selection 
#define TFT_MONITOR                             0
#define TFT_SIMSUN                              1
#define TFT_HOTBED                              2

// general screen defines
#define LMIN_XN                                 0  // minimum y value
#define LMAX_XN                               800  // maximum y value
#define LMIN_YN                                 0  // minimum y value
#define LMAX_YN                               480  // maximum y value

#define LMIN_XF                      (LMIN_XN*16)  // in EVE 'VERTEX2F' format
#define LMAX_XF                      (LMAX_XN*16)  // in EVE 'VERTEX2F' format
#define LMIN_YF                      (LMIN_YN*16)  // in EVE 'VERTEX2F' format
#define LMAX_YF                      (LMAX_YN*16)  // in EVE 'VERTEX2F' format

#define TIME_XN                                 4  // wallclock x origin
#define TIME_YN                                 2  // wallclock y origin

// ambient data coordinates
#define MON_AMBIENT_XN                          30  // ambient data x origin
#define MON_AMBIENT_YN                         420  // ambient data y origin

// -- struct TFT state, stored in EEPROM
typedef struct
{ 
  // global
  uint8_t panel;                         // current TFT panel being diplayed
  uint8_t watering;                      // watering busy flag
  uint8_t store;                         // EEPROM store button pressed

  // Monitor Display Panel
  // <none> 

  // SimSun Display Panel
  uint16_t rlen[SW_NUM_GBOXES];          // LED light red/blu mixture [0..100%]
  uint16_t ilen[SW_NUM_GBOXES];          // lenght of intensity bar [0..300PAR]
  uint32_t tlen[2];                      // dawn & dusk in [s] (0..24*60*60)
  
  // HotBed Display Panel
  uint16_t wtime_sel;                    // water timer select day to water
  uint16_t wtime_len;                    // water timer program lenght in [days]
  uint16_t wtime_day;                    // water timer current day [days]
  uint32_t wtime_pos;                    // water timer slider [s]
  uint16_t pid_setpoint[SW_NUM_GBOXES];  // PID heater setpoint rotary slider (0..65535)
  uint8_t  pid_active[SW_NUM_GBOXES];    // PID heater control on/off         (note: Master state only, not SunWeb state)
  uint8_t  bottom_leds[SW_NUM_GBOXES];   // bottom LEDs control [%PWM]        (note: Master state only, not SunWeb state)
  uint8_t  fan_speed[SW_NUM_GBOXES];     // fan speed control off/min/mid/max {note: Master state only, not SunWeb state)

  // dirty bits
  uint8_t  dsk_new;                      // new SimSun dusk settings
  uint8_t  dwn_new;                      // new SimSun dawn settings
  uint8_t  mix_new[SW_NUM_GBOXES];       // new SimSun red/blu light mixture
  uint8_t  par_new[SW_NUM_GBOXES];       // new SimSun light intensity
  uint8_t  led_new[SW_NUM_GBOXES];       // new HotBed bottom LED setting, send over SunWeb
  uint8_t  fan_new[SW_NUM_GBOXES];       // new HotBed fan speed setting, send over SunWeb
  uint8_t  act_new[SW_NUM_GBOXES];       // new HotBed PID active/inactive state, send over SunWeb
  uint8_t  set_new[SW_NUM_GBOXES];       // 
} t_state;

// -- globals
extern t_state *ts;    // pointer to 't_state'
extern uint8_t *ts_p;  // typacasted to char array (for EEPROM read/write)

// -- prototypes
void init_tft (void);

void tft_update (void);

void tft_mon_tags (uint8_t tag, uint16_t track_pos);
void tft_ss_tags  (uint8_t tag, uint16_t track_pos);
void tft_hb_tags  (uint8_t tag, uint16_t track_pos);

void tft_switch_store (uint8_t tag);

void tft_display_screen (void);

void tft_display_time (uint16_t x_pos, uint16_t y_pos);

void tft_display_watering (void);

void tft_draw_panelswitch (void);

void tft_eestore (void);

void tft_touch_calibrate (void);

void tft_get_snapshot (uint32_t adr);
void tft_ppm_snapshot (uint32_t adr, uint16_t width, uint16_t height);

void tft_next_day (void);

void tft_tstate_dump (void);

#endif










