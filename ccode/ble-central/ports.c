// file    : ports.c
// author  : ao/rb
// purpose : IO port routines - STM32F4xx
// date    : 170810
// last    : 200516
//
// note    : make sure GPIO is in mode GPIO_SPEED_FREQ_HIGH for fast signals
//           (IO in slow mode will block fast signals!1!)

#include "includes.h"

static const struct port_info
{
  uint16_t mode : 2;
  uint16_t speed: 2;
  uint16_t od   : 1;
  uint16_t pupd : 2;
  uint16_t afr  : 4;
} porta[16] = 
{
   [0] = {GPIO_Mode_AF,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_UART4},    // UART4 - TXD_485
   [1] = {GPIO_Mode_AF,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_UART4},    // UART4 - RXD_485
   [2] = {GPIO_Mode_OUT, GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // DIR_485
   [3] = {GPIO_Mode_AN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // EC1  - ADC[3]
   [4] = {GPIO_Mode_AN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // NTC1 - ADC[4]
   [5] = {GPIO_Mode_AN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // EC_DAC
   [6] = {GPIO_Mode_AN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // PH1  - ADC[6]
   [7] = {GPIO_Mode_AN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // EC0  - ADC[7]
   [8] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
   [9] = {GPIO_Mode_AF,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_UP,     GPIO_AF_USART1},   // UART1 Tx 
  [10] = {GPIO_Mode_AF,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_UP,     GPIO_AF_USART1},   // UART1 Rx 
  [11] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC 
  [12] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC 
  [13] = {GPIO_Mode_AF,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_SWJ},      // SWIO
  [14] = {GPIO_Mode_AF,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_SWJ},      // SWCLK 
  [15] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
};

static const struct port_info portb[16] = 
{
   [0] = {GPIO_Mode_AN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // PRESSURE  - ADC[8]
   [1] = {GPIO_Mode_AN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // LIGHT_OUT - ADC[9]
   [2] = {GPIO_Mode_OUT, GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // LIGHT_SD
   [3] = {GPIO_Mode_IN,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
   [4] = {GPIO_Mode_IN,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
   [5] = {GPIO_Mode_IN,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
   [6] = {GPIO_Mode_OUT, GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // LED_RED
   [7] = {GPIO_Mode_OUT, GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // LED_GRN
   [8] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // LEVEL2
   [9] = {GPIO_Mode_AF,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_DOWN,   GPIO_AF_TIM4},     // PUMP2 - Timer 4 PWM out OC4
  [10] = {GPIO_Mode_AF,  GPIO_Speed_100MHz, GPIO_OType_OD, GPIO_PuPd_NOPULL, GPIO_AF_I2C2},     // SCL0
  [11] = {GPIO_Mode_AF,  GPIO_Speed_100MHz, GPIO_OType_OD, GPIO_PuPd_NOPULL, GPIO_AF_I2C2},     // SDA0
  [12] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // CO2_RDY
  [13] = {GPIO_Mode_OUT, GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_UP},                       // EE_SEL_L
  [14] = {GPIO_Mode_IN,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
  [15] = {GPIO_Mode_IN,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
};

static const struct port_info portc[16] = 
{
   [0] = {GPIO_Mode_AN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // PAR_OUT - ADC[10]
   [1] = {GPIO_Mode_AN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // EC2     - ADC[11]
   [2] = {GPIO_Mode_AN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // NTC2    - ADC[12]
   [3] = {GPIO_Mode_AN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // PH2     - ADC[13]
   [4] = {GPIO_Mode_AN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // NTC0    - ADC[4] 
   [5] = {GPIO_Mode_AN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // PH9     - ADC[15]
   [6] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
   [7] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
   [8] = {GPIO_Mode_IN,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
   [9] = {GPIO_Mode_OUT, GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC -- RN4871 RESET_L, test only
  [10] = {GPIO_Mode_AF,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_SPI3},     // SCK3
  [11] = {GPIO_Mode_AF,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_DOWN,   GPIO_AF_SPI3},     // MISO3
  [12] = {GPIO_Mode_AF,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_SPI3},     // MOSI3
  [13] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
  [14] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC 
  [15] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
};

static const struct port_info portd[16] = 
{
   [0] = {GPIO_Mode_OUT, GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_UP},                       // TFT_SEL_L
   [1] = {GPIO_Mode_OUT, GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_UP},                       // TFT_PD_L
   [2] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_UP},                       // TFT_INT_L
   [3] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
   [4] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
   [5] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC  
   [6] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC  
   [7] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC  
   [8] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
   [9] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
  [10] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
  [11] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC  
  [12] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
  [13] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
  [14] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC  
  [15] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
};

static const struct port_info porte[16] = 
{
   [0] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // NC
   [1] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // LEVEL1
   [2] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // NC
   [3] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // NC
   [4] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // LEVEL0
   [5] = {GPIO_Mode_AF,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN, GPIO_AF_TIM9},       // PUMP1 - Timer 9 PWM out OC1
   [6] = {GPIO_Mode_AF,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN, GPIO_AF_TIM9},       // PUMP0 - Timer 9 PWM out OC2
   [7] = {GPIO_Mode_OUT, GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // REF_EC
   [8] = {GPIO_Mode_OUT, GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // REF_PH
   [9] = {GPIO_Mode_OUT, GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_UP},                       // RTC_SEl_L
  [10] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
  [11] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
  [12] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
  [13] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
  [14] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
  [15] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NC
};

static const struct port_info porth[2] = 
{
   [0] = {GPIO_Mode_OUT, GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // EC_SEL0
   [1] = {GPIO_Mode_OUT, GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // EC_SEL1
};

// initilaize GPIO port
static void init_port (GPIO_TypeDef *GPIOx, const struct port_info *port)
{
  int i;
  uint32_t mode = 0, speed = 0, type = 0, pupd = 0;
  uint64_t afr = 0;

  for (i = 0; i < 16; i++)
  {
    mode  |= (uint32_t) port[i].mode  << i*2;
    speed |= (uint32_t) port[i].speed << i*2;
    pupd  |= (uint32_t) port[i].pupd  << i*2;
    type  |= (uint32_t) port[i].od    << i;
    afr   |= (uint64_t) port[i].afr   << i*4;
  }

  GPIOx->AFR[0]  = afr;           // set AFR registers first
  GPIOx->AFR[1]  = afr >> 32;     // set AFR registers first
  GPIOx->MODER   = mode;
  GPIOx->OSPEEDR = speed;
  GPIOx->PUPDR   = pupd;
  GPIOx->OTYPER  = type;
}

// initialize all GPIO ports
void init_ports (void) 
{
  // enable clocks for GPIO ports
  RCC->AHB1ENR |= RCC_AHB1Periph_GPIOA 
               |  RCC_AHB1Periph_GPIOB 
               |  RCC_AHB1Periph_GPIOC 
               |  RCC_AHB1Periph_GPIOD 
               |  RCC_AHB1Periph_GPIOE
               |  RCC_AHB1Periph_GPIOH;

  // dummy read
  (void) RCC->AHB1ENR;

  // setup GPIO ports
  init_port (GPIOA, porta);
  init_port (GPIOB, portb);
  init_port (GPIOC, portc);
  init_port (GPIOD, portd);
  init_port (GPIOE, porte);
  init_port (GPIOH, porth);

  // <> BLE test only, deassert RESET_L
  GPIOC->SET = BLE_RESET_L;

  // ambient light sensor always enabled
  photo_sensor_on ();

  // rest all off
  led_red_off   ();
  led_grn_off   ();
  ec_power_off  ();
  ph_power_off  ();
  tft_power_off ();
  tft_unsel     ();
  ds3234_unsel  ();
}


// green LED routines
void led_grn_on (void)
{
  GPIOB->SET = LED_GRN;
}

void led_grn_off (void)
{
 GPIOB->CLR = LED_GRN; 
}

void led_grn_toggle (void)
{
  if (GPIOB->ODR & LED_GRN)
    GPIOB->CLR = LED_GRN;
  else
    GPIOB->SET = LED_GRN;
}

void led_grn_flash (uint16_t delay)
{
  GPIOB->SET = LED_GRN;
  msleep (delay);  
  GPIOB->CLR = LED_GRN;
}


// red LED routines
void led_red_on (void)
{
  GPIOB->SET = LED_RED;
}

void led_red_off (void)
{
 GPIOB->CLR = LED_RED; 
}

void led_red_toggle (void)
{
  if (GPIOB->ODR & LED_RED)
    GPIOB->CLR = LED_RED;
  else
    GPIOB->SET = LED_RED;
}

void led_red_flash (uint16_t delay)
{
  GPIOB->SET = LED_RED;
  msleep (delay);  
  GPIOB->CLR = LED_RED;
}

// RS-485 routines
void rs485_dir (uint16_t dir)
{
  if (dir)
    GPIOA->SET = DIR_485;     // RS-485 transmit
  else
    GPIOA->CLR = DIR_485;     // RS-485 receive
}

// select DS3234 RTC
void ds3234_sel (void)
{
  GPIOE->CLR = RTC_SEL_L;
}

// deselect DS3234 RTC
void ds3234_unsel (void)
{
  GPIOE->SET = RTC_SEL_L;
}


// EC circuit power on
void ec_power_on (void)
{
  GPIOE->SET = REF_EC;
}

// EC circuit power off
void ec_power_off (void)
{
  GPIOE->CLR = REF_EC;
}


// pH circuit power on
void ph_power_on (void)
{
  GPIOE->SET = REF_PH;
}

// pH circuit power off
void ph_power_off (void)
{
  GPIOE->CLR = REF_PH;
}


// TFT SPI power on
void tft_power_on (void)
{
  GPIOD->SET = TFT_PD_L;
}

// TFT SPI power off
void tft_power_off (void)
{
  GPIOD->CLR = TFT_PD_L;
}

// TFT SPI select
void tft_sel (void)
{
  GPIOD->CLR = TFT_SEL_L;
  usleep (5);  // <> needed?
}

// TFT SPI unselect
void tft_unsel (void)
{
  usleep (5);  // <> needed?  
  GPIOD->SET = TFT_SEL_L;
  usleep (5);  // <> needed?  
}

// EEPROM SPI select
void eeprom_sel (void)
{
  GPIOB->CLR = EE_SEL_L;
  usleep (5); // relax a bit
}

// EEPROM SPI unselect
void eeprom_unsel (void)
{
  usleep (5); // relax a bit
  GPIOB->SET = EE_SEL_L;
  usleep (5); // relax a bit
}

// light sensor enable
void photo_sensor_on (void)
{
  GPIOB->CLR = LIGHT_SD;
}

// light sensor disable
void photo_sensor_off (void)
{
  GPIOB->SET = LIGHT_SD;
}









