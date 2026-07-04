// file    : sunweb-master.h
// author  : rb
// purpose : header file for sunweb-master.c
// date    : 190116
// last    : 191219
//

#ifndef _SUNWEB_MASTER_H_
#define _SUNWEB_MASTER_H_

// struct holding GroBox state
typedef struct
{
  // SimSun variable
  uint8_t  ss_id;                                     // SimSun Device ID
  uint8_t  ss_attc;                                   // SimSun Device attached
  uint8_t  ss_dred;                                   // LED intensity Deep Red LEDS  [% PWM]
  uint8_t  ss_fred;                                   // LED intensity Far Red LEDS   [% PWM]
  uint8_t  ss_rblu;                                   // LED intensity Royal Blu LEDS [% PWM]
  uint16_t ss_dred_flux;                              // LED intensity Deep Red LEDS  [umol/m^2*s]
  uint16_t ss_rblu_flux;                              // LED intensity Royal Blu LEDS [umol/m^2*s]
  uint16_t ss_tact;                                   // LED panel temperature

  // HotBed variables
  uint8_t  hb_id;                                     // HotBed Device ID
  uint8_t  hb_attc;                                   // HotBed Device attached
  uint32_t hb_tset;                                   // setpoint temperature
  uint32_t hb_tact;                                   // actual temperature
  uint32_t hb_tamb;                                   // ambient temperature
  uint8_t  hb_pid;                                    // temperature PID control active
  uint8_t  hb_fan;                                    // fan speed     [% PWM]
  uint8_t  hb_led;                                    // LED intensity [% PWM]
} gbox;

extern gbox *gb;

// -- defines
#define SW_NUM_DEVICES                            6   // max number SunWeb Devices connected
#define SW_NUM_GBOXES            (SW_NUM_DEVICES/2)   // max number GroBoxes

#define SW_MAXLEN                                16   // SunWeb maximum packet length
#define TX_BUFLEN                         SW_MAXLEN   // length SunWeb transmit buffer
#define RX_BUFLEN                         SW_MAXLEN   // length SunWeb receive buffer

#define SW_RECV                                   0   // RS-485 direction: receive
#define SW_SEND                                   1   // RS-485 direction: send

#define SW_HDRLEN                                 4   // SunWeb Packet header length (SYNC+ID+CMD+LEN)

#define SW_ID_BYTE                        rx_buf[1]   // Rx packet byte holding Device ID
#define SW_CMD_BYTE                       rx_buf[2]   // Rx packet byte holding command
#define SW_LEN_BYTE                       rx_buf[3]   // Rx packet byte holding packet length

// SunWeb Commands
#define SW_CMD_PROBE                           0xa0   // probe SunWeb Device
#define SW_CMD_ATTACH                          0xa1   // attach SunWeb Device
#define SW_CMD_RELEASE                         0xa2   // detach SunWeb Device

// SunWeb statemachine
#define SX_DO_RELEASE                             0
#define SX_DO_PROBE                               1
#define SX_DO_PROGRAM                             2
#define SX_DO_HANDLE                              3

// SunWeb Device probing statemachine
#define PROBE_SS                                  0
#define PROBE_HB                                  1
#define PROBE_HANDLE_SS                           2
#define PROBE_HANDLE_HB                           3
#define PROBE_NEXT                                4
#define PROBE_READY                               5

// Device logging state machine
#define SW_LOG_HBTEMP                             0
#define SW_LOG_HBSTATE                            1
#define SW_LOG_SSSTATE                            2

#define SW_LOG_DELAY                             12   // log data 5 times per minute

#define HB_LED_INTENSITY                         10   // HotBed bottom LED light intensity [%PWM]
#define HB_FAN_OFF                                0   // HotBed fan speed off [%PWM]
#define HB_FAN_MIN                               10   // HotBed fan speed min [%PWM]
#define HB_FAN_MID                               40   // HotBed fan speed mid [%PWM]
#define HB_FAN_MAX                               60   // HotBed fan speed max [%PWM]

// -- prototypes
void init_sunweb (void);

void sw_update (void);

void    sw_release_all  (void);
uint8_t sw_probe_all    (void);
void    sw_probe_handle (uint8_t id);
void    sw_program_all  (void);
void    sw_parse_cmd    (void);

void sw_log_all   (void);

void sw_start_probe (void);

void sw_init_gboxes (void);
void sw_dump_gboxes (void);

void sunweb_dump (void);

#endif

