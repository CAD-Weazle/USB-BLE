// file    : main.c
// author  : rb
// purpose : Feasycom BLE Module with USB
// board   : BLE-USB v1.0
// date    : 200628
// last    : 220907
//

#include "includes.h"

int main (void)
{
  init_pll   ();
  init_ports ();
  init_timer ();
  init_uart1 (115200);   // debug
  init_uart2 (115200);   // BLE module
  init_usb   ();
   
  msleep (1000);

  init_ble ();

  while (1)
  {
    poll_cli ();
    poll_usb ();
    poll_ble ();

    if (csec_elapsed)
    {
      usb_core ();
    
      csec_elapsed = 0;
    }

    if (sec_elapsed)
    {
      led_red_flash (5);

      ble_do ();

      sec_elapsed = 0;
    }
  }
}


