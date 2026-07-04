// file    : ports.c
// author  : ao/rb
// purpose : IO port routines - STM32F4xx
// date    : 170810
// last    : 220214
//
// note    : make sure GPIO is in mode GPIO_SPEED_FREQ_HIGH for fast signals
//           (IO in slow mode will block fast signals!1!)

#include "includes.h"

struct port_info
{
  uint16_t mode : 2;
  uint16_t speed: 2;
  uint16_t od   : 1;
  uint16_t pupd : 2;
  uint16_t afr  : 4;
};

static const struct port_info porta[16] = 
{
  [ 0] = {GPIO_Mode_IN,  GPIO_Speed_25MHz,  GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // BLE_SWDIO <>
  [ 1] = {GPIO_Mode_IN,  GPIO_Speed_25MHz,  GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // BLE_SWCLK <>
  [ 2] = {GPIO_Mode_AF,  GPIO_Speed_25MHz,  GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF7_USART2},  // BLE_RXD 
  [ 3] = {GPIO_Mode_AF,  GPIO_Speed_25MHz,  GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF7_USART2},  // BLE_TXD
  [ 4] = {GPIO_Mode_OUT, GPIO_Speed_25MHz,  GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // BLE_TRAN
  [ 5] = {GPIO_Mode_OUT, GPIO_Speed_25MHz,  GPIO_OType_OD, GPIO_PuPd_DOWN},                     // BLE_RESET_L
  [ 6] = {GPIO_Mode_IN,  GPIO_Speed_25MHz,  GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NU
  [ 7] = {GPIO_Mode_IN,  GPIO_Speed_25MHz,  GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NU
  [ 8] = {GPIO_Mode_AF,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_MCO},      // MCO1
  [ 9] = {GPIO_Mode_AF,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF7_USART1},  // SER_TXD
  [10] = {GPIO_Mode_AF,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF7_USART1},  // SER_RXD
  [11] = {GPIO_Mode_AF,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_OTG},      // USB-
  [12] = {GPIO_Mode_AF,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_OTG},      // USB+
  [13] = {GPIO_Mode_AF,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_SWJ},      // SWIO
  [14] = {GPIO_Mode_AF,  GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_SWJ},      // SWCLK 
};

static const struct port_info portb[16] = 
{
   [0] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NU
   [1] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NU
   [3] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NU
   [4] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NU
   [5] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NU
   [6] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NU
   [7] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NU
   [8] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // BLE_LED
   [9] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_NOPULL},                   // BLE_STATUS
  [10] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NU
  [11] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NU
  [12] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NU
  [13] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NU
  [14] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NU  
  [15] = {GPIO_Mode_IN,  GPIO_Speed_2MHz,   GPIO_OType_PP, GPIO_PuPd_DOWN},                     // NU 
};

static const struct port_info portc[16] = 
{
  [13] = {GPIO_Mode_IN,  GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},                       // NU
  [14] = {GPIO_Mode_OUT, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL},                     // LED_RED
  [15] = {GPIO_Mode_IN,  GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},                       // NU
};

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

void init_ports (void) 
{
  // enable clocks for GPIOA/B/C
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | 
                  RCC_AHB1ENR_GPIOBEN | 
                  RCC_AHB1ENR_GPIOCEN;

  // enable compensation cell - note needed, slow signals?
//SYSCFG->CMPCR |= SYSCFG_CMPCR_CMP_PD;

  init_port (GPIOA, porta);
  init_port (GPIOB, portb);
  init_port (GPIOC, portc);
}

// red LED routines
void led_red_on (void)
{
  gpio_set (GPIOC, LED_RED);
}

void led_red_off (void)
{
  gpio_clr (GPIOC, LED_RED);
}

void led_red_toggle (void)
{
  if (GPIOC->ODR & (1 << LED_RED))
    gpio_clr (GPIOC, LED_RED);
  else
    gpio_set (GPIOC, LED_RED);
}

void led_red_flash (uint16_t delay)
{
  gpio_set (GPIOC, LED_RED);
  msleep (delay);
  gpio_clr (GPIOC, LED_RED);
}








