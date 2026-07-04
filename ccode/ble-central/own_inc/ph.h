// file    : ph.h
// author  : rb
// purpose : header file for ph.c
// date    : 161111
// last    : 191210
//

#ifndef __PH_H__
#define __PH_H__

// -- defines
#define PH_NUM_DEVICES       3        // number of pH sensors

#define PH_FILTERDEPTH      64         // moving average filter depth
 
#define PH_CAL_WAIT       1200         // waiting time during calibration [s/10]
#define PH_CAL_RINSE       300         // cleaning time between calibrations [s/10] 
#define PH_CAL_ERR_DLY      30         // display time error window [s/10]
#define PH_CAL_OK_DLY       20         // display time ok window [s/10]

#define PH_SENSOR1           0         // pH sensor 1
#define PH_SENSOR2           1         // pH sensor 2
#define PH_SENSOR3           2         // pH sensor 3

#define PH4_CALPOINT         4         // pH 4.0, for pH calculation
#define PH7_CALPOINT         7         // pH 7.0, for pH calculation
#define PH_CALSPAN    (PH7_CALPOINT - PH4_CALPOINT)    // pH span between cal. points, for pH calculation

#define PH4_NORMAL_ADC    1820         // ballpark calibration value pH 4.0
#define PH7_NORMAL_ADC    2050         // ballpark calibration value pH 7.0

#define PH_MAXERR          50         // allowed deviation from default calibration value 
                                      // during calibration (appox. 0.5 pH unit)

// LED signaling constants
#define START_BLINKS         5         // # LED blinks at start
#define READY_BLINKS         5         // # LED blinks when ready
#define ERROR_BLINKS        10         // # LED blinks after error

// calibration state machine
#define PH_CAL_IDLE          0
#define PH_CAL_START         1
#define PH_CAL_4             2
#define PH_CAL_4_WAIT        3
#define PH_CAL_CLEAN         4
#define PH_CAL_7             5
#define PH_CAL_7_WAIT        6
#define PH_CAL_ERR           7
#define PH_CAL_OK            8
#define PH_CAL_DONE          9

// -- globals
extern uint8_t  ph_raw;
extern uint16_t ph_cur[PH_NUM_DEVICES];

extern uint16_t ph4_adc[PH_NUM_DEVICES];
extern uint16_t ph7_adc[PH_NUM_DEVICES];


// -- prototypes
void init_ph (void);

void ph_update (void);
void ph_get    (void);

void ph_dump     (void);
void ph_dump_raw (void);

void ph_cal_sensor (uint8_t probe);
void ph_calibrate  (void);

void ph_log_all   ();
void ph_log_start (void);
void ph_log_stop  (void);

#endif



