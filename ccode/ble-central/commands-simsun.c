// file    : commands-simsun.c
// author  : rb
// purpose : SunWeb SimSun commands
// date    : 190114
// last    : 191225
// note    : target DLI = 20 mol/day, with 18 hours illumination & 6 hours light off
//           light flux is then ~300 [umol/m^2.s]
//
// note    : Royal Blue LEDs can not output more than ~120 [umol/m^2*s]

#include "includes.h"

#define SS_DEBUG_CMDS                  0    // debug command processing

/*
# DRED Light Intensity: PAR [umol/m^2*s] vs PWM[%]
-2.625 0
27.133 5
40.481 10
61.706 15
91.280 20
112.910 25
135.229 30
164.770 35
186.870 40
209.409 45
239.606 50
262.144 55
284.901 60
307.877 65
338.512 70
361.706 75
384.682 80
416.192 85
440.262 90
464.332 95
498.468 100

# RBLU Light Intensity: PAR [umol/m^2*s] vs PWM[%] (from table above, divided by 4.57)

*/

// photon flux / PWM[%] relation in GroBox with reflective isolation foil
float dred_curve[5] = 
{
   0.18741177091865941481,
   0.23761161689622966865,
  -0.00022238353960730130,
   0.00000060262255285021,
  -0.00000000061261936510,
};

float rblu_curve[5] = 
{
  3.99493037248811272237,
  0.84289679763930001149,
 -0.00442450063505777143,
  0.00005450590163539598,
 -0.00000020276071193124
};

// convert Deep Red light flux [umol/m^2*s] to PWM [%]
uint8_t dred_to_pwm (float flux)
{
  float tmp = polynomial (dred_curve, 5, flux);

  return ((uint8_t)(tmp + 0.5));
}

// convert Royal Blue light flux [umol/m^2*s] to PWM [%]
uint8_t rblu_to_pwm (float flux)
{
  // Royal Blu LEDs can no temit more than ~120 [umol/m^2*s]
  if (flux > 120)
    flux = 120;
  
  float tmp = polynomial (rblu_curve, 5, flux);

  return ((uint8_t)(tmp + 0.5));
}

// evaluate polynomial function
float polynomial (float *p, uint8_t n, float x)
{
  float y = p[--n];

  while (n > 0)
    y = y * x + p[--n];

  return (y);
}

// set SunWeb Device dawn time
void ss_set_dawn (uint8_t badr, uint32_t time)
{
  uint8_t buf[3];
  uint8_t hrs, min, sec;
  
  // convert time format
  hrs = time / (60*60);
  min = (time - hrs*60*60) / 60;
  sec = 0;  // 15 minute intervals only, so always zero

  // pack buffer
  buf[0] = hrs;
  buf[1] = min;
  buf[2] = sec;

  // send SunWeb packet 
  sw_txpack (badr, SW_CMD_SETDAWN, 3, buf);
  sw_txsend ();

  debug (SS_DEBUG_CMDS, "> SETDAWN      SimSun 0x%02x\n", badr);
}

// set SunWeb Device dusk time
void ss_set_dusk (uint8_t badr, uint32_t time)
{
  uint8_t buf[3];

  uint8_t hrs, min, sec;
  
  // convert time format
  hrs = time / (60*60);
  min = (time - hrs*60*60) / 60;
  sec = 0;  // 15 minute intervals only, so always zero

  // pack buffer
  buf[0] = hrs;
  buf[1] = min;
  buf[2] = sec;

  // send SunWeb packet
  sw_txpack (badr, SW_CMD_SETDUSK, 3, buf);
  sw_txsend ();

  debug (SS_DEBUG_CMDS, "> SETDUSK      SimSun 0x%02x\n", badr);
}

// set light intensity Deep Red LEDs [%PWM]
void ss_set_dred (uint8_t badr, uint8_t pwm)
{
  sw_txpack (badr, SW_CMD_SETDRED, 1, &pwm);
  sw_txsend ();

  debug (SS_DEBUG_CMDS, "> SETDRED      SimSun 0x%02x\n", badr);
}

// set light intensity Royal Blue LEDs [%PWM]
void ss_set_rblu (uint8_t badr, uint8_t pwm)
{
  sw_txpack (badr, SW_CMD_SETRBLU, 1, &pwm);
  sw_txsend ();

  debug (SS_DEBUG_CMDS, "> SETRBLU      SimSun 0x%02x\n", badr);
}

// set light intensity Far Red LEDs [on/off]
void ss_set_fred (uint8_t badr, uint8_t pwm)
{
  sw_txpack (badr, SW_CMD_SETFRED, 1, &pwm);
  sw_txsend ();

  debug (SS_DEBUG_CMDS, "> SETFRED      SimSun 0x%02x\n", badr);
}

// set light intensity Deep Red LEDs [umol/m^2*s]
void ss_mol_dred (uint8_t badr, uint16_t flux) 
{
  uint8_t pwm = dred_to_pwm ((float)flux);

  sw_txpack (badr, SW_CMD_SETDRED, 1, &pwm);
  sw_txsend ();

  debug (SS_DEBUG_CMDS, "> MOLDRED      SimSun 0x%02x: %d\n", badr, pwm);
}

// set light intensity Royal Blue LEDs [umol/m^2*s]
void ss_mol_rblu (uint8_t badr, uint16_t flux)
{
  uint8_t pwm = rblu_to_pwm ((float)flux);

  sw_txpack (badr, SW_CMD_SETRBLU, 1, &pwm);
  sw_txsend ();

  debug (SS_DEBUG_CMDS, "> MOLRBLU      SimSun 0x%02x: %d\n", badr, pwm);
}

// set maximum light intensity Deep Red LEDs [umol/m^2*s]
void ss_max_dred (uint8_t badr, uint16_t flux)
{
  uint8_t pwm = dred_to_pwm ((float)flux);

  sw_txpack (badr, SW_CMD_SETDREDMAX, 1, &pwm);
  sw_txsend ();

  debug (SS_DEBUG_CMDS, "> SETDREDMAX   SimSun 0x%02x: %d\n", badr, pwm);
}

// set maximum light intensity Royal Blue LEDs [umol/m^2*s
void ss_max_rblu (uint8_t badr, uint16_t flux)
{
  uint8_t pwm = rblu_to_pwm ((float)flux);

  sw_txpack (badr, SW_CMD_SETRBLUMAX, 1, &pwm);
  sw_txsend ();

  debug (SS_DEBUG_CMDS, "> SETRBLUMAX   SimSun 0x%02x: %d\n", badr, pwm);
}

// start Device SimSun program 
void ss_simsun_start (uint8_t badr)
{
  sw_txpack (badr, SW_CMD_SIMSUNSTART, 0, NULL);
  sw_txsend ();

  debug (SS_DEBUG_CMDS, "> SIMSUNSTART  SimSun 0x%02x\n", badr);
}

// stop Device SimSun program 
void ss_simsun_stop (uint8_t badr)
{
  sw_txpack (badr, SW_CMD_SIMSUNSTOP, 0, NULL);
  sw_txsend ();

  debug (SS_DEBUG_CMDS, "> SIMSUNSTOP   SimSun 0x%02x\n", badr);
}

// get SimSun temperature & LED intensity
void ss_get_state (uint8_t badr)
{
  sw_txpack (badr, SS_CMD_GETSTATE, 0, NULL);
  sw_txsend ();

  debug (SS_DEBUG_CMDS, "> GETSTATE     SimSun 0x%02x\n", badr);
}

// send NTC offset parameter
void ss_set_ntc (uint8_t badr, int16_t err)
{
  uint8_t buf[2];

  buf[0] = (int8_t)((err & 0xff00) >> 8);
  buf[1] = (int8_t)(err & 0x00ff);

  sw_txpack (badr, SS_CMD_NTCERR, 2, buf);
  sw_txsend ();

  printf ("NTCERR      Tx - SimSun 0x%02x, %d\n", badr, err);
}



