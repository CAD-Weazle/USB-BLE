#ifndef stm_h
#define stm_h

#include "stm32f411xe.h"

/*
 * Flash defines
 */

#define FLASH_KEY1 0x45670123
#define FLASH_KEY2 0xcdef89ab

/* 
 * USB defines
 */

/*
 * GPIO defines
 */
enum
{ 
    GPIO_Mode_IN   = 0x00, /*!< GPIO Input Mode */
    GPIO_Mode_OUT  = 0x01, /*!< GPIO Output Mode */
    GPIO_Mode_AF   = 0x02, /*!< GPIO Alternate function Mode */
    GPIO_Mode_AN   = 0x03  /*!< GPIO Analog Mode */
};

enum
{ 
    GPIO_OType_PP = 0x00,
    GPIO_OType_OD = 0x01
};

enum
{ 
    GPIO_Speed_2MHz   = 0x00, /*!< Low speed */
    GPIO_Speed_25MHz  = 0x01, /*!< Medium speed */
    GPIO_Speed_50MHz  = 0x02, /*!< Fast speed */
    GPIO_Speed_100MHz = 0x03  /*!< High speed on 30 pF (80 MHz Output max speed on 15 pF) */
};

enum
{ 
    GPIO_PuPd_NOPULL = 0x00,
    GPIO_PuPd_UP     = 0x01,
    GPIO_PuPd_DOWN   = 0x02
};

enum
{
    GPIO_AF_MCO = 0,
    GPIO_AF_SWJ = 0,
    GPIO_AF_USART2 = 7,
    GPIO_AF_OTG = 10,
};

#endif
