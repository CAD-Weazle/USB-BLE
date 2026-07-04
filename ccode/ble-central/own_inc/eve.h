// file    : eve.h
// author  : rb
// purpose : header file for eve.c
// date    : 170620
// last    : 191213
//

#ifndef _EVE_H_
#define _EVE_H_

#define EVE_BLIGHT_OFF                       0x00 // backlight off
#define EVE_BLIGHT_ON                        0x80 // backlight full wack

#define EVE_MEM_WR                           0x80 // EVE memory write 
#define EVE_MEM_RD                           0x00 // EVE memory read
#define EVE_HST_CMD                          0x40 // EVE host command

// defines for ME813A-WH50C 800x480 TFT display
#define EVE_HCYCLE                          928UL
#define EVE_HOFFSET                          88UL
#define EVE_HSIZE                           800UL
#define EVE_HSYNC0                            0UL
#define EVE_HSYNC1                           48UL
#define EVE_VCYCLE                          525UL
#define EVE_VOFFSET                          32UL
#define EVE_VSIZE                           480UL
#define EVE_VSYNC0                            0UL
#define EVE_VSYNC1                            3UL
#define EVE_SWIZZLE                           0UL
#define EVE_CSPREAD                           0UL
#define EVE_PCLKPOL                           1UL
#define EVE_PCLK                              5UL
#define EVE_TOUCH_RZTHRESH                 1200UL  // not used, resistive touch only

// EVE Host Commands
#define EVE_ACTIVE                           0x00  // FT813 active
#define EVE_STANDBY                          0x41  // FT813 standby mode (clock running)
#define EVE_SLEEP                            0x42  // FT813 sleep mode (clock off)
#define EVE_CLKEXT                           0x44  // select external clock source 
#define EVE_CLKINT                           0x48  // select internal clock source 
#define EVE_PWRDOWN                          0x50  // FT813 power down mode (core off) 
#define EVE_CLK36MHZ                         0x61  // select 36MHz PLL output
#define EVE_CLK48MHZ                         0x62  // select 48MHz PLL output
#define EVE_RST_PULSE                        0x68  // reset core - all registers to default & processors reset

// memory map
#define EVE_RAM_GP                     0x000000UL  // general purpose RAM
#define EVE_RAM_DL                     0x300000UL  // Display List RAM 
#define EVE_RAM_REG                    0x302000UL  // EVE registers 
#define EVE_RAM_CMD                    0x308000UL  // circular buffer co-processor commands

// memory sizes
#define EVE_RAM_GP_SIZE               1024*1024UL  // 1024 KB General Purpose RAM
#define EVE_RAM_DL_SIZE                  8*1024UL  //    8 KB Display List RAM
#define EVE_RAM_REG_SIZE                 4*1024UL  //    4 KB EVE register space
#define EVE_RAM_CMD_SIZE                 4*1024UL  //    4 KB command RAM, read by co-pro

// misc. locations
#define EVE_ROM_CHIPID               0x000c0000UL  // located in RAM_GP map
#define EVE_ROM_FONT                 0x001e0000UL
#define EVE_ROM_FONT_ADDR            0x002ffffcUL  // pointer to ROM fonts

// ** EVE Registers
// graphics engine registers
#define EVE_REG_HCYCLE               0x0030202cUL
#define EVE_REG_HOFFSET              0x00302030UL
#define EVE_REG_HSIZE                0x00302034UL
#define EVE_REG_HSYNC0               0x00302038UL
#define EVE_REG_HSYNC1               0x0030203cUL
#define EVE_REG_VCYCLE               0x00302040UL
#define EVE_REG_VOFFSET              0x00302044UL
#define EVE_REG_VSIZE                0x00302048UL
#define EVE_REG_VSYNC0               0x0030204cUL
#define EVE_REG_VSYNC1               0x00302050UL
#define EVE_REG_DLSWAP               0x00302054UL // rd 0b00: DL ready, render screen: wr 0b01: after current line / wr 0b10: after current frame
#define EVE_REG_ROTATE               0x00302058UL
#define EVE_REG_OUTBITS              0x0030205cUL
#define EVE_REG_DITHER               0x00302060UL
#define EVE_REG_SWIZZLE              0x00302064UL
#define EVE_REG_CSPREAD              0x00302068UL
#define EVE_REG_PCLK_POL             0x0030206cUL
#define EVE_REG_PCLK                 0x00302070UL
#define EVE_REG_TAG                  0x0030207cUL 
#define EVE_REG_TAG_X                0x00302074UL
#define EVE_REG_TAG_Y                0x00302078UL

// audio registers
#define EVE_REG_VOL_PB               0x00302080UL  // set volume audio playback: 0x00: mute / 0xff: full wack
#define EVE_REG_VOL_SOUND            0x00302084UL  // set volume synthesizer   : 0x00: mute / 0xff: full wack
#define EVE_REG_SOUND                0x00302088UL  // select synthesized sound
#define EVE_REG_PLAY                 0x0030208cUL  // 0b1: play synthesized sound
#define EVE_REG_PLAYBACK_START       0x003020b4UL
#define EVE_REG_PLAYBACK_LENGTH      0x003020b8UL
#define EVE_REG_PLAYBACK_READPTR     0x003020bcUL
#define EVE_REG_PLAYBACK_FREQ        0x003020c0UL
#define EVE_REG_PLAYBACK_FORMAT      0x003020c4UL
#define EVE_REG_PLAYBACK_LOOP        0x003020c8UL
#define EVE_REG_PLAYBACK_PLAY        0x003020ccUL

// touch screen registers
// -- common
#define EVE_REG_TOUCH_CONFIG         0x00302168UL  // don't edit, works with default values
#define EVE_REG_TOUCH_TRANSFORM_A    0x00302150UL  // touch screen calibration, fixed point format (S.15.16)
#define EVE_REG_TOUCH_TRANSFORM_B    0x00302154UL  // touch screen calibration, fixed point format (S.15.16)
#define EVE_REG_TOUCH_TRANSFORM_C    0x00302158UL  // touch screen calibration, fixed point format (S.15.16)
#define EVE_REG_TOUCH_TRANSFORM_D    0x0030215cUL  // touch screen calibration, fixed point format (S.15.16)
#define EVE_REG_TOUCH_TRANSFORM_E    0x00302160UL  // touch screen calibration, fixed point format (S.15.16)
#define EVE_REG_TOUCH_TRANSFORM_F    0x00302164UL  // touch screen calibration, fixed point format (S.15.16)
#define EVE_REG_TOUCH_RAW_XY         0x0030211cUL

/*
// -- resistive touch (not used with ME813A-WH50C module)
#define EVE_REG_TOUCH_MODE           0x00302104UL
#define EVE_REG_TOUCH_ADC_MODE       0x00302108UL
#define EVE_REG_TOUCH_CHARGE         0x0030210cUL
#define EVE_REG_TOUCH_SETTLE         0x00302110UL
#define EVE_REG_TOUCH_OVERSAMPLE     0x00302114UL
#define EVE_REG_TOUCH_RZTHRESH       0x00302118UL
#define EVE_REG_TOUCH_RZ             0x00302120UL
#define EVE_REG_TOUCH_SCREEN_XY      0x00302124UL
#define EVE_REG_TOUCH_TAG_XY         0x00302128UL
#define EVE_REG_TOUCH_TAG            0x0030212cUL
#define EVE_REG_TOUCH_DIRECT_XY      0x0030218cUL
#define EVE_REG_TOUCH_DIRECT_Z1Z2    0x00302190UL
*/
// -- capacitive touch
#define EVE_REG_CTOUCH_MODE          0x00302104UL // 0b00: off / 0b11: on
#define EVE_REG_CTOUCH_EXTENDED      0x00302108UL // 0b0: extended mode, 5 touch points, 0b1: compatiblity mode, 1 touch point
#define EVE_REG_CTOUCH_TOUCH0_XY     0x00302124UL // ? only listed in datasheet ?
#define EVE_REG_CTOUCH_TOUCH1_XY     0x0030211cUL 
#define EVE_REG_CTOUCH_TOUCH2_XY     0x0030218cUL
#define EVE_REG_CTOUCH_TOUCH3_XY     0x00302190UL
#define EVE_REG_CTOUCH_TOUCH4_X      0x0030216cUL
#define EVE_REG_CTOUCH_TOUCH4_Y      0x00302120UL
#define EVE_REG_CTOUCH_TAG_XY        0x00302128UL
#define EVE_REG_CTOUCH_TAG           0x0030212cUL
#define EVE_REG_CTOUCH_TAG1_XY       0x00302130UL // ? only listed in datasheet ?
#define EVE_REG_CTOUCH_TAG1          0x00302134UL // ? only listed in datasheet ?
#define EVE_REG_CTOUCH_TAG2_XY       0x00302138UL // ? only listed in datasheet ?
#define EVE_REG_CTOUCH_TAG2          0x0030213cUL // ? only listed in datasheet ?
#define EVE_REG_CTOUCH_TAG3_XY       0x00302140UL // ? only listed in datasheet ?
#define EVE_REG_CTOUCH_TAG3          0x00302144UL // ? only listed in datasheet ?
#define EVE_REG_CTOUCH_TAG4_XY       0x00302148UL // ? only listed in datasheet ?
#define EVE_REG_CTOUCH_TAG4          0x0030214cUL // ? only listed in datasheet ?

// co-processor engine registers
#define EVE_REG_CMD_WRITE            0x003020fcUL  // command FIFO end address   - updated by host
#define EVE_REG_CMD_READ             0x003020f8UL  // command FIFO start address - updated by co-pro
#define EVE_REG_CMD_DL               0x00302100UL  // Display List start address
#define EVE_REG_CMDB_SPACE           0x00302574UL  // free space in RAM_CMD
#define EVE_REG_CMDB_WRITE           0x00302578UL  // bulk data write to RAM_CMD (auto increment????)

// special registers
#define EVE_REG_TRACKER              0x00309000UL // tracks object touch point 0
#define EVE_REG_TRACKER_1            0x00309004UL // tracks object touch point 1
#define EVE_REG_TRACKER_2            0x00309008UL // tracks object touch point 2
#define EVE_REG_TRACKER_3            0x0030900cUL // tracks object touch point 3
#define EVE_REG_TRACKER_4            0x00309010UL // tracks object touch point 4
#define EVE_REG_MEDIAFIFO_READ       0x00309014UL // ? only listed in programmers guide ?
#define EVE_REG_MEDIAFIFO_WRITE      0x00309018UL // ? only listed in programmers guide ?

// misc. registers
#define EVE_REG_ID                   0x00302000UL  // FT813 chip ID (0x7c)
#define EVE_REG_FRAMES               0x00302004UL  // number of screen frames since reset
#define EVE_REG_CLOCK                0x00302008UL  // number of main clocks since reset
#define EVE_REG_FREQUENCY            0x0030200cUL  // main clock frequency, default 60Mc
#define EVE_REG_CPURESET             0x00302020UL  // 0b001: copro / 0b010: touch / 0b100: audio rset
#define EVE_REG_GPIO_DIR             0x00302090UL  // bit 1..0 for GPIO1..0, bit 7 for pin DISP
#define EVE_REG_GPIO                 0x00302094UL
#define EVE_REG_GPIOX_DIR            0x00302098UL
#define EVE_REG_GPIOX                0x0030209cUL
#define EVE_REG_INT_FLAGS            0x003020a8UL
#define EVE_REG_INT_EN               0x003020acUL
#define EVE_REG_INT_MASK             0x003020b0UL
#define EVE_REG_PWM_HZ               0x003020d0UL  // backlight PWM freq., range 250Hz..10kHz
#define EVE_REG_PWM_DUTY             0x003020d4UL  // backlight PWM, 0: off / 128: full wack
#define EVE_REG_TRIM                 0x00302180UL  // internal clock trim
#define EVE_REG_SPI_WIDTH            0x00302188UL  // SPI bus witdt & extra dummy during bye read access

// <> not listed in Programmers Guide ?
#define EVE_REG_MACRO_0              0x003020d8UL
#define EVE_REG_MACRO_1              0x003020dcUL

// Ft81x EVE commands
#define CMD_DLSTART                    0xffffff00
#define CMD_SWAP                       0xffffff01
#define CMD_INTERRUPT                  0xffffff02
#define CMD_BGCOLOR                    0xffffff09
#define CMD_FGCOLOR                    0xffffff0a
#define CMD_GRADIENT                   0xffffff0b
#define CMD_TEXT                       0xffffff0c
#define CMD_BUTTON                     0xffffff0d
#define CMD_KEYS                       0xffffff0e
#define CMD_PROGRESS                   0xffffff0f
#define CMD_SLIDER                     0xffffff10
#define CMD_SCROLLBAR                  0xffffff11
#define CMD_TOGGLE                     0xffffff12
#define CMD_GAUGE                      0xffffff13
#define CMD_CLOCK                      0xffffff14
#define CMD_CALIBRATE                  0xffffff15
#define CMD_SPINNER                    0xffffff16
#define CMD_STOP                       0xffffff17
#define CMD_MEMCRC                     0xffffff18
#define CMD_REGREAD                    0xffffff19
#define CMD_MEMWRITE                   0xffffff1a
#define CMD_MEMSET                     0xffffff1b
#define CMD_MEMZERO                    0xffffff1c
#define CMD_MEMCPY                     0xffffff1d
#define CMD_APPEND                     0xffffff1e
#define CMD_SNAPSHOT                   0xffffff1f
#define CMD_INFLATE                    0xffffff22
#define CMD_GETPTR                     0xffffff23
#define CMD_LOADIMAGE                  0xffffff24
#define CMD_GETPROPS                   0xffffff25
#define CMD_LOADIDENTITY               0xffffff26
#define CMD_TRANSLATE                  0xffffff27
#define CMD_SCALE                      0xffffff28
#define CMD_ROTATE                     0xffffff29
#define CMD_SETMATRIX                  0xffffff2a
#define CMD_SETFONT                    0xffffff2b
#define CMD_TRACK                      0xffffff2c
#define CMD_DIAL                       0xffffff2d
#define CMD_NUMBER                     0xffffff2e
#define CMD_SCREENSAVER                0xffffff2f
#define CMD_SKETCH                     0xffffff30
#define CMD_LOGO                       0xffffff31
#define CMD_COLDSTART                  0xffffff32
#define CMD_GETMATRIX                  0xffffff33
#define CMD_GRADCOLOR                  0xffffff34
// additional commands for FT81x 
#define CMD_MEDIAFIFO                  0xffffff39
#define CMD_PLAYVIDEO                  0xffffff3A
#define CMD_ROMFONT                    0xffffff3F
#define CMD_SETBASE                    0xffffff38
#define CMD_SETBITMAP                  0xffffff43
#define CMD_SETFONT2                   0xffffff3B
#define CMD_SETROTATE                  0xffffff36
#define CMD_SETSCRATCH                 0xffffff3C
#define CMD_SNAPSHOT2                  0xffffff37
#define CMD_VIDEOFRAME                 0xffffff41
#define CMD_VIDEOSTART                 0xffffff40

// arguments for CLEAR command
#define CLR_COL                             0b100 // clear color buffer
#define CLR_STN                             0b010 // clear stencil buffer
#define CLR_TAG                             0b001 // clear tag buffer

// macros for FT8xx Display List generation 
#define ALPHA_FUNC(func,ref)         ((9UL<<24)|(((func)&7UL)<<8)|(((ref)&255UL)<<0))
#define BEGIN                        (0x1fUL<<24)
#define BITMAP_HANDLE(handle)        ((5UL<<24)|(((handle)&31UL)<<0))
#define BITMAP_LAYOUT(format,linestride,height) ((7UL<<24)|(((format)&31UL)<<19)|(((linestride)&1023UL)<<9)|(((height)&511UL)<<0))
#define BITMAP_SIZE(filter,wrapx,wrapy,width,height) ((8UL<<24)|(((filter)&1UL)<<20)|(((wrapx)&1UL)<<19)|(((wrapy)&1UL)<<18)|(((width)&511UL)<<9)|(((height)&511UL)<<0))
#define BITMAP_TRANSFORM_A(a)        ((21UL<<24)|(((a)&131071UL)<<0))
#define BITMAP_TRANSFORM_B(b)        ((22UL<<24)|(((b)&131071UL)<<0))
#define BITMAP_TRANSFORM_C(c)        ((23UL<<24)|(((c)&16777215UL)<<0))
#define BITMAP_TRANSFORM_D(d)        ((24UL<<24)|(((d)&131071UL)<<0))
#define BITMAP_TRANSFORM_E(e)        ((25UL<<24)|(((e)&131071UL)<<0))
#define BITMAP_TRANSFORM_F(f)        ((26UL<<24)|(((f)&16777215UL)<<0))
#define BLEND_FUNC(src,dst)          ((11UL<<24)|(((src)&7UL)<<3)|(((dst)&7UL)<<0))
#define CALL(dest)                   ((29UL<<24)|(((dest)&65535UL)<<0))
#define CELL(cell)                   ((6UL<<24)|(((cell)&127UL)<<0))
#define CLEAR                        (0x26UL<<24)
#define CLEAR_A                      (0x0fUL<<24)
#define CLEAR_RGB                    (0x02UL<<24)
#define CLEAR_STENCIL(s)             ((17UL<<24)|(((s)&255UL)<<0))
#define CLEAR_TAG                    (0x12UL<<24)
#define COLOR_A                      (0x10UL<<24)
#define COLOR_MASK(r,g,b,a)          ((32UL<<24)|(((r)&1UL)<<3)|(((g)&1UL)<<2)|(((b)&1UL)<<1)|(((a)&1UL)<<0))
#define COLOR_RGB                    (0x04UL<<24)
#define DISPLAY                      (0x00UL<<24)
#define END                          (0x21UL<<24)
#define JUMP(dest)                   ((30UL<<24)|(((dest)&65535UL)<<0))
#define LINE_WIDTH                   (0x0eUL<<24)
#define MACRO(m)                     ((37UL<<24)|(((m)&1UL)<<0))
#define POINT_SIZE                   (0x0dUL<<24)
#define RESTORE_CONTEXT              (0x23UL<<24)
#define RETURN                       (0x24UL<<24)
#define SAVE_CONTEXT                 (0x22UL<<24)
#define STENCIL_FUNC(func,ref,mask)  ((10UL<<24)|(((func)&7UL)<<16)|(((ref)&255UL)<<8)|(((mask)&255UL)<<0))
#define STENCIL_MASK(mask)           ((19UL<<24)|(((mask)&255UL)<<0))
#define STENCIL_OP(sfail,spass)      ((12UL<<24)|(((sfail)&7UL)<<3)|(((spass)&7UL)<<0))
#define TAG                          (0x03UL<<24)
#define TAG_MASK(mask)               ((20UL<<24)|(((mask)&1UL)<<0))
#define VERTEX2F(x,y)                ((1UL<<30)|(((x)&32767UL)<<15)|(((y)&32767UL)<<0))
#define VERTEX2II(x,y,handle,cell)   ((2UL<<30)|(((x)&511UL)<<21)|(((y)&511UL)<<12)|(((handle)&31UL)<<7)|(((cell)&127UL)<<0))
#define VERTEX_TRANSLATE_X(x)        ((0x2bUL<<24)|((x)&0xffffUL)<<0)
#define VERTEX_TRANSLATE_Y(y)        ((0x2cUL<<24)|((y)&0xffffUL)<<0)

// defines used for graphics commands 
#define EVE_NEVER                             0UL
#define EVE_LESS                              1UL
#define EVE_LEQUAL                            2UL
#define EVE_GREATER                           3UL
#define EVE_GEQUAL                            4UL
#define EVE_EQUAL                             5UL
#define EVE_NOTEQUAL                          6UL
#define EVE_ALWAYS                            7UL

// bitmap formats 
#define EVE_ARGB1555                          0UL
#define EVE_L1                                1UL
#define EVE_L4                                2UL
#define EVE_L8                                3UL
#define EVE_RGB332                            4UL
#define EVE_ARGB2                             5UL
#define EVE_ARGB4                             6UL
#define EVE_RGB565                            7UL
#define EVE_PALETTED                          8UL
#define EVE_TEXT8X8                           9UL
#define EVE_TEXTVGA                          10UL
#define EVE_BARGRAPH                         11UL

// stencil defines 
#define EVE_KEEP                              1UL
#define EVE_REPLACE                           2UL
#define EVE_INCR                              3UL
#define EVE_DECR                              4UL
#define EVE_INVERT                            5UL

// display list swap defines 
#define EVE_DLSWAP_DONE                       0UL
#define EVE_DLSWAP_LINE                       1UL
#define EVE_DLSWAP_FRAME                      2UL

// alpha blending 
#define EVE_ZERO                              0UL
#define EVE_ONE                               1UL
#define EVE_SRC_ALPHA                         2UL
#define EVE_DST_ALPHA                         3UL
#define EVE_ONE_MINUS_SRC_ALPHA               4UL
#define EVE_ONE_MINUS_DST_ALPHA               5UL

// interrupt bits
#define EVE_INT_SWAP                         0x01
#define EVE_INT_TOUCH                        0x02
#define EVE_INT_TAG                          0x04
#define EVE_INT_SOUND                        0x08
#define EVE_INT_PLAYBACK                     0x10
#define EVE_INT_CMDEMPTY                     0x20
#define EVE_INT_CMDFLAG                      0x40
#define EVE_INT_CONVCOMPLETE                 0x80

// graphics primitives
#define EVE_BITMAPS                           1UL
#define EVE_POINTS                            2UL
#define EVE_LINES                             3UL
#define EVE_LINE_STRIP                        4UL
#define EVE_EDGE_STRIP_R                      5UL
#define EVE_EDGE_STRIP_L                      6UL
#define EVE_EDGE_STRIP_A                      7UL
#define EVE_EDGE_STRIP_B                      8UL
#define EVE_RECTS                             9UL

// widget command 
#define EVE_OPT_MONO                            1
#define EVE_OPT_NODL                            2
#define EVE_OPT_FLAT                       0x0100
#define EVE_OPT_CENTERX                    0x0200
#define EVE_OPT_CENTERY                    0x0400
#define EVE_OPT_CENTER (EVE_OPT_CENTERX | EVE_OPT_CENTERY)
#define EVE_OPT_NOBACK                     0x1000
#define EVE_OPT_NOTICKS                    0x2000
#define EVE_OPT_NOHM                       0x4000
#define EVE_OPT_NOPOINTER                  0x4000
#define EVE_OPT_NOSECS                     0x8000
#define EVE_OPT_NOHANDS                    0xc000
#define EVE_OPT_RIGHTX                     0x0800
#define EVE_OPT_SIGNED                     0x0100

// audio sample types
#define EVE_LINEAR_SAMPLES                    0UL  // 8-bit signed samples
#define EVE_ULAW_SAMPLES                      1UL  // 8-bit ulaw samples 
#define EVE_ADPCM_SAMPLES                     2UL  // 4bit ima adpcm samples 

// synthesized sound
#define EVE_SILENCE                          0x00
#define EVE_SQUAREWAVE                       0x01
#define EVE_SINEWAVE                         0x02
#define EVE_SAWTOOTH                         0x03
#define EVE_TRIANGLE                         0x04
#define EVE_BEEPING                          0x05
#define EVE_ALARM                            0x06
#define EVE_WARBLE                           0x07
#define EVE_CAROUSEL                         0x08
#define EVE_PIPS(n)                  (0x0f + (n))
#define EVE_HARP                             0x40
#define EVE_XYLOPHONE                        0x41
#define EVE_TUBA                             0x42
#define EVE_GLOCKENSPIEL                     0x43
#define EVE_ORGAN                            0x44
#define EVE_TRUMPET                          0x45
#define EVE_PIANO                            0x46
#define EVE_CHIMES                           0x47
#define EVE_MUSICBOX                         0x48
#define EVE_BELL                             0x49
#define EVE_CLICK                            0x50
#define EVE_SWITCH                           0x51
#define EVE_COWBELL                          0x52
#define EVE_NOTCH                            0x53
#define EVE_HIHAT                            0x54
#define EVE_KICKDRUM                         0x55
#define EVE_POP                              0x56
#define EVE_CLACK                            0x57
#define EVE_CHACK                            0x58
#define EVE_MUTE                             0x60
#define EVE_UNMUTE                           0x61

// synthesized sound frequencies, midi notes 
#define EVE_MIDI_A0                            21
#define EVE_MIDI_A_0                           22
#define EVE_MIDI_B0                            23
#define EVE_MIDI_C1                            24
#define EVE_MIDI_C_1                           25
#define EVE_MIDI_D1                            26
#define EVE_MIDI_D_1                           27
#define EVE_MIDI_E1                            28
#define EVE_MIDI_F1                            29
#define EVE_MIDI_F_1                           30
#define EVE_MIDI_G1                            31
#define EVE_MIDI_G_1                           32
#define EVE_MIDI_A1                            33
#define EVE_MIDI_A_1                           34
#define EVE_MIDI_B1                            35
#define EVE_MIDI_C2                            36
#define EVE_MIDI_C_2                           37
#define EVE_MIDI_D2                            38
#define EVE_MIDI_D_2                           39
#define EVE_MIDI_E2                            40
#define EVE_MIDI_F2                            41
#define EVE_MIDI_F_2                           42
#define EVE_MIDI_G2                            43
#define EVE_MIDI_G_2                           44
#define EVE_MIDI_A2                            45
#define EVE_MIDI_A_2                           46
#define EVE_MIDI_B2                            47
#define EVE_MIDI_C3                            48
#define EVE_MIDI_C_3                           49
#define EVE_MIDI_D3                            50
#define EVE_MIDI_D_3                           51
#define EVE_MIDI_E3                            52
#define EVE_MIDI_F3                            53
#define EVE_MIDI_F_3                           54
#define EVE_MIDI_G3                            55
#define EVE_MIDI_G_3                           56
#define EVE_MIDI_A3                            57
#define EVE_MIDI_A_3                           58
#define EVE_MIDI_B3                            59
#define EVE_MIDI_C4                            60
#define EVE_MIDI_C_4                           61
#define EVE_MIDI_D4                            62
#define EVE_MIDI_D_4                           63
#define EVE_MIDI_E4                            64
#define EVE_MIDI_F4                            65
#define EVE_MIDI_F_4                           66
#define EVE_MIDI_G4                            67
#define EVE_MIDI_G_4                           68
#define EVE_MIDI_A4                            69
#define EVE_MIDI_A_4                           70
#define EVE_MIDI_B4                            71
#define EVE_MIDI_C5                            72
#define EVE_MIDI_C_5                           73
#define EVE_MIDI_D5                            74
#define EVE_MIDI_D_5                           75
#define EVE_MIDI_E5                            76
#define EVE_MIDI_F5                            77
#define EVE_MIDI_F_5                           78
#define EVE_MIDI_G5                            79
#define EVE_MIDI_G_5                           80
#define EVE_MIDI_A5                            81
#define EVE_MIDI_A_5                           82
#define EVE_MIDI_B5                            83
#define EVE_MIDI_C6                            84
#define EVE_MIDI_C_6                           85
#define EVE_MIDI_D6                            86
#define EVE_MIDI_D_6                           87
#define EVE_MIDI_E6                            88
#define EVE_MIDI_F6                            89
#define EVE_MIDI_F_6                           90
#define EVE_MIDI_G6                            91
#define EVE_MIDI_G_6                           92
#define EVE_MIDI_A6                            93
#define EVE_MIDI_A_6                           94
#define EVE_MIDI_B6                            95
#define EVE_MIDI_C7                            96
#define EVE_MIDI_C_7                           97
#define EVE_MIDI_D7                            98
#define EVE_MIDI_D_7                           99
#define EVE_MIDI_E7                           100
#define EVE_MIDI_F7                           101
#define EVE_MIDI_F_7                          102
#define EVE_MIDI_G7                           103
#define EVE_MIDI_G_7                          104
#define EVE_MIDI_A7                           105
#define EVE_MIDI_A_7                          106
#define EVE_MIDI_B7                           107
#define EVE_MIDI_C8                           108

// -- globals
extern char eve_buf[128];              // temp storage for printing strings

// -- prototypes
void init_eve (void);

uint8_t eve_init (void);

void eve_host_cmd_write (uint8_t dat);

uint8_t  eve_mem_rd8  (uint32_t adr);
uint16_t eve_mem_rd16 (uint32_t adr);
uint32_t eve_mem_rd32 (uint32_t adr);

void     eve_mem_wr8  (uint32_t adr, uint8_t  dat);
void     eve_mem_wr16 (uint32_t adr, uint16_t dat);
void     eve_mem_wr32 (uint32_t adr, uint32_t dat);

void eve_cmd_dl (uint32_t cmd);

void eve_cmd_wait  (void);
void eve_cmd_start (uint32_t color);
void eve_cmd_stop  (void);
void eve_cmd_write (uint32_t dat);
void eve_cmd_exec  (void);

void eve_cmd_bgcolor  (uint32_t col);
void eve_cmd_fgcolor  (uint32_t col);
void eve_cmd_point    (int16_t x0, int16_t y0, uint16_t size);
void eve_cmd_text     (int16_t x0, int16_t y0, int16_t font, uint16_t options, char* str);
void eve_cmd_number   (int16_t x0, int16_t y0, int16_t font, uint16_t options, int32_t num);
void eve_cmd_setbase  (uint32_t base);
void eve_cmd_fgcolor  (uint32_t col);
void eve_cmd_bgcolor  (uint32_t col);
void eve_cmd_button   (int16_t x0, int16_t y0, int16_t w0, int16_t h0, int16_t font, uint16_t options, const char* str);
void eve_cmd_track    (int16_t x_pos, int16_t y_pos, int16_t width, int16_t height, int16_t tag);
void eve_cmd_progress (int16_t x_pos, int16_t y_pos, int16_t width, int16_t height, uint16_t options, uint16_t val, uint16_t range);
void eve_cmd_snapshot (uint32_t adr);

void eve_write_string (const char *str);

void eve_dump_tftregs (void);

void eve_backlight (uint8_t dat);

void eve_sound     (uint8_t sound, uint8_t pitch, uint16_t len);
void eve_sound_on  (uint8_t sound, uint8_t pitch, uint8_t volume);
void eve_sound_off (void);

uint8_t eve_pen_down (void);

#endif




