// file    : main.c
// author  : rb
// purpose : SunWeb Master + ME813A-WH50C TFT module + BLE test code 
// board   : SunWeb MASTER v1.1 - STM32F407VE + BLE patched
// date    : 180609
// last    : 191219
//

#include "includes.h"

// main
int main (void)
{
  // enable FPU access, set CP10 and CP11 Full Access
  SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));  

  // init all
  init_timer  ();
  init_ports  ();
  init_pll    ();
  init_uart1  (115200);  // RS-232 
  init_uart4  (9600);    // SunWeb - original
//init_uart4  (115200);  // BLE module - test
  init_spi    ();
  init_i2c    ();
  init_adc    ();
  init_dac    ();
  init_eeprom ();        // call before pH/EC/NTC/TFT
  init_ec     ();
  init_ph     ();
  init_ntc    ();
  init_ds3234 ();
  init_scd30  ();
  init_pump   ();
  init_eve    ();
  init_tft    ();        
  init_sunweb ();        // call after 'init_tft'
//init_ble    ();
 
  // first prompt & go...
  prompt  ();
  version (); 

  while (1)
  {
    // poll CLI & SunWeb
    poll_cli ();         // poll Command Line Interface
    poll_bus ();         // poll SunWeb Bus

    if (csec_elapsed)
    {    
      // update TFT screens
      tft_update ();

      csec_elapsed = 0;
    }

    if (dsec_elapsed)
    {
      // get state SunWeb Master and SunWeb Devices & update sensors, pumps and time
      monitor_update ();

      dsec_elapsed = 0;
    }

    if (sec_elapsed)
    {
      led_red_flash (1);

      // dump state SunWeb Master & SunWeb Devices
      monitor_dump ();

      sec_elapsed = 0;
    }
  }

  return 0;
}
