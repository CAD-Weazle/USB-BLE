// file    : main.c
// author  : rb
// purpose : Feasycom BLE Module test
// board   : BLE-JIG v1.0 with Feasycom module patched
// date    : 200507
// last    : 220222
//
// note    : randomly hangs when UART2 is enabled / CLI is polled
//

#include "includes.h"

int main (void)  
{
  // init all the things!1!!
  init_clock  ();                                // turn on clocks
  init_ticker ();                                // start timers
  init_ports  ();                                // setup GPIOs
//init_uart2  (115200);                          // setup UART2 - CLI, only needed for debugging
  init_uart3  (115200);                          // setup UART3 - BLE
  init_adc    ();                                // setup ADCs
  init_power  ();                                // setup core voltage
  init_sht15  ();                                // setup SHT15 sensor
  init_ble    ();                                // setup BLE module

  while (1)
  {
  //poll_cli ();                                 // poll commandline, only needed for debugging
  //poll_ble ();                                 // poll BLE module, only needed for initialization 

    if (sec_elapsed)
    {
      // handle BLE module 
      ble_do ();

      sec_elapsed = 0;
    }
  }
}
