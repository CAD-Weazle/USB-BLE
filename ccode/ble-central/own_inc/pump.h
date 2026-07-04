// file    : pump.h
// author  : rb
// purpose : header file for pump.c
// date    : 190911
// last    : 200207

#ifndef _PUMP_H_
#define _PUMP_H_

// -- defines
#define PUMP_SOUND_VOLUME                        20

#define PUMP_NUM_DEVICES                          3   // number of pump

#define PUMP_PWM_INITIAL                         90   // initial pump flow (%PWM]
#define PUMP_PWM_STEADY                          35   // reduced steady state flow [%PWM]
#define PUMP_TIME                                13   // time pump is running [s]
#define PUMP_INTERDELAY                           2   // time between pump operations

// state machine
#define PUMP_START                                0     
#define PUMP_BUSY                                 1
#define PUMP_DONE                                 2     

// -- prototypes
void init_pump (void);

void pump_update (void);

void pump_go (uint8_t num, uint8_t delay);

void pump_on (void);
void pump_off (void);

void pump_set_start (uint32_t tsecs);
void pump_get_start (void);

void pump_check (void);

#endif



