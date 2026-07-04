// file    : ports.c
// author  : ao/rb
// purpose : STM32L1xx IO port routines
// date    : 170810
// last    : 200516
//

#include "includes.h"

// GPIO bit fields
static const struct port_info
{
  uint16_t mode : 2;
  uint16_t speed: 2;
  uint16_t od   : 1;
  uint16_t pupd : 2;
  uint16_t afr  : 4;
} port_info_type; 


// GPIO Port A settings
static const struct port_info porta[16] = 
{
  [ 0] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLUP},                  // INT_L
  [ 1] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_NOPULL},                  // VSOL      - ADC[1]
  [ 2] = {GPIO_MODE_AF,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_NOPULL, GPIO_AF7_USART2}, // SER_TXD
  [ 3] = {GPIO_MODE_AF , GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_NOPULL, GPIO_AF7_USART2}, // SER_RXD
  [ 4] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
  [ 5] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
  [ 6] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
  [ 7] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
  [ 8] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
  [ 9] = {GPIO_MODE_OUT, GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_NOPULL},                  // LED_RED
  [10] = {GPIO_MODE_OUT, GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_NOPULL},                  // LED_GRN
  [11] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_NOPULL},                  // NU (USB_DM)
  [12] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_NOPULL},                  // NU (USB_DP)
  [13] = {GPIO_MODE_AF,  GPIO_SPEED_FREQ_HIGH, GPIO_OTYPE_PP, GPIO_NOPULL, GPIO_AF0_SWJ},    // SWD_IO
  [14] = {GPIO_MODE_AF,  GPIO_SPEED_FREQ_HIGH, GPIO_OTYPE_PP, GPIO_NOPULL, GPIO_AF0_SWJ},    // SWD_CLK 
  [15] = {GPIO_MODE_OUT, GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_OD, GPIO_NOPULL},                  // GBAT (open drain)
};

// GPIO Port A settings - STOP mode
static const struct port_info stopa[16] = 
{
   [0] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // INT_L
   [1] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // VSOL      - ADC[1]
   [2] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // SER_TXD
   [3] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // SER_RXD
   [4] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // NU
   [5] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // NU
   [6] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // NU
   [7] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // NU
   [8] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // NU
   [9] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // LED_RED
  [10] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // LED_GRN
  [11] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // NU (USB_DM)
  [12] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // NU (USB_DP)
  [13] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // SWD_IO
  [14] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // SWD_CLK
  [15] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW, GPIO_OTYPE_PP, GPIO_PULLDOWN},                 // GBAT
};

// GPIO Port B settings
static const struct port_info portb[16] = 
{
  [ 0] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
  [ 1] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
  [ 2] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
  [ 3] = {GPIO_MODE_AF,  GPIO_SPEED_FREQ_HIGH, GPIO_OTYPE_PP, GPIO_NOPULL, GPIO_AF1_TIM2},   // BAT_SINK  - Timer 2 PWM out OC2
  [ 4] = {GPIO_MODE_OUT, GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_NOPULL},                  // SHDN_L
  [ 5] = {GPIO_MODE_OUT, GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // CC
  [ 6] = {GPIO_MODE_AF,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLUP, GPIO_AF4_I2C1},   // SCL       - I2C1
  [ 7] = {GPIO_MODE_AF,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_OD, GPIO_PULLUP, GPIO_AF4_I2C1},   // SDA       - I2C1
  [ 8] = {GPIO_MODE_OUT, GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_NOPULL},                  // CLR_L
  [ 9] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_NOPULL},                  // POL
  [10] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
  [11] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
  [12] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
  [13] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_NOPULL},                  // VBAT      - ADC[19]
  [14] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
  [15] = {GPIO_MODE_IN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
};

// GPIO Port B settings - STOP mode
static const struct port_info stopb[16] = 
{
   [0] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
   [1] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
   [2] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
   [3] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // BAT_SINK
   [4] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLUP},                  // SHDN_L
   [5] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // CC
   [6] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLUP},                  // SCL       - I2C1
   [7] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLUP},                  // SDA       - I2C1
   [8] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLUP},                  // CLR_L
   [9] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // POL
  [10] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
  [11] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
  [12] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NU
  [13] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_NOPULL},                  // VBAT      - ADC[19]
  [14] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_NOPULL},                  // NU
  [15] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_NOPULL},                  // NU
};

// GPIO Port C settings
static const struct port_info portc[16] = 
{
  [13] = {GPIO_MODE_OUT, GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_OD, GPIO_NOPULL},                  // GSOL (open drain)
  [14] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NC 
  [15] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NC
};


// GPIO Port C settings - STOP mode
static const struct port_info stopc[16] = 
{
  [13] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // GSOL
  [14] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NC 
  [15] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN},                // NC
};

// GPIO Port H settings - STOP mode
static const struct port_info stoph[16] = 
{
   [0] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN },               // NC
   [1] = {GPIO_MODE_AN,  GPIO_SPEED_FREQ_LOW,  GPIO_OTYPE_PP, GPIO_PULLDOWN },               // NC
};

// init GPIO Port
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

// initialize GPIO ports
void init_ports (void) 
{
  // enable clocks for GPIO ports
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN |
                 RCC_AHBENR_GPIOBEN |
                 RCC_AHBENR_GPIOCEN;

  // set-up GPIO ports
  init_port (GPIOA, porta);
  init_port (GPIOB, portb);
  init_port (GPIOC, portc);

  // all LEDs off
  led_grn_off ();
  led_red_off ();

  // sensor power down & VGND disconnected
  vbat_disable ();
  vsol_disable ();

  // enable & clear battery gas gauge
  ltc4150_enable ();
  ltc4150_clear  ();
}

// set GPIO to low power inputs in STOP mode
void ports_stop (void) 
{
  init_port (GPIOA, stopa);
  init_port (GPIOB, stopb);
  init_port (GPIOC, stopc);
  init_port (GPIOH, stoph);
}

// set GPIO normal operation
void ports_resume (void) 
{
  init_port (GPIOA, porta);
  init_port (GPIOB, portb);
  init_port (GPIOC, portb);
}

// red LED routines
void led_red_on (void)
{
  gpio_set (GPIOA, LED_RED);
}

void led_red_off (void)
{
  gpio_clr (GPIOA, LED_RED);
}

void led_red_toggle (void)
{
  if (GPIOA->ODR & (1 << LED_RED))
    gpio_clr (GPIOA, LED_RED);
  else
    gpio_set (GPIOA, LED_RED);
}

void led_red_flash (uint16_t delay)
{
  gpio_set (GPIOA, LED_RED);
  msleep (delay);
  gpio_clr (GPIOA, LED_RED);
}

// green LED routines
void led_grn_on (void)
{
  gpio_set (GPIOA, LED_GRN);
}

void led_grn_off (void)
{
  gpio_clr (GPIOA, LED_GRN);
}

void led_grn_toggle (void)
{
  if (GPIOA->ODR & (1 << LED_GRN))
    gpio_clr (GPIOA, LED_GRN);
  else
    gpio_set (GPIOA, LED_GRN);
}

void led_grn_flash (uint16_t delay)
{
  gpio_set (GPIOA, LED_GRN);
  msleep (delay);
  gpio_clr (GPIOA, LED_GRN);
}

// connect dividers for Vbat to GND
void vbat_enable (void)
{
  gpio_clr (GPIOA, GBAT);

  // let analog level settle
  msleep (5);
}

// disconnect dividers for Vbat from GND
void vbat_disable (void)
{
  gpio_set (GPIOA, GBAT);
}

// connect divider for Vsun ground to GND
void vsol_enable (void)
{
  gpio_clr (GPIOC, GSOL);

  // let analog level settle
  msleep (5);
}

// disconnect dividers for Vsun from GND
void vsol_disable (void)
{
  gpio_set (GPIOC, GSOL);
}
















