// file    : monitor.h
// author  : rb
// purpose : header file for monitor.c
// date    : 190917
// last    : 191113
//

#ifndef _MONITOR_H_
#define _MONITOR_H_

#define EC_NUM_DEVICES       3  // already defined in 'ec.h'
#define PH_NUM_DEVICES       3  // already defined in 'ph.h'
#define NTC_NUM_DEVICES      3  // already defined in 'ntc.h'

// -- struct SunWeb MASTER state
typedef struct
{
  // time
  uint8_t  hrs;                 // time in normal hrs:min:sec format
  uint8_t  min;                 //
  uint8_t  sec;                 //
  uint32_t tsecs;               // time in 'raw' [s] format

  // EC
  uint16_t ec[EC_NUM_DEVICES];            
  uint8_t  ec_cal_busy;         // EC sensor calibration busy flag  - set by EC code
  uint8_t  ec_cal_start;        // EC sensor calibration start flag - set by TFT code
  uint8_t  ec_cal_stage;        // EC sensor calibration stage
  uint8_t  ec_cal_probe;        // sensor being calibrated
  uint16_t ec_cal_waittime;     // waiting time total (for progress bar)
  uint16_t ec_cal_progress;     // waiting time done  (for progress bar)

  // pH
  uint16_t ph[PH_NUM_DEVICES];  // actual pH values          
  uint8_t  ph_cal_busy;         // pH sensor calibration busy flag  - set by pH code
  uint8_t  ph_cal_start;        // pH sensor calibration start flag - set by TFT code
  uint8_t  ph_cal_stage;        // pH sensor calibration stage
  uint8_t  ph_cal_probe;        // sensor being calibrated
  uint16_t ph_cal_waittime;     // waiting time total (for progress bar)
  uint16_t ph_cal_progress;     // waiting time done  (for progress bar)

  // NTC (of EC sensors)
  uint16_t ntc[NTC_NUM_DEVICES];            

  // CO2/humidity/temperature
  float co2;
  float temp;
  float humi;

  // light/PAR/pressure
  uint16_t amb;
  uint16_t par;
  uint16_t prs;
} m_state;

extern m_state *ms;

void gaan_met_die_banaan (void);

// -- protoypes
void monitor_update (void);

void monitor_dump (void);
void master_dump  (void);

void master_log_start (void);
void master_log_stop  (void);

void master_log_interval (uint16_t dat);

#endif
