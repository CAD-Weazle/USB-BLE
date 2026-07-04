// file    : 25aa040.h
// author  : rb
// purpose : header file for 25aa040.c
// date    : 190730
// last    : 191210
//

#ifndef _25AA040_H_
#define _25AA040_H_

// -- defines
#define EE_SIZE                     512     // EEPROM size [byte]

// EEPROM memory map
#define EE_BUSMAP_BASE                                0x00  // busmap base address
#define EE_BUSMAP_SIZE                      SW_NUM_DEVICES  // busmap size [byte]
#define PH_CALDAT_BASE  (EE_BUSMAP_BASE + EE_BUSMAP_SIZE)   // pH calibration data base address
#define PH_CALDAT_SIZE  (PH_NUM_DEVICES*2*sizeof(uint16_t)) // pH calibration data size [bytes] (two points, stored as words)
#define EC_CALDAT_BASE  (PH_CALDAT_BASE + PH_CALDAT_SIZE)   // EC calibration data base address
#define EC_CALDAT_SIZE  (EC_NUM_DEVICES*sizeof(float))      // EC calibration data size [bytes] (single factor, stored as float)
#define NTC_CALDAT_BASE (EC_CALDAT_BASE + EC_CALDAT_SIZE)   // NTC calibration data base address
#define NTC_CALDAT_SIZE (NTC_NUM_DEVICES*sizeof(uint16_t))  // NTC calibration data size [bytes] (single factor, stored as word)

#define EE_TS_BASE                                    0x80  // EEPROM address storage 'ts_state' struct

// commands
#define EE_CMD_READ          0b00000011          // read data from memory
#define EE_CMD_WRITE         0b00000010          // write data to memory 
#define EE_CMD_WRDI          0b00000100          // reset write eneble latch - disable write 
#define EE_CMD_WREN          0b00000110          // set write enable latch   - enable write
#define EE_CMD_RDSR          0b00000101          // read STATUS register
#define EE_CMD_WRSR          0b00000001          // write STATUS register 

#define EE_MSB               0b00001000          // bit 8 of read/write address

// status register
#define EE_STATUS_WIP        0b00000001          // Write In Progess bit
#define EE_STATUS_WEL        0b00000010          // Write Enable Latch bit
#define EE_STATUS_BP0        0b00000100          // Block Protection 0 bit
#define EE_STATUS_BP1        0b00001000          // Block Protection 1 bit

// -- prototypes
void init_eeprom (void);

void ee_read_buf  (uint8_t *buf, uint16_t len, uint16_t adr);
void ee_write_buf (uint8_t *buf, uint16_t len, uint16_t adr);

uint8_t  ee_read_byte  (uint16_t adr);
uint16_t ee_read_word  (uint16_t adr);
uint32_t ee_read_long  (uint16_t adr);
float    ee_read_float (uint16_t adr);

void ee_write_byte  (uint16_t adr, uint8_t dat);
void ee_write_word  (uint16_t adr, uint16_t dat);
void ee_write_long  (uint16_t adr, uint32_t dat);
void ee_write_float (uint16_t adr, float dat);

uint8_t ee_busy (void);

void ee_write_enable  (void);
void ee_write_disable (void);

void ee_wipe (void);
void ee_dump (void);

void    ee_read_busmap (void);
void    ee_dump_busmap (void);
void    ee_fill_busmap (uint8_t num, uint8_t id);
uint8_t ee_get_busmap  (uint8_t num, uint8_t id);

void     ee_init_phcal (void);
void     ee_set_phcal  (uint8_t sensor, uint8_t cal_point, uint16_t cal_dat);
uint16_t ee_get_phcal  (uint8_t sensor, uint8_t cal_point);
void     ee_dump_phcal (void);

void  ee_init_eccal (void);
void  ee_set_eccal  (uint8_t sensor, float cal_dat);
float ee_get_eccal  (uint8_t sensor);
void  ee_dump_eccal (void);

void    ee_init_ntccal (void);
void    ee_set_ntccal (uint8_t sensor, int16_t cal_dat);
int16_t ee_get_ntccal (uint8_t sensor);
void    ee_dump_ntccal (void);

void ee_tft_init  (void);
void ee_tft_read  (void);
void ee_tft_write (void);
void ee_tft_dump  (void);

#endif


