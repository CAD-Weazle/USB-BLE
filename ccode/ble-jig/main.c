// file    : main.c
// author  : rb
// purpose : solar charger & Bluetooth Low Energy test
// board   : BLE-JIG v1.0
// date    : 200225
// last    : 200421
//
// note    : current consumption during Stop mode & BLE Low Power:  ~79uA

#include "includes.h"

int main (void)  
{
  // init all the things!1!!
  init_clock  ();                                // turn on clocks
  init_ticker ();                                // start timers
  init_ports  ();                                // set GPIOs
  init_uart2  (9600);                            // init UART2 - CLI (Baudrate hardcoded due to low MCU clock)
  init_uart3  (9600);                            // init UART3 - BLE (Baudrate hardcoded due to low MCU clock)
  init_adc    ();                                // setup ADCs
  init_power  ();                                // set core voltage
  init_ble    ();                                // init BLE module
  init_sht15  ();                                // setup SHT15 sensor

  // dump version
  version ();

  while (1)
  {
    poll_cli ();                                 // poll commandline  - UART2
    poll_ble ();                                 // poll BLE response - UART3

    if (sec_elapsed)
    {
      // handle sensors & send measurements to BLE Central
      ble_do ();

      sec_elapsed = 0;
    }
  }
}
