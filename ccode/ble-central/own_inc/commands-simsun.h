// file    : command-simsun.h
// author  : rb
// purpose : header file for commands-simsun.c
// date    : 190112
// last    : 191215
//

#ifndef _COMMANDS_SIMSUN_H_
#define _COMMANDS_SIMSUN_H_

// -- defines

// SimSun commands
#define SW_CMD_SETDAWN       0x10      // set dawn time (LEDs on)
#define SW_CMD_SETDUSK       0x11      // set dusk time (LEDs off)
#define SW_CMD_SETDRED       0x12      // set light intensity Deep Red LEDs
#define SW_CMD_SETRBLU       0x13      // set light intensity Royal Blue LEDs
#define SW_CMD_SETFRED       0x14      // set light intensity Far Red LEDs
#define SW_CMD_SETDREDMAX    0x15      // set maximum light intensity Deep Red LEDs
#define SW_CMD_SETRBLUMAX    0x16      // set maximum light intensity Royal Blue LEDs
#define SW_CMD_SIMSUNSTART   0x17      // start SimSun program
#define SW_CMD_SIMSUNSTOP    0x18      // stop SimSun program
#define SS_CMD_GETSTATE      0x19      // get temperature & LED intensity
#define SS_CMD_NTCERR        0x1a      // write NTC calibration parameter

// -- prototypes
uint8_t dred_to_pwm (float flux);
uint8_t rblu_to_pwm (float flux);

float polynomial (float *p, uint8_t n, float x);

void ss_set_dawn (uint8_t badr, uint32_t time);
void ss_set_dusk (uint8_t badr, uint32_t time);

void ss_set_dred (uint8_t badr, uint8_t pwm);
void ss_set_rblu (uint8_t badr, uint8_t pwm);
void ss_set_fred (uint8_t badr, uint8_t pwm);

void ss_mol_dred (uint8_t badr, uint16_t flux);
void ss_mol_rblu (uint8_t badr, uint16_t flux);

void ss_max_dred (uint8_t badr, uint16_t flux);
void ss_max_rblu (uint8_t badr, uint16_t flux);

void ss_simsun_start (uint8_t badr);
void ss_simsun_stop  (uint8_t badr);

void ss_set_clock  (uint8_t badr);
void ss_sync_clock (uint8_t badr);

void ss_get_state (uint8_t badr);
void ss_set_ntc   (uint8_t badr, int16_t err);

#endif



