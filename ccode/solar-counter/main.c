// file    : main.c
// author  : rb
// purpose : solar cell battery charger & battery gas gauge
// board   : SOLAR COUNTER v1.0 - STM32L151
// date    : 200421
// last    : 200616
//

#include "includes.h"

int main (void)  
{
  // init all the things!1!!
  init_clock   ();                               // turn on clocks
  init_ticker  ();                               // start timers
  init_ports   ();                               // set GPIOs
  init_uart2   (115200);                         // init UART2 - CLI
  init_adc     ();                               // setup ADCs
  init_i2c     ();                               // setup I2C
  init_power   ();                               // set core voltage
//init_ltc4150 ();                               // setup LTC4150 battery gas gauge
  init_ltc2941 ();                               // setup LTC2941 battery gas gauge

  // dump version
  version ();

  while (1)
  {
    poll_cli ();                                 // poll commandline

    if (dsec_elapsed)
    {
      adc_update ();
    
      dsec_elapsed = 0;
    }

    if (sec_elapsed)
    {
      led_red_flash (5);

      ltc2941_update ();

      adc_dump     ();
      ltc2941_dump ();
      printf2 ("\n");
 
      sec_elapsed = 0;
    }
  }
}
