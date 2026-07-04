// file    : ec.h
// author  : rb
// purpose : header file for ec.c
// date    : 190502
// last    : 191210
//

#ifndef _EC_H_
#define _EC_H_

// -- defines
#define EC_NUM_DEVICES       3         // number of EC sensors
#define EC_FILTERDEPTH      64         // moving average filter depth
#define EC_WAVE_DELAY        2         // excitation wave inter-sample delay
#define EC_CAL_WAIT       1200         // waiting time during calibration [s/10]

#define NUM_DOTS           128         // number of points single full sine wave
#define SINE_AMP           400         // amplitude excitation signal (/2 to get [mV])
#define PI         3.141592654         // good old pi, you know...

#define EC_SENSOR1           0         // EC sensor 1
#define EC_SENSOR2           1         // EC sensor 2
#define EC_SENSOR3           2         // EC sensor 3

//#define EC_Rf              500        // I/U converter feedback resistor
#define EC_ALFA          (0.02)        // temperature correction coeff [%/oC]

// analog multiplexer
#define EC_MUX_CH0           0
#define EC_MUX_CH1           1
#define EC_MUX_CH2           2
#define EC_MUX_HIZ           3

// calibration state machine
#define EC_CAL_IDLE          0
#define EC_CAL_START         1
#define EC_CAL_INSERT        2
#define EC_CAL_BUSY          3
#define EC_CAL_CLEAN         4
#define EC_CAL_READY        99

// -- prototypes
void init_ec (void);

void ec_update (void);

void ec_get (void);

void     ec_sjw (uint8_t channel);
uint16_t ec_peak_detect (void);
uint16_t ec_filter (uint8_t channel, uint16_t dat);

void ec_cal_sensor (uint8_t probe);
void ec_calibrate (void);

void ec_select (uint8_t channel);
void ec_dump     (void);
void ec_dump_tab (void);

void ec_log_all   (void);
void ec_log_start (void);
void ec_log_stop  (void);

void ec_state_update (void);

#endif




