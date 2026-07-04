// file    : tft.c
// author  : rb
// purpose : ME813A-WH50C TFT module routines - FTDI FT813 EVE (Embedded Video Engine) - 
// date    : 190620
// last    : 191217
//
// note    : supports CTPM with Focaltech FT5x06 series or Azotech IQS5xx series drive chips 

#include "includes.h"

#define EVE_CMD_DL_DEBUG   0           // debug data from 'eve_cmd_dl' routine
#define EVE_CMD_DEBUG      0           // debug EVE command FIFO routines
#define EVE_MEM_DEBUG      0           // debug memory read/write

// global storage
char eve_buf[128];                 // temp storage for printing strings

// Display List & command FIFO offset pointers
uint32_t dl_ofs  = 0;
uint32_t cmd_ofs = 0;

// set up EVE processor
void init_eve (void)
{
  // init EVE
  eve_init (); 
}

// init EVE (note: SPI clock should be =< 11Mc)
uint8_t eve_init (void)
{
	uint16_t timeout = 0;

  // cycle POWER_DOWN_L pin
	tft_power_off ();
	msleep (100);	    // relax at least 5ms 
  tft_power_on ();
	msleep (100);	    // relax at least 20ms 

  // write EVE Host Commands: select external clock & start FT813
  eve_host_cmd_write (EVE_CLKEXT);
	eve_host_cmd_write (EVE_ACTIVE);

  // wait while busy 
	while (eve_mem_rd8 (EVE_REG_ID) != 0x7c)	
	{
   	msleep (10);
		
		if (timeout++ > 100)
    {
      printf ("#init failed, check TFT\n");
			return 0;
    }
	}

  printf ("#selftest ready, TFT ok\n");

  // turn off backlight
  eve_backlight (EVE_BLIGHT_OFF);

	// init TFT display
	eve_mem_wr16 (EVE_REG_HCYCLE,   EVE_HCYCLE);	 // total number of clocks per line, including front/back porch
	eve_mem_wr16 (EVE_REG_HOFFSET,  EVE_HOFFSET);	 // start of active line 
	eve_mem_wr16 (EVE_REG_HSIZE,    EVE_HSIZE);		 // active display width 
	eve_mem_wr16 (EVE_REG_HSYNC0,   EVE_HSYNC0);	 // start of horizontal sync pulse 
	eve_mem_wr16 (EVE_REG_HSYNC1,   EVE_HSYNC1);	 // end of horizontal sync pulse 
	eve_mem_wr16 (EVE_REG_VSIZE,    EVE_VSIZE);		 // active display height 
	eve_mem_wr16 (EVE_REG_VOFFSET,  EVE_VOFFSET);	 // start of active screen 
	eve_mem_wr16 (EVE_REG_VCYCLE,   EVE_VCYCLE);	 // total number of lines per screen, incl pre/post 
	eve_mem_wr16 (EVE_REG_VSYNC0,   EVE_VSYNC0);	 // start of vertical sync pulse 
	eve_mem_wr16 (EVE_REG_VSYNC1,   EVE_VSYNC1);	 // end of vertical sync pulse 
	eve_mem_wr8  (EVE_REG_SWIZZLE,  EVE_SWIZZLE);	 // FT813 output to LCD - pin order 
	eve_mem_wr8  (EVE_REG_PCLK_POL, EVE_PCLKPOL);	 // LCD data is clocked in on this PCLK edge 
	eve_mem_wr8  (EVE_REG_CSPREAD,	EVE_CSPREAD);  // helps with noise, when set to 1 fewer signals are changed simultaneously, reset-default: 1

  // set DISP pin direction & enable
  eve_mem_wr8  (EVE_REG_GPIO_DIR, 0x80 | eve_mem_rd8 (EVE_REG_GPIO_DIR));
  eve_mem_wr8  (EVE_REG_GPIO    , 0x80 | eve_mem_rd8 (EVE_REG_GPIO));
 
  // set audio amplifier control pin direction 
  eve_mem_wr8  (EVE_REG_GPIOX_DIR, 0x04 | eve_mem_rd8 (EVE_REG_GPIOX_DIR));

  // enable PCLK, enable display & wait while EVE busy
  eve_mem_wr8  (EVE_REG_PCLK, EVE_PCLK);
  msleep (500);                                  // avoid flicker at POR

  // init touch screen controller
//eve_mem_wr8 (EVE_REG_CPURESET, 0b010);         // assert Touch Engine RESET (as per DS 4.7.6) <> needed?

  // write touch screen calibration parameters
  eve_mem_wr32 (EVE_REG_TOUCH_TRANSFORM_A, 0x00010f04);
  eve_mem_wr32 (EVE_REG_TOUCH_TRANSFORM_B, 0x000004bc);
  eve_mem_wr32 (EVE_REG_TOUCH_TRANSFORM_C, 0xffe57e66);
  eve_mem_wr32 (EVE_REG_TOUCH_TRANSFORM_D, 0xfffffce0);
  eve_mem_wr32 (EVE_REG_TOUCH_TRANSFORM_E, 0xfffefe28);
  eve_mem_wr32 (EVE_REG_TOUCH_TRANSFORM_F, 0x01e74558);

//eve_mem_wr8 (EVE_REG_CPURESET, 0b000);         // deassert Touch Engine RESET <> needed?

  // enable touch <> needed?
//eve_mem_wr8 (EVE_REG_CTOUCH_MODE, 0x03);

  // enable backlight when EVE ready
  eve_backlight (EVE_BLIGHT_ON);

	return 1;
}

// ** EVE Host command routines 
// write 8-bit EVE Host Command
// note: used to set Host registers, i.e. clock source selection, reset, powerdown, sleep etc.
// note: Host registers can't be read
void eve_host_cmd_write (uint8_t dat)
{
  // write command & padding
	tft_sel ();
  spi3_write_byte (dat);
	spi3_write_byte (0x00);
	spi3_write_byte (0x00);
	tft_unsel ();
}

// ** EVE memory command routines 
// read byte from memory
uint8_t eve_mem_rd8 (uint32_t adr)
{
	uint8_t dat;

  // SPI select EVE
	tft_sel ();

  // send 22-bit read address with read bits & dummy byte
	spi3_write_byte ((uint8_t)(adr >> 16) | EVE_MEM_RD); 
	spi3_write_byte ((uint8_t)(adr >> 8));
	spi3_write_byte ((uint8_t)(adr));
	spi3_write_byte (0x00);

  // read data
	dat = spi3_read_byte (); 
	
  // SPI deselect EVE 
  tft_unsel ();

  debug (EVE_MEM_DEBUG, "mem_rd: adr: 0x%06x dat: 0x%02x\n", adr, dat);

	return (dat);	
}

// read word from memory
uint16_t eve_mem_rd16 (uint32_t adr)
{
	uint16_t dat;
  
  // SPI select EVE
	tft_sel ();

  // send 22-bit read address with read bits & dummy byte
	spi3_write_byte ((uint8_t)(adr >> 16) | EVE_MEM_RD); 
	spi3_write_byte ((uint8_t)(adr >> 8));
	spi3_write_byte ((uint8_t)(adr));
	spi3_write_byte (0x00);

  // read data
	dat = spi3_read_byte (); 
	dat = ((uint16_t)spi3_read_byte () << 8) | dat; 

  // SPI deselect EVE 
  tft_unsel ();

	return (dat);	
}

// read long from memeory
uint32_t eve_mem_rd32 (uint32_t adr)
{
	uint32_t dat;

  // SPI select EVE
	tft_sel ();

  // send 22-bit read address with write bits & dummy byte
	spi3_write_byte ((uint8_t)(adr >> 16) | EVE_MEM_RD); 
	spi3_write_byte ((uint8_t)(adr >> 8));
	spi3_write_byte ((uint8_t)(adr));
	spi3_write_byte (0x00);

  // read data
	dat = spi3_read_byte (); 
	dat = ((uint32_t)spi3_read_byte () <<  8) | dat; 
	dat = ((uint32_t)spi3_read_byte () << 16) | dat; 
	dat = ((uint32_t)spi3_read_byte () << 24) | dat; 

  // SPI deselect EVE 
  tft_unsel ();

	return (dat);	
}

// write byte to memory
void eve_mem_wr8 (uint32_t adr, uint8_t dat)
{
  // SPI select EVE
	tft_sel ();

  // send 22-bit read address with write bits & data byte
	spi3_write_byte ((uint8_t)(adr >> 16) | EVE_MEM_WR);
	spi3_write_byte ((uint8_t)(adr >> 8));
	spi3_write_byte ((uint8_t)(adr));
	spi3_write_byte (dat);

  // SPI deselect EVE 
  tft_unsel ();

  debug (EVE_MEM_DEBUG, "mem_wr: adr: 0x%06x dat: 0x%02x\n", adr, dat);
}

// write word to memory
void eve_mem_wr16 (uint32_t adr, uint16_t dat)
{
  // SPI select EVE
	tft_sel ();

  // send 22-bit address with write bits & data
	spi3_write_byte ((uint8_t)(adr >> 16) | EVE_MEM_WR);
	spi3_write_byte ((uint8_t)(adr >> 8));
	spi3_write_byte ((uint8_t)(adr));
	spi3_write_byte ((uint8_t)dat);
	spi3_write_byte ((uint8_t)(dat >> 8));

  // SPI deselect EVE 
  tft_unsel ();
}

// write long to memory
void eve_mem_wr32 (uint32_t adr, uint32_t dat)
{
  // SPI select EVE
	tft_sel ();

  // send 22-bit address with write bits & data
	spi3_write_byte ((uint8_t)(adr >> 16) | EVE_MEM_WR);
	spi3_write_byte ((uint8_t)(adr >> 8));
	spi3_write_byte ((uint8_t)(adr));
	spi3_write_byte ((uint8_t)dat);
	spi3_write_byte ((uint8_t)(dat >> 8));
	spi3_write_byte ((uint8_t)(dat >> 16));
	spi3_write_byte ((uint8_t)(dat >> 24));

  // SPI deselect EVE 
  tft_unsel ();
}


// ** EVE Display List routines (better to use command FIFO)
// add command to Display List
void eve_cmd_dl (uint32_t cmd)
{
  debug (EVE_CMD_DL_DEBUG, "adr: 0x%08x cmd: 0x%08x\n", EVE_RAM_DL + dl_ofs, cmd);

  // write command buffer
  eve_mem_wr32 (EVE_RAM_DL + dl_ofs, cmd);

  dl_ofs += 4;
}


// ** EVE command FIFO routines
// wait for command FIFO empty
void eve_cmd_wait (void)
{
  uint16_t wr_ptr = eve_mem_rd16 (EVE_REG_CMD_WRITE);  // command FIFO write pointer
  uint16_t rd_ptr = eve_mem_rd16 (EVE_REG_CMD_READ);   // command FIFO read pointer

  // whait for FIFO processed (rd_ptr == rw_ptr)
  while (rd_ptr != wr_ptr)
  {
    wr_ptr = eve_mem_rd16 (EVE_REG_CMD_WRITE);
    rd_ptr = eve_mem_rd16 (EVE_REG_CMD_READ);
  }

  // update FIFO command offset 
  cmd_ofs = wr_ptr;

  debug (EVE_CMD_DEBUG, "cmd_ofs: 0x%03x\n", cmd_ofs);
}

// begin sending co-processor commands
// note: 'tft_sel' remains asserted
void eve_cmd_start (uint32_t color)
{
  // wait while command FIFO busy & update offset first empty location
  eve_cmd_wait ();

  // update start write addres command FIFO 
  uint32_t adr = EVE_RAM_CMD + cmd_ofs;

  // select TFT 
  tft_sel ();               

  // send 22-bit read address & set write bit
	spi3_write_byte ((uint8_t)(adr >> 16) | EVE_MEM_WR);
	spi3_write_byte ((uint8_t)(adr >> 8));
	spi3_write_byte ((uint8_t)(adr));

  // start new Display List, set background color screen & clear screen
  eve_cmd_write (CMD_DLSTART);
  eve_cmd_write (CLEAR_RGB | color);
  eve_cmd_write (COLOR_A | 0xff);      // alpha blending off
  eve_cmd_write (CLEAR | CLR_COL | CLR_STN | CLR_TAG);

  debug (EVE_CMD_DEBUG, "\ncmd_new: adr: 0x%06lx\n", (adr | (EVE_MEM_WR << 16)));
}

// stop sensding co-processor commands & execute Display List
// note: deassert 'tft_sel'
void eve_cmd_stop (void)
{
  // mark end of Display List & swap display buffers
  eve_cmd_write (END);  
  eve_cmd_write (DISPLAY);
  eve_cmd_write (CMD_SWAP);

  // start execution: update command FIFO write pointer
  eve_cmd_exec ();
  
  // wait while busy
  eve_cmd_wait ();
}

// write command to command FIFO
// note: 'tft_sel' must be asserted
void eve_cmd_write (uint32_t dat)
{
  // send 32-bit data byte, LSB first
	spi3_write_byte ((uint8_t)dat);
	spi3_write_byte ((uint8_t)(dat >> 8));
	spi3_write_byte ((uint8_t)(dat >> 16));
	spi3_write_byte ((uint8_t)(dat >> 24));

  // adjust offset pointer 
  cmd_ofs += 4;

  debug (EVE_CMD_DEBUG, "cmd_add: adr: 0x%06lx dat: 0x%08lx\n", EVE_RAM_CMD + cmd_ofs, dat);
}

// start execution FIFO commands by updating command write pointer 
void eve_cmd_exec (void)
{
  // deselect TFT
  tft_unsel ();              

  // update write pointer with new command offsset
  eve_mem_wr32 (EVE_REG_CMD_WRITE, cmd_ofs);

  debug (EVE_CMD_DEBUG, "cmd_exe: adr: 0x%06lx ofs: 0x%08lx\n", EVE_REG_CMD_WRITE, cmd_ofs);
}

// set background color
void eve_cmd_bgcolor (uint32_t color)
{
	eve_cmd_write (CMD_BGCOLOR);

  spi3_write_byte ((uint8_t)(color));
	spi3_write_byte ((uint8_t)(color >> 8));
	spi3_write_byte ((uint8_t)(color >> 16));
	spi3_write_byte (0x00);

  // adjust offset pointer 
  cmd_ofs += 4;
}

// set foreground color
void eve_cmd_fgcolor (uint32_t color)
{
	eve_cmd_write (CMD_FGCOLOR);

  spi3_write_byte ((uint8_t)(color));
	spi3_write_byte ((uint8_t)(color >> 8));
	spi3_write_byte ((uint8_t)(color >> 16));
	spi3_write_byte (0x00);

  // adjust offset pointer 
  cmd_ofs += 4;
}

// draw point
void eve_cmd_point (int16_t x0, int16_t y0, uint16_t size)
{
	uint32_t tmp;

	eve_cmd_write (BEGIN | EVE_POINTS);

  // calculate point size
  tmp = POINT_SIZE | (size*16);

  spi3_write_byte ((uint8_t)(tmp));
  spi3_write_byte ((uint8_t)(tmp >> 8));
  spi3_write_byte ((uint8_t)(tmp >> 16));
  spi3_write_byte ((uint8_t)(tmp >> 24));

  // calculate position
  tmp = VERTEX2F (x0*16, y0*16);

  spi3_write_byte((uint8_t)(tmp));
  spi3_write_byte((uint8_t)(tmp >> 8));
  spi3_write_byte((uint8_t)(tmp >> 16));
  spi3_write_byte((uint8_t)(tmp >> 24));

  spi3_write_byte((uint8_t)(END));
  spi3_write_byte((uint8_t)(END >> 8));
  spi3_write_byte((uint8_t)(END >> 16));
  spi3_write_byte((uint8_t)(END >> 24));

  // adjust offset pointer 
  cmd_ofs += 12;
}

// write text to command memory
void eve_cmd_text (int16_t x0, int16_t y0, int16_t font, uint16_t options, char* str)
{
	eve_cmd_write (CMD_TEXT);

	spi3_write_byte ((uint8_t)(x0));
	spi3_write_byte ((uint8_t)(x0 >> 8));

	spi3_write_byte ((uint8_t)(y0));
	spi3_write_byte ((uint8_t)(y0 >> 8));

	spi3_write_byte ((uint8_t)(font));
	spi3_write_byte ((uint8_t)(font >> 8));

	spi3_write_byte ((uint8_t)(options));
	spi3_write_byte ((uint8_t)(options >> 8));

  // adjust offset pointer 
  cmd_ofs += 8;
	
  // send string
  uint8_t cnt = 0;
	uint8_t pad = 0;

  // send string characters to command memory
	while (str[cnt] != '\0')
  	spi3_write_byte (str[cnt++]);

	// add at least one 0x00 byte, pad to 4-byte align
	pad = cnt & 3;        // 0, 1, 2 or 3 
	pad = 4 - pad;        // 4, 3, 2 or 1 
	cnt += pad;

	while (pad-- > 0)
  	spi3_write_byte (0x00);

  // adjust offset pointer 
  cmd_ofs += cnt;
}

// draw number with adjustable base <> obselete, use 'cmd_text ()' ?
void eve_cmd_number (int16_t x0, int16_t y0, int16_t font, uint16_t options, int32_t num)
{
	eve_cmd_write (CMD_NUMBER);

  spi3_write_byte ((uint8_t)(x0));
  spi3_write_byte ((uint8_t)(x0 >> 8));

  spi3_write_byte ((uint8_t)(y0));
  spi3_write_byte ((uint8_t)(y0 >> 8));

  spi3_write_byte ((uint8_t)(font));
  spi3_write_byte ((uint8_t)(font >> 8));

  spi3_write_byte ((uint8_t)(options));
  spi3_write_byte ((uint8_t)(options >> 8));

  spi3_write_byte ((uint8_t)(num));
  spi3_write_byte ((uint8_t)(num >> 8));
  spi3_write_byte ((uint8_t)(num >> 16));
  spi3_write_byte ((uint8_t)(num >> 24));

  // adjust offset pointer 
  cmd_ofs += 12;
}

// set number base <> obselete, use 'cmd_text ()' ?
void eve_cmd_setbase (uint32_t base)
{
  eve_cmd_write (CMD_SETBASE);

  spi3_write_byte ((uint8_t)(base));
  spi3_write_byte ((uint8_t)(base >> 8));
  spi3_write_byte ((uint8_t)(base >> 16));
  spi3_write_byte ((uint8_t)(base >> 24));

  // adjust offset pointer 
  cmd_ofs += 4;
}

// display button 
void eve_cmd_button (int16_t x_pos, int16_t y_pos, int16_t width, int16_t height, int16_t font, uint16_t options, const char* str)
{
	eve_cmd_write (CMD_BUTTON);

	spi3_write_byte ((uint8_t)(x_pos));
	spi3_write_byte ((uint8_t)(x_pos >> 8));

	spi3_write_byte ((uint8_t)(y_pos));
	spi3_write_byte ((uint8_t)(y_pos >> 8));

	spi3_write_byte ((uint8_t)(width));
	spi3_write_byte ((uint8_t)(width >> 8));

	spi3_write_byte ((uint8_t)(height));
	spi3_write_byte ((uint8_t)(height >> 8));

	spi3_write_byte ((uint8_t)(font));
	spi3_write_byte ((uint8_t)(font >> 8));

	spi3_write_byte ((uint8_t)(options));
	spi3_write_byte ((uint8_t)(options >> 8));

  // adjust offset pointer 
  cmd_ofs += 12;

	eve_write_string (str);
}

// write string as part of a command
void eve_write_string (const char *str)
{
	uint8_t cnt = 0;
	uint8_t pad = 0;

	while (str[cnt] != '\0')
	{
  	spi3_write_byte (str[cnt]);
		cnt++;
	}

	// padd with at least 0x00 bytes, long align
	pad = cnt & 3;   // 0, 1, 2 or 3 
	pad = 4 - pad;   // 4, 3, 2 or 1 
	cnt += pad;

	while (pad > 0)
	{
  	spi3_write_byte (0x00);
		pad--;
	}

  // adjust offset pointer 
  cmd_ofs += cnt;
}

// track area for touch
void eve_cmd_track (int16_t x_pos, int16_t y_pos, int16_t width, int16_t height, int16_t tag)
{
	eve_cmd_write (CMD_TRACK);

	spi3_write_byte ((uint8_t)(x_pos));
	spi3_write_byte ((uint8_t)(x_pos >> 8));

	spi3_write_byte ((uint8_t)(y_pos));
	spi3_write_byte ((uint8_t)(y_pos >> 8));

	spi3_write_byte ((uint8_t)(width));
	spi3_write_byte ((uint8_t)(width >> 8));

	spi3_write_byte ((uint8_t)(height));
	spi3_write_byte ((uint8_t)(height >> 8));

	spi3_write_byte ((uint8_t)(tag));
	spi3_write_byte ((uint8_t)(tag >> 8));

	spi3_write_byte (0x00);
	spi3_write_byte (0x00);

  // adjust offset pointer 
  cmd_ofs += 12;
}

// display progress bar
void eve_cmd_progress (int16_t x_pos, int16_t y_pos, int16_t width, int16_t height, uint16_t options, uint16_t val, uint16_t range)
{
	eve_cmd_write (CMD_PROGRESS);

  spi3_write_byte ((uint8_t)(x_pos));
	spi3_write_byte ((uint8_t)(x_pos >> 8));

	spi3_write_byte ((uint8_t)(y_pos));
	spi3_write_byte ((uint8_t)(y_pos >> 8));

	spi3_write_byte ((uint8_t)(width));
	spi3_write_byte ((uint8_t)(width >> 8));

	spi3_write_byte ((uint8_t)(height));
	spi3_write_byte ((uint8_t)(height >> 8));

	spi3_write_byte ((uint8_t)(options));
	spi3_write_byte ((uint8_t)(options >> 8));

	spi3_write_byte ((uint8_t)(val));
	spi3_write_byte ((uint8_t)(val >> 8));

	spi3_write_byte ((uint8_t)(range));
	spi3_write_byte ((uint8_t)(range >> 8));

  // pad to 4-byte align
	spi3_write_byte (0x00);
	spi3_write_byte (0x00);

  // adjust offset pointer 
  cmd_ofs += 16;
}

// dump screen to RAM_G
void eve_cmd_snapshot (uint32_t adr)
{
	eve_cmd_write (CMD_SNAPSHOT);

	spi3_write_byte ((uint8_t)(adr));
	spi3_write_byte ((uint8_t)(adr >> 8));
	spi3_write_byte ((uint8_t)(adr >> 16));
	spi3_write_byte ((uint8_t)(adr >> 24));

  // adjust offset pointer 
  cmd_ofs += 4;
}

// control TFT backlight 
void eve_backlight (uint8_t dat)
{
  eve_mem_wr8 (EVE_REG_PWM_DUTY, dat);
}

// touch screen 'debouncer'
uint8_t eve_pen_down (void)
{ 
  // read TAG interrupt flag
  if (eve_mem_rd8 (EVE_REG_INT_FLAGS) & EVE_INT_TAG)
    return 1;
  else
    return 0;
}

// play sound
void eve_sound (uint8_t sound, uint8_t pitch, uint16_t len)
{
  // enable audio amplifier
  eve_mem_wr8  (EVE_REG_GPIOX, 0x04 | eve_mem_rd8 (EVE_REG_GPIOX));

  // play sound
  eve_mem_wr8  (EVE_REG_VOL_SOUND, 0x40);	                   // set synthesizer volume
  eve_mem_wr16 (EVE_REG_SOUND, ((uint8_t)pitch<<8) | sound); // select sound
  eve_mem_wr8  (EVE_REG_PLAY, 1);                            // play sound 

  // wait 
  msleep (len);

  // sound off
  eve_mem_wr16 (EVE_REG_SOUND, 0);	                         // play silence
  eve_mem_wr8  (EVE_REG_VOL_SOUND, 0x00);	                   // synthesizer volume off

  // shutdown audio amplifier (remove static noise)
  eve_mem_wr8  (EVE_REG_GPIOX, ~0x04 & eve_mem_rd8 (EVE_REG_GPIOX));  
}

// start playing sound
void eve_sound_on (uint8_t sound, uint8_t pitch, uint8_t volume)
{
  // enable audio amplifier
  eve_mem_wr8  (EVE_REG_GPIOX, 0x04 | eve_mem_rd8 (EVE_REG_GPIOX));

  // play sound
  eve_mem_wr8  (EVE_REG_VOL_SOUND, volume);	                 // set synthesizer volume
  eve_mem_wr16 (EVE_REG_SOUND, ((uint8_t)pitch<<8) | sound); // select sound & pitch
  eve_mem_wr8  (EVE_REG_PLAY, 1);                            // play sound 
}

// stop playing sound
void eve_sound_off (void)
{
  // sound off
  eve_mem_wr16 (EVE_REG_SOUND, 0);	                         // play silence
  eve_mem_wr8  (EVE_REG_VOL_SOUND, 0x00);	                   // synthesizer volume off

  // shutdown audio amplifier (remove static noise)
  eve_mem_wr8  (EVE_REG_GPIOX, ~0x04 & eve_mem_rd8 (EVE_REG_GPIOX));  
}

// dump EVE registers
void eve_dump_tftregs (void)
{
  printf ("EVE_REG_ID       : 0x%02lx\n", eve_mem_rd32 (EVE_REG_ID));
  printf ("EVE_REG_HCYCLE   : %3ld\n",    eve_mem_rd32 (EVE_REG_HCYCLE));
  printf ("EVE_REG_HOFFSET  : %3ld\n",    eve_mem_rd32 (EVE_REG_HOFFSET));
  printf ("EVE_REG_HSIZE    : %3ld\n",    eve_mem_rd32 (EVE_REG_HSIZE));
  printf ("EVE_REG_HSYNC0   : %3ld\n",    eve_mem_rd32 (EVE_REG_HSYNC0));
  printf ("EVE_REG_HSYNC1   : %3ld\n",    eve_mem_rd32 (EVE_REG_HSYNC1));
  printf ("EVE_REG_VCYCLE   : %3ld\n",    eve_mem_rd32 (EVE_REG_VCYCLE));
  printf ("EVE_REG_VOFFSET  : %3ld\n",    eve_mem_rd32 (EVE_REG_VOFFSET));
  printf ("EVE_REG_VSIZE    : %3ld\n",    eve_mem_rd32 (EVE_REG_VSIZE));
  printf ("EVE_REG_VSYNC0   : %3ld\n",    eve_mem_rd32 (EVE_REG_VSYNC0));
  printf ("EVE_REG_VSYNC1   : %3ld\n",    eve_mem_rd32 (EVE_REG_VSYNC1));

  printf ("EVE_REG_DLSWAP   : %3ld\n",    eve_mem_rd32 (EVE_REG_DLSWAP));
  printf ("EVE_REG_ROTATE   : %3ld\n",    eve_mem_rd32 (EVE_REG_ROTATE));
  printf ("EVE_REG_OUTBITS  : %3ld\n",    eve_mem_rd32 (EVE_REG_OUTBITS));
  printf ("EVE_REG_DITHER   : %3ld\n",    eve_mem_rd32 (EVE_REG_DITHER));

  printf ("EVE_REG_SWIZZLE  : %3ld\n",    eve_mem_rd32 (EVE_REG_SWIZZLE));
  printf ("EVE_REG_CSPREAD  : %3ld\n",    eve_mem_rd32 (EVE_REG_CSPREAD));
  printf ("EVE_REG_PCLK_POL : %3ld\n",    eve_mem_rd32 (EVE_REG_PCLK_POL));
  printf ("EVE_REG_PCLK     : %3ld\n",    eve_mem_rd32 (EVE_REG_PCLK));

  printf ("EVE_REG_GPIO_DIR : 0x%02x\n",  eve_mem_rd8 (EVE_REG_GPIO_DIR));
  printf ("EVE_REG_GPIO     : 0x%02x\n",  eve_mem_rd8 (EVE_REG_GPIO));
  printf ("EVE_REG_GPIOX_DIR: 0x%02x\n",  eve_mem_rd8 (EVE_REG_GPIOX_DIR));
  printf ("EVE_REG_GPIOX    : 0x%02x\n",  eve_mem_rd8 (EVE_REG_GPIOX));

  printf ("EVE_REG_CMD_WRITE: 0x%03lx\n", eve_mem_rd32 (EVE_REG_CMD_WRITE));
  printf ("EVE_REG_CMD_READ : 0x%03lx\n", eve_mem_rd32 (EVE_REG_CMD_READ));
  printf ("EVE_REG_CMD_DL   : 0x%03lx\n", eve_mem_rd32 (EVE_REG_CMD_DL));

  printf ("EVE_REG_CTOUCH_MODE      : 0x%08lx\n", eve_mem_rd32 (EVE_REG_CTOUCH_MODE));
  printf ("EVE_REG_TOUCH_TRANSFORM_A: 0x%08lx\n", eve_mem_rd32 (EVE_REG_TOUCH_TRANSFORM_A));
  printf ("EVE_REG_TOUCH_TRANSFORM_B: 0x%08lx\n", eve_mem_rd32 (EVE_REG_TOUCH_TRANSFORM_B));
  printf ("EVE_REG_TOUCH_TRANSFORM_C: 0x%08lx\n", eve_mem_rd32 (EVE_REG_TOUCH_TRANSFORM_C));
  printf ("EVE_REG_TOUCH_TRANSFORM_D: 0x%08lx\n", eve_mem_rd32 (EVE_REG_TOUCH_TRANSFORM_D));
  printf ("EVE_REG_TOUCH_TRANSFORM_E: 0x%08lx\n", eve_mem_rd32 (EVE_REG_TOUCH_TRANSFORM_E));
  printf ("EVE_REG_TOUCH_TRANSFORM_F: 0x%08lx\n", eve_mem_rd32 (EVE_REG_TOUCH_TRANSFORM_F));
}






/*
  // enable audio amplifier
  eve_mem_wr8  (EVE_REG_GPIOX_DIR, 0x04 | eve_mem_rd8 (EVE_REG_GPIOX_DIR));
  eve_mem_wr8  (EVE_REG_GPIOX    , 0x04 | eve_mem_rd8 (EVE_REG_GPIOX));

  // play sound
  eve_mem_wr8  (EVE_REG_VOL_PB,    0x00);	       // set audio volume
  eve_mem_wr8  (EVE_REG_VOL_SOUND, 0x10);	       // set synthesizer volume

  eve_mem_wr16 (EVE_REG_SOUND, (100<<8) | EVE_BEEPING); // select sound & pitch
  eve_mem_wr8  (EVE_REG_PLAY, 1);                // play sound 
  msleep (700);

  // sound off
  eve_mem_wr16 (EVE_REG_SOUND, 0x60);	           // play silence
  eve_mem_wr8  (EVE_REG_VOL_SOUND, 0x00);	       // synthesizer volume off
  eve_mem_wr8  (EVE_REG_GPIOX, ~0x04 & eve_mem_rd8 (EVE_REG_GPIOX));  // shutdown audio amplifier (remove static noise)
*/
