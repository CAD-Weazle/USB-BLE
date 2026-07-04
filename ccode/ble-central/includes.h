#ifndef __INCLUDES__
#define __INCLUDES__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"    // def's missing in 'stm32f4xx.h' ?
#include "stm32f4xx_gpio.h"   // def's missing in 'stm32f4xx.h' ?
#include "core/core_cm4.h"

#include "monitor.h"
#include "sunweb-master.h"
#include "tft.h"

#include "pll.h"
#include "ports.h"
#include "timer.h"
#include "uart1.h"
#include "uart4.h"
#include "spi.h"
#include "i2c.h"
#include "adc.h"
#include "dac.h"

#include "cli.h"
#include "ble.h"
#include "printf.h"
#include "printf4.h"
#include "dump.h"
#include "version.h"

#include "ec.h"
#include "ntc.h"
#include "ph.h"
#include "pump.h"
#include "ds3234.h"
#include "25aa040.h"
#include "scd30.h"
#include "eve.h"
#include "tft-monitor.h"
#include "tft-simsun.h"
#include "tft-hotbed.h"

#include "bus-handler.h"
#include "commands-master.h"
#include "commands-hotbed.h"
#include "commands-simsun.h"

#endif
