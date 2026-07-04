// file    : tft-simsun.h
// author  : rb
// purpose : header file for tft-simsun.c
// date    : 191120
// last    : 191210
//

#ifndef _TFT_SIMSUN_H_
#define _TFT_SIMSUN_H_

// -- defines
// LED red/blue color mixture
#define SS_LBAR_XN                            100  // red/blu light mixture slider x origin
#define SS_LBAR_YN                             50  // red/blu light mixture slider y origin
#define SS_LBAR_WN                            600  // red/blu light mixture slider width
#define SS_LBAR_HN                             30  // red/blu light mixture slider heigth
#define SS_LBAR_PN                            120  // red/blu light mixture slider pitch
#define SS_LBAR_TN                             34  // red/blu light slider text y-offset

#define SS_LBAR_XF                (SS_LBAR_XN*16)  // in EVE 'VERTEX2F' format
#define SS_LBAR_YF                (SS_LBAR_YN*16)  // in EVE 'VERTEX2F' format
#define SS_LBAR_WF                (SS_LBAR_WN*16)  // in EVE 'VERTEX2F' format
#define SS_LBAR_HF                (SS_LBAR_HN*16)  // in EVE 'VERTEX2F' format
#define SS_LBAR_PF                (SS_LBAR_PN*16)  // in EVE 'VERTEX2F' format

// light intensity 
#define SS_IBAR_XN                     SS_LBAR_XN  // light intensity slider x origin
#define SS_IBAR_YN                     SS_LBAR_YN  // light intensity slider y origin
#define SS_IBAR_WN                     SS_LBAR_WN  // light intensity slider width
#define SS_IBAR_HN                     SS_LBAR_HN  // light intensity slider heigth
#define SS_IBAR_PN                     SS_LBAR_PN  // light intensity slider pitch
#define SS_IBAR_TN                             15  // light intensity slider text y-offset

#define SS_IBAR_XF                (SS_IBAR_XN*16)  // in EVE 'VERTEX2F' format
#define SS_IBAR_YF                (SS_IBAR_YN*16)  // in EVE 'VERTEX2F' format
#define SS_IBAR_WF                (SS_IBAR_WN*16)  // in EVE 'VERTEX2F' format
#define SS_IBAR_HF                (SS_IBAR_HN*16)  // in EVE 'VERTEX2F' format
#define SS_IBAR_PF                (SS_IBAR_PN*16)  // in EVE 'VERTEX2F' format

// day time bar
#define SS_TBAR_XN                     SS_LBAR_XN  // time slider x origin
#define SS_TBAR_YN                     SS_LBAR_YN  // time slider y origin
#define SS_TBAR_WN                     SS_LBAR_WN  // time slider wifth
#define SS_TBAR_HN                (SS_LBAR_HN+10)  // time slider heigth

#define SS_TBAR_XF                (SS_TBAR_XN*16)  // time slider x origin
#define SS_TBAR_YF                (SS_TBAR_YN*16)  // time slider y origin
#define SS_TBAR_HF                (SS_TBAR_HN*16)  // time slider heigth


// -- prototypes
void init_ss_tft (void);

void tft_ss_panel (void);
void tft_ss_hmi   (void);

void tft_ss_mixsliders (void);
void tft_ss_daysliders (void);

#endif

