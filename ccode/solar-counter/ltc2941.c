// file    : ltc2941.c
// author  : rb
// purpose : I2C battery gas gauge
// date    : 200430
// last    : 200616
//
// note    : 1mAh = 3.6C
//
// note    : Qlsb = 0.085mAh * (50mR / Rsense) * M/128 ; Rsense = 1R
//                = 306mA * M/128
//
//             M     Qlsb [uA]   Qmax [C]  capacity [mAh] 
//           ---------------------------------------------
//             1       119.53      7.83       2.17
//             2       239.06     15.67       4.35
//             4       478.13     31.33       8.70
//             8       956.25     62.67      17.41
//            16      1912.50    125.34      34.81
//            32      3825.00    250.68      69.63
//            64      7650.00    501.35     139.26
//           128     15300.00   1002.70     278.53
//

#include "includes.h"

float charge  = 0.0;
float current = 0.0;

// init I2C 
void init_ltc2941 (void)
{
  // no Vbat Alert, Prescaler = 1, Charge Complete active, 
//ltc2941_wr_reg8 (LTC2941_REG_CONTROL, LTC2941_PRESCALE_1  | LTC2941_CHARGE_COMPLETE);  // too small
  ltc2941_wr_reg8 (LTC2941_REG_CONTROL, LTC2941_PRESCALE_32 | LTC2941_CHARGE_COMPLETE);  // testing now -- too small?
//ltc2941_wr_reg8 (LTC2941_REG_CONTROL, LTC2941_PRESCALE_64 | LTC2941_CHARGE_COMPLETE);  // test next

  // set charge counter half-way
  ltc2941_wr_reg16 (LTC2941_REG_CHARGE, 0x7fff);
}

// update charge state
void ltc2941_update (void)
{
  static float charge_old = 0;
  static uint8_t first = 1;
  uint16_t raw = 0;

  // read LTC2941 only when battery power ok
  if (adc_battery_ok ())
  {
    // calculate charge (see table above)
    raw = ltc2941_rd_reg16 (LTC2941_REG_CHARGE);

    // PRESCALE_1
    charge = (raw * 119.53) / (1000 * 1000);

    // PRESCALE_32
    charge = (raw * 3825.00) / (1000 * 1000);

    // PRESCALE_64
  //charge = (raw * 7650.00) / (1000 * 1000);
  }   
  else
  {
    // mark current zero & bail
    current = ltc2941_ifilter (0);

    // adjust current calculation when coming out of depleted state
    first = 1;

    return;
  }

  // set LTC2941 Charge Complete when battery fully charged
  if (adc_battery_full ())
  {
    // assert CC pin
    ltc2941_charge_complete ();

    // update charge 
    raw = ltc2941_rd_reg16 (LTC2941_REG_CHARGE);

    // PRESCALE_1
//  charge = (raw * 119.53) / (1000 * 1000);

    // PRESCALE_32
    charge = (raw * 3825.00) / (1000 * 1000);

    // PRESCALE_64
  //charge = (raw * 7650.00) / (1000 * 1000);

    // signal current is zero
    first = 1;
  }

  // calculate current, start smoothly
  if (first)
  {
    charge_old = charge;
    first = 0;
  }
   
  // calculate current [uA]
  current = 1000 * ltc2941_ifilter (charge - charge_old);
   
  // update state
  charge_old = charge;
}

// filter charge current data
float ltc2941_ifilter (float dat)
{
         float sum = 0;
  static float isamples[LTC2941_IFILTERDEPTH];

  // add charge data to filter  
  for (int i = LTC2941_IFILTERDEPTH-1; i > 0; i--)
    isamples[i] = isamples[i-1];
    
  isamples[0] = dat;   
  
  // calculate sum of all data points
  for (int i = 0; i < LTC2941_IFILTERDEPTH; i++)
    sum += isamples[i];

  // calculate average current [mA]
  return (1000*sum / LTC2941_IFILTERDEPTH);
}

// read byte from register
uint8_t ltc2941_rd_reg8 (uint8_t reg)
{
  return (i2c1_read_reg (LTC2941_I2C_ADR, reg));
}

// read word from register
uint16_t ltc2941_rd_reg16 (uint8_t reg)
{
  uint8_t hi, lo;

  hi = i2c1_read_reg (LTC2941_I2C_ADR, reg);
  lo = i2c1_read_reg (LTC2941_I2C_ADR, reg+1);

  return (((uint16_t)hi << 8) | lo);
}

// write byte to register
void ltc2941_wr_reg8 (uint8_t reg, uint8_t dat)
{
  i2c1_write_reg (LTC2941_I2C_ADR, reg, dat);
}

// write word to register
void ltc2941_wr_reg16 (uint8_t reg, uint16_t dat)
{
  i2c1_write_reg (LTC2941_I2C_ADR, reg+0, (uint8_t)((dat & 0xff00) >> 8));
  i2c1_write_reg (LTC2941_I2C_ADR, reg+1, (uint8_t) (dat & 0x00ff));
}

// pulse Charge Complete LTC2941
void ltc2941_charge_complete (void)
{
  // make sure Charge Complete active (needed after brown-out)
  ltc2941_wr_reg8 (LTC2941_REG_CONTROL, LTC2941_PRESCALE_32 | LTC2941_CHARGE_COMPLETE);
  msleep (10);

  // pulse CC pin
  gpio_set (GPIOB, CC);
  msleep (10);
  led_grn_flash (5);
  gpio_clr (GPIOB, CC);
}

// dump LTC4129 state
void ltc2941_dump (void)
{
  printf2 ("Q[C]: %2.6f I[uA]: %2.3f", charge, current); // PRESCALE_1/PRESCALE_32
//printf2 ("Q[C]: %2.3f I[mA]: %2.3f", charge, current); // PRESCALE_64
}

// dump LTC2941 registers
void ltc2941_reg_dump (void)
{
  printf2 ("LTC2941 Status Register    : 0x%02x\n", ltc2941_rd_reg8  (LTC2941_REG_STATUS));
  printf2 ("LTC2941 Control Register   : 0x%02x\n", ltc2941_rd_reg8  (LTC2941_REG_CONTROL));
  printf2 ("LTC2941 Charge Register    : 0x%04x\n", ltc2941_rd_reg16 (LTC2941_REG_CHARGE));
  printf2 ("LTC2941 Thres. Lo Register : 0x%04x\n", ltc2941_rd_reg16 (LTC2941_REG_THRESHOLD_HI));
  printf2 ("LTC2941 Thres. Hi Register : 0x%04x\n", ltc2941_rd_reg16 (LTC2941_REG_THRESHOLD_LO));
}





