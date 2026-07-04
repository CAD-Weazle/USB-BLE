// file    : power.h
// author  : rb
// purpose : header file for power.c
// date    : 171127
// last    : 200312
//

#ifndef _POWER_H_
#define _POWER_H_

// -- defines
#define PWR_MCU_CLEAR      (0b11 << PWR_CR_VOS_Pos)
#define PWR_MCU_1V8        (0b01 << PWR_CR_VOS_Pos)
#define PWR_MCU_1V5        (0b10 << PWR_CR_VOS_Pos)
#define PWR_MCU_1V2        (0b11 << PWR_CR_VOS_Pos)

// -- prototypes
void init_power (void);

void power_down (void);
void power_up   (void);

void power_event_enable  (void);
void power_event_disable (void);

void power_enter_stopmode (void);  // <> obselete

void power_enter_ulpmode (void);
void power_leave_ulpmode (void);

void power_enter_lprmode (void);
void power_leave_lprmode (void);

void power_set_corevoltage (void);

#endif
