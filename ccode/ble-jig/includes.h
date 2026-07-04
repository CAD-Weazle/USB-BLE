//#include <stdlib.h>
//#include "stm32l1xx_hal_rcc.h"
//#include "stm32l1xx_hal_gpio_ex.h"
//#include "stm32l1xx_hal_uart.h"

#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "stm32l1xx.h"
#include "stm32l1xx_hal_gpio.h"

#include "version.h"
#include "clock.h"
#include "ticker.h"
#include "ports.h"
#include "power.h"
#include "uart2.h"
#include "uart3.h"
#include "printf2.h"
#include "printf3.h"
#include "dump.h"
#include "cli.h"
#include "dma.h"
#include "adc.h"
#include "ble.h"
#include "sht15.h"
