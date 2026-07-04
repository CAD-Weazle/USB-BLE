// file    : cli.c
// author  : rb
// purpose : command line interface SunWeb Master
// date    : 081104
// last    : 200523
//
// note    : updated version, doesn't need 'type.h' & 'types.h' anymore
//         : added 'rb_atoh'

#include "includes.h"
#include <ctype.h>
#include <strings.h>

// globals 
uint8_t pass_thru = 0;    // CLI/BLE pass thru flag

uint8_t co2_log = 0;

//uchar buf_len;
uint8_t buf_len;
char    buf[INBUF_LEN];

int   argc;
char *argv[10];

// display terminal prompt
void prompt (void)
{ 
  // normal prompt
  printf (">");
}

// display terminal prompt with extra '\n'
void first_prompt (void)
{
  printf ("\n>");
}

// convert ASCII string to integer
int rb_atoi (char *s)
{
  int i = 0, n, sign = 1;
  
  // check for negative number
  if (s[0] == '-')
  {
    sign = -1;
    i++;
  }

  // look for numbers & calculate integer
  for (n = 0; s[i] >= '0' && s[i] <= '9'; i++)
    n = 10 * n + s[i] - '0';
  
  return (sign * n);
}


// convert ASCII string to long
long rb_atol (char *s)
{
  int i = 0, sign = 1;
  long n;
  
  // check for negative number
  if (s[0] == '-')
  {
    sign = -1;
    i++;
  }
  
  // look for numbers & calculate long
  for( n = 0; s[i] >= '0' && s[i] <= '9'; i++ )
    n = 10 * n + s[i] - '0';
  
  return (sign * n);
}

// convert ASCII string to hex int
uint32_t rb_atoh (char *s)
{
  int i = 0;
  int digit, lcase;
  uint32_t n;
  
  // look for numbers / lower case letters & calculate integer
  for (n = 0; ((digit = (s[i] >= '0' && s[i] <= '9')) || 
               (lcase = (s[i] >= 'a' && s[i] <= 'f')) ||
                        (s[i] >= 'A' && s[i] <= 'F')); i++)
  {
    n *= 16;

    if (digit)
      n += s[i] - '0';
    else if (lcase)
      n += 10 + (s[i] - 'a');
    else      
      n += 10 + (s[i] - 'A');
  }

  return (n);
}

// skip whitespace
char *skipws (char *line)
{
  if( line != NULL )
  {
    while( *line == ' ' )
      line++;
  }

  return( line );
}

// split up command line in words seperated by whitespaces
int split_line (char *line, char **words)
{
  int n = 0;
  char *sep = line;
  
  while ((line = skipws(sep)) != NULL && *line)
  {
    sep = strchr (line, ' ');

    if (sep)
      *sep++ = 0;

    words[n++] = line;
  }

  return (n);
}

// poll command line interface USART2
void poll_cli (void)
{
  int c = uart1_getc ();

  // test for Rx buffer empty
  if (c < 0)
    return;

  // commandline to BLE module pass thru
  if (c == '~')
  {
    if (pass_thru == 0)
    {
      pass_thru = 1;

      printf ("\E[35m%s\E[30m\n", "enable pass-thru");
    }
    else
    {
      pass_thru = 0;

      printf ("\E[35m%s\E[30m\n", "leave pass-thru");
    }
  }

  // handle CLI characters
  if (pass_thru)
  {
    // send CLI data to BLE
    uart4_putc (c);
  }
  else
  {
    // add byte to buffer & parse
    if (( c == '\r' ) | ( c == '\n' ))   // check for enter to end string
    {
      printf ("\n");                     // print newline

      buf[buf_len] = '\0';               // '\0' terminate buffer
      buf_len      = 0;                  // reset pointer
    
      argc = split_line (buf, argv);     // parse command
      parse_line (argc, argv);          

      prompt ();                         // display new prompt
    }
    else if (c == '\b' || c == 0x7f)     // check for backspace and delete
    {
      if( buf_len > 0 )
        buf_len--;
    }
    else if (isprint(c) && buf_len < INBUF_LEN)
      buf[buf_len++] = c;
  }
}

// parse command line
void parse_line (int argc, char **argv)
{
  if (argc)
  {
    /////////// remove later //////////////////

    // ** Feasycom BLE commands
    if (!strcmp (argv[0], "at"))
      printf4 ("AT\n");

    if (!strcmp (argv[0], "rst"))
      printf4 ("AT+REBOOT\n");

    if (!strcmp (argv[0], "hrst"))
      ble_reset ();

    if (!strcmp (argv[0], "disc"))
      printf4 ("AT+DISC\n");

    if (!strcmp (argv[0], "ver"))
      printf4 ("AT+VER\n");

    if (!strcmp (argv[0], "adr"))
      printf4 ("AT+ADDR\n");

    if (!strcmp (argv[0], "name"))
      printf4 ("AT+NAME\n");

    if (!strcmp (argv[0], "baud"))
      printf4 ("AT+BAUD\n");

    if (!strcmp (argv[0], "scan"))
      printf4 ("AT+SCAN=1,48\n");

    if (!strcmp (argv[0], "role"))
      printf4 ("AT+ROLE\n");

    if (!strcmp (argv[0], "con"))
      ble_connect ("DAC8F9C0EEB9");

    // pump control 
    if (!strcmp (argv[0], "pon"))
    {
      pump_on ();
    }

    if (!strcmp (argv[0], "poff"))
    {
      pump_off ();

      pwm_set (0, 0);
      pwm_set (1, 0);
      pwm_set (2, 0);
    }

    if (!strcmp (argv[0], "pget"))
    {
      pump_get_start ();
    }

    // dump screen in PPM format to serial port <> fix this
    if (!strcmp (argv[0], "prscr"))
    {
      tft_get_snapshot (EVE_RAM_GP);
      tft_ppm_snapshot (EVE_RAM_GP, EVE_HSIZE, EVE_VSIZE);
    //tft_ppm_snapshot (EVE_RAM_GP, 800, 480);
    }

    ///////////////// UITZOEK0N!1!1 ///////////////////////

    // release all SunWeb Devices
    if (!strcmp (argv[0], "ra"))
    {
      sw_release_all ();
    }

    // probe all SunWeb Devices
    if (!strcmp (argv[0], "pa"))
    {
      sw_probe_all ();
    }

    // ** busmap commands
    // add SunWeb Device to busmap
    if (!strcmp (argv[0], "bm_add"))
    {
      if (argc == 3)
      {
        // add SunWeb Device to busmap
        uint8_t box = rb_atoi (argv[1]);
        uint8_t id  = (uint8_t)rb_atoh (argv[2]);

        ee_fill_busmap (box, id);        
      }
    }

    // dump busmap
    if (!strcmp (argv[0], "bm_dump"))
    {
      ee_dump_busmap ();
    }

    // dump growbox array
    if (!strcmp (argv[0], "gb_dump"))
    {
      sw_dump_gboxes ();
    }

    // erase busmap EEPROM
    if (!strcmp (argv[0], "bm_erase"))
    {
    //flash_erase_busmap ();
    }

    // check busmap present
    if (!strcmp (argv[0], "bm_present"))
    {
    //flash_busmap_present ();
    }

    // dump external EEPROM holding calibration parameters
    if (!strcmp (argv[0], "ee_dump"))
    {
      ee_dump ();
    }

    // set DS3234 time
    if (!strcmp (argv[0], "ds_set"))
    {
      if (argc == 4)
      {
        // set time
        uint8_t hrs = rb_atoi (argv[1]);
        uint8_t min = rb_atoi (argv[2]);
        uint8_t sec = rb_atoi (argv[3]);

        ds3234_set_time (hrs, min, sec);
        ds3234_dump_time (1);
      }
    }

    // get DS3234 time
    if (!strcmp (argv[0], "ds_get"))
    {
      ds3234_dump_time (1);
    }

    // get/set DS3234 date
    if (!strcmp (argv[0], "ds_date"))
    {
      if (argc == 4)
      {
        // set time
        uint8_t yrs = rb_atoi (argv[1]);
        uint8_t mnt = rb_atoi (argv[2]);
        uint8_t day = rb_atoi (argv[3]);

        ds3234_set_date (yrs, mnt, day);
      }

      ds3234_dump_date ();
    }

    // get timestamp
    if (!strcmp (argv[0], "ds_stamp"))
    {
      ds3234_timestamp ();
    }

    // log time & date
    if (!strcmp (argv[0], "clock"))
    {
      ds3234_start_log ();
    }

    
    // ** pH calibration commands

    // start calibration pH sensor (pH sensors: 1..3)
    if (!strcmp (argv[0], "ph_cal"))
    {
      if (argc == 2)
      {
        uint8_t sensor = rb_atoi (argv[1]);
        ph_cal_sensor (sensor - 1);
      }
    }

    // set pH calibration data <> needed?
    if (!strcmp (argv[0], "ph_set"))
    {
      if (argc == 4)
      {
        uint8_t  sensor    = rb_atoi (argv[1]);
        uint8_t  cal_point = rb_atoi (argv[2]);
        uint16_t cal_dat   = rb_atoi (argv[3]);
        ee_set_phcal (sensor, cal_point, cal_dat);
      }
    }

    // dump pH calibration data
    if (!strcmp (argv[0], "ph_dump"))
    {
      ee_dump_phcal ();
    }

    // start logging pH sensors
    if (!strcmp (argv[0], "ph_log"))
    {
      ph_log_start ();
    }

    // ** EC calibration commands

    // start calibration EC sensor  (EC sensors: 1..3)
    if (!strcmp (argv[0], "ec_cal"))
    {
      if (argc == 2)
      {
        uint8_t sensor = rb_atoi (argv[1]);
        ec_cal_sensor (sensor - 1);
      }
    }

    // dump EC calibration data
    if (!strcmp (argv[0], "ec_dump"))
    {
      ee_dump_eccal ();
    }

    // start logging EC sensors
    if (!strcmp (argv[0], "ec_log"))
    {
      ec_log_start ();
    }

    // ** NTC calibration commands

    // write offset error NTC sensor (embedded in EC sensor)
    if (!strcmp (argv[0], "ntc_err"))
    {
      // write new offset error &
      if (argc == 3)
      {
        uint8_t sensor = rb_atoi (argv[1]);
        int16_t err = rb_atoi (argv[2]);
        ee_set_ntccal (sensor, err);   // store iN EEPROM
        ntc_get_error ();              // update state
      }

      // dump & update state
      ntc_dump_error ();
    }

    // dump NTC calibration data
    if (!strcmp (argv[0], "ntc_dump"))
    {
      ee_dump_ntccal ();
    }

    // start logging NTC sensors
    if (!strcmp (argv[0], "ntc_log"))
    {
    //ntc_log_start ();
    }





    ////////////////////////////////////////////////////////
    //
    // ** SunWeb Commands
    //
    ////////////////////////////////////////////////////////

    // set SunWeb Device Bus ID
    if (!strcmp (argv[0], "sw_setid"))
    {
      uint8_t badr = (uint8_t)(rb_atoh (argv[1]));

      sw_set_busid (badr);
    }

    // restart network
    if (!strcmp (argv[0], "sw_probe"))
    {
      sw_start_probe ();
    }

    // release SunWeb Device
    if (!strcmp (argv[0], "sw_release"))
    {
      uint8_t badr = (uint8_t)(rb_atoh (argv[1]));

      sw_release (badr);
    }

    // set SunWeb Device time
    if (!strcmp (argv[0], "sw_rtcset"))
    {
      if (argc == 2)
      {
        uint8_t badr = (uint8_t)(rb_atoh (argv[1]));

        sw_set_clock (badr);
      }
      else
      {
        sw_set_clock (SW_ID_BROADCAST);
      }
    }

    // sync SimSun Devices to midnight
    if (!strcmp (argv[0], "sw_rtcsync"))
    {
      if (argc == 2)
      {
        uint8_t badr = (uint8_t)rb_atoh (argv[1]);

        sw_sync_clock (badr);
      }
      else
      {
        sw_sync_clock (SW_ID_BROADCAST);
      }
    }

    ////////////////////////////////////////////////////////
    //
    // ** SimSun Commands
    //
    ////////////////////////////////////////////////////////

    // set SimSun dawn time
    if (!strcmp (argv[0], "ss_dawn"))
    {
      if (argc == 4)
      {
        uint8_t hrs = rb_atoi (argv[1]);
        uint8_t min = rb_atoi (argv[2]);
        uint8_t sec = rb_atoi (argv[3]);

        ss_set_dawn (SW_ID_BCASTLEDS, hrs*60*60 + min*60 + sec);
      }
      else if (argc == 5)
      {
        uint8_t badr = (uint8_t)rb_atoh (argv[1]);
        uint8_t hrs  = rb_atoi (argv[2]);
        uint8_t min  = rb_atoi (argv[3]);
        uint8_t sec  = rb_atoi (argv[4]);

        ss_set_dawn (badr, hrs*60*60 + min*60 + sec);
      }
    }

    // set SimSun dusk time
    if (!strcmp (argv[0], "ss_dusk"))
    {
      if (argc == 4)
      {
        uint8_t hrs = rb_atoi (argv[1]);
        uint8_t min = rb_atoi (argv[2]);
        uint8_t sec = rb_atoi (argv[3]);

        ss_set_dusk (SW_ID_BCASTLEDS, hrs*60*60 + min*60 + sec);
      }
      else if (argc == 5)
      {
        uint8_t badr = (uint8_t)rb_atoh (argv[1]);
        uint8_t hrs  = rb_atoi (argv[2]);
        uint8_t min  = rb_atoi (argv[3]);
        uint8_t sec  = rb_atoi (argv[4]);

        ss_set_dusk (badr, hrs*60*60 + min*60 + sec);
      }
    }

    // set SimSun light intensity Deep Red LEDs [%PWM]
    if (!strcmp (argv[0], "ss_dred"))
    {      
      if (argc == 2)

      {
        // multicast
        uint8_t val = rb_atoi (argv[1]);   

        ss_set_dred (SW_ID_BCASTLEDS, val);
      }
      else if (argc == 3)
      {
        // addressed
        uint8_t badr = (uint8_t)rb_atoh (argv[1]);
        uint8_t val  = rb_atoi (argv[2]);

        ss_set_dred (badr, val);
      }
    }

    // set SimSun light intensity Royal Blue LEDs [%PWM]
    if (!strcmp (argv[0], "ss_rblu"))
    {
      if (argc == 2)
      {
        // multicast
        uint8_t val = rb_atoi (argv[1]);

        ss_set_rblu (SW_ID_BCASTLEDS, val);
      }
      else if (argc == 3)
      {
        // addressed
        uint8_t badr = (uint8_t)rb_atoh (argv[1]);
        uint8_t val  = rb_atoi (argv[2]);

        ss_set_rblu (badr, val);
      }
    }

    // set SimSun light intensity Far Red LEDs [on/off]
    if (!strcmp (argv[0], "ss_fred"))
    {
      if (argc == 2)
      {
        // multicast
        uint8_t val = rb_atoi (argv[1]);

        ss_set_fred (SW_ID_BCASTLEDS, val);
      }
      else if (argc == 3)
      {
        // addressed
        uint8_t badr = (uint8_t)rb_atoh (argv[1]);
        uint8_t val  = rb_atoi (argv[2]);

        ss_set_fred (badr, val);
      }
    }

    // set SimSun Deep Red LEDs light flux [umol/m^2*s]
  //if (!strcmp (argv[0], "ss_dred_mol"))
    if (!strcmp (argv[0], "rmol"))
    {
      if (argc == 2)
      {
        // multicast
        uint16_t flux = rb_atoi (argv[1]);

        ss_mol_dred (SW_ID_BCASTLEDS, flux);
      }
      else if (argc == 3)
      {
        // addressed
        uint8_t  badr = (uint8_t)rb_atoh (argv[1]);
        uint16_t flux = rb_atoi (argv[2]);

        ss_mol_dred (badr, flux);
      }
    }

  //if (!strcmp (argv[0], "ss_rblu_mol"))
    if (!strcmp (argv[0], "bmol"))
    {
      if (argc == 2)
      {
        // multicast
        uint16_t flux = rb_atoi (argv[1]);

        ss_mol_rblu (SW_ID_BCASTLEDS, flux);
      }
      else if (argc == 3)
      {
        // addressed
        uint8_t  badr = (uint8_t)rb_atoh (argv[1]);
        uint16_t flux = rb_atoi (argv[2]);

        ss_mol_rblu (badr, flux);
      }
    }

    // set maximum light intensity Deep Red LEDs
    if (!strcmp (argv[0], "ss_dred_max"))
    {
      if (argc == 2)
      {
        // multicast
        uint8_t val = rb_atoi (argv[1]);

        ss_max_dred (SW_ID_BCASTLEDS, val);
      }
      else if (argc == 3)
      {
        // addressed
        uint8_t badr = (uint8_t)rb_atoh (argv[1]);
        uint8_t val  = rb_atoi (argv[2]);

        ss_max_dred (badr, val);
      }
    }

    // set maximum light intensity Royal Blue LEDs
    if (!strcmp (argv[0], "ss_rblu_max"))
    {
      if (argc == 2)
      {
        // multicast
        uint8_t val = rb_atoi (argv[1]);

        ss_max_rblu (SW_ID_BCASTLEDS, val);
      }
      else if (argc == 3)
      {
        // addressed
        uint8_t badr = (uint8_t)rb_atoh (argv[1]);
        uint8_t val  = rb_atoi (argv[2]);

        ss_max_rblu (badr, val);
      }
    }

    // start SimSun light program
    if (!strcmp (argv[0], "ss_start"))
    {
      if (argc == 1)
      {
        // multicast
        ss_simsun_start (SW_ID_BCASTLEDS);
      }
      else if (argc == 2)
      {
        // addressed
        uint8_t badr = (uint8_t)rb_atoh (argv[1]);

        ss_simsun_start (badr);
      }
    }

    // stop SimSun light program
    if (!strcmp (argv[0], "ss_stop"))
    {
      if (argc == 1)
      {
        // multicast
        ss_simsun_stop (SW_ID_BCASTLEDS);
      }
      else if (argc == 2)
      {
        // addressed
        uint8_t badr = (uint8_t)rb_atoh (argv[1]);

        ss_simsun_stop (badr);
      }
    }

    // get temperature SunWeb Device
    if (!strcmp (argv[0], "ss_getstate"))
    {
      if (argc == 2)
      {
        uint8_t id = (uint8_t)rb_atoh (argv[1]);

        ss_get_state (id);
      }
    }

    // write NTC calibration parameter
    if (!strcmp (argv[0], "ss_ntcerr"))
    {
      if (argc == 3)
      {
        // single device
        uint8_t badr = rb_atoh (argv[1]);
        int16_t err  = rb_atoi (argv[2]); // [oC*10]

        ss_set_ntc (badr, err);
      }
    }

    ////////////////////////////////////////////////////////
    //
    // ** HotBed Commands
    //
    ////////////////////////////////////////////////////////

    // set HotBed temperature [10*oC]
    if (!strcmp (argv[0], "hb_set"))
    {
      if (argc == 2)
      {
        // multicast
        uint16_t temp = rb_atoi (argv[1]);
        hb_set_temp (SW_ID_BCASTHEAT, temp);
      }
      else if (argc == 3)
      {
        // single device
        uint8_t  badr = rb_atoh (argv[1]);
        uint16_t temp = rb_atoi (argv[2]);
        hb_set_temp (badr, temp*10);
      }
    }

    // get HotBed temperature
    if (!strcmp (argv[0], "hb_get"))
    {
      if (argc == 2)
      {
        uint8_t  badr = rb_atoh (argv[1]);
        hb_get_temp (badr);
      }
    }

    // start HotBed PID temperature control
    if (!strcmp (argv[0], "hb_start"))
    {
      if (argc == 1)
      {
        // multicast
        hb_start_pid (SW_ID_BCASTHEAT);
      }
      else if (argc == 2)
      {
        // addressed
        uint8_t badr = rb_atoh (argv[1]);
        hb_start_pid (badr);
      }
    }

    // stop HotBed PID temperature control
    if (!strcmp (argv[0], "hb_stop"))
    {
      if (argc == 1)
      {
        // multicast
        hb_stop_pid (SW_ID_BCASTHEAT);
      }
    }

    // set HotBed bottom LEDs intensity
    if (!strcmp (argv[0], "hb_led"))
    {
      if (argc == 2)
      {
        // multicast
        uint8_t pwm = rb_atoi (argv[1]);
        hb_set_led (SW_ID_BCASTHEAT, pwm);
      }
      else if (argc == 3)
      {
        // single device
        uint8_t badr = rb_atoh (argv[1]);
        uint8_t pwm  = rb_atoi (argv[2]);
        hb_set_led (badr, pwm);
      }
    }

    // set HotBed fan speed 
    if (!strcmp (argv[0], "hb_fan"))
    {
      if (argc == 2)
      {
        // multicast
        uint8_t pwm = rb_atoi (argv[1]);
        hb_set_fan (SW_ID_BCASTHEAT, pwm);
      }
      else if (argc == 3)
      {
        // single device
        uint8_t badr = rb_atoh (argv[1]);
        uint8_t pwm  = rb_atoi (argv[2]);
        hb_set_fan (badr, pwm);
      }
    }

    // start/stop HotBed bottom LEDs show
    if (!strcmp (argv[0], "hb_show"))
    {
      if (argc == 2)
      {
        if (!strcmp (argv[1], "on"))
          hb_start_show (SW_ID_BCASTHEAT);

        if (!strcmp (argv[1], "off"))
          hb_stop_show (SW_ID_BCASTHEAT);
      }
    }

    // get all Pt100 temperatures (for offset error calibration only)
    if (!strcmp (argv[0], "hb_pt100"))
    {
      uint8_t id = (uint8_t)(rb_atoh (argv[1]));

      hb_get_rtd (id);
    }

    // write Pt100 calibration parameter
    if (!strcmp (argv[0], "hb_pterr"))
    {
      if (argc == 4)
      {
        // single device
        uint8_t badr = rb_atoh (argv[1]);
        uint8_t num  = rb_atoi (argv[2]);
        int16_t err  = rb_atoi (argv[3]); // [oC*10]

        hb_set_rtd (badr, num, err);
      }
    }

    ////////////////////////////////////////////////////////
    //
    // ** RN4871 BLE module commands <> test
    //
    ////////////////////////////////////////////////////////

    // reset module
    if (!strcmp (argv[0], "b_rst"))
    {
      printf ("BLE: reset\n");
      ble_reset ();
    }

    // start scan for BLE devices
    if (!strcmp (argv[0], "b_scan"))
    {
    //ble_scan_start ();
    }

    // stop scanning for BLE devices
    if (!strcmp (argv[0], "b_stop"))
    {
    //ble_scan_stop ();
    }

    // dump info BLE device
    if (!strcmp (argv[0], "b_dump"))
    {
      printf ("BLE: dump info\n");
      msleep (200);

      printf4 ("D\r");
    }

    // set name BLE device
    if (!strcmp (argv[0], "b_name"))
    {
      printf ("BLE: set device name\n");
      msleep (200);

      printf4 ("SN,SunWeb_Master\r");
    }

    // connect to BLE Jig 
    if (!strcmp (argv[0], "b_conn"))
    {
      printf ("BLE: connect with BLE Jig\n");
      msleep (200);

      printf4 ("C,0,801F12BE1D35\r");
    }

    // dump connection retry stats
    if (!strcmp (argv[0], "cstat"))
      ble_dump_stats ();

    ////////////////////////////////////////////////////////
    //
    // ** misc. Commands
    //
    ////////////////////////////////////////////////////////

    // set log data display interval [s]
    if (!strcmp (argv[0], "log_int"))
    {
      uint16_t dat = rb_atoi (argv[1]);
      master_log_interval (dat);
    }

    // start logging SunWeb Master & SunWeb Devices state
    if (!strcmp (argv[0], "log"))
    {
      master_log_start ();
    }

    // stop all
    if (!strcmp (argv[0], "q"))
    {
      ds3234_stop_log ();  // <> obselete?
      ph_log_stop ();      // <> obselete?
      ec_log_stop ();      // <> obselete?

      master_log_stop ();
    }

    // dump version information
    if (!strcmp (argv[0], "version"))
      version ();

    // dump help
    if (!strcmp (argv[0], "help"))
      help ();

    // <> debug only
    if (!strcmp (argv[0], "gaan"))
    {
      gaan_met_die_banaan ();
    }

  }   
} 
    
// dump version info
void version (void)
{
  printf ("#- Feasycom BLE Central - " VERSION " -\n");
}

// dump commands
void help (void)
{
  printf ("-- SunWeb Master commands:\n");
  printf ("pa                        - probe bus for SunWeb Devices\n");
  printf ("ra                        - release all SunWeb Devices\n");
  printf ("da                        - dump SunWeb Devices found\n");
  printf ("bm_add <box> <id>         - add SunWeb Device to busmap\n");
  printf ("bm_dump                   - dump busmap\n");
  printf ("bm_erase                  - erase busmap EEPROM\n");
  printf ("bm_present                - check busmap present\n");
  printf ("gb_dump                   - dump growbox status\n");
  printf ("ds_set                    - set DS3234 time\n");
  printf ("ds_get                    - get DS3234 time\n");
  printf ("ds_date                   - set DS3234 date\n");
  printf ("ds_stamp                  - get timestamp\n");
  printf ("clock                     - log time & date <> needed?\n");
  printf ("ph_cal <num>              - start calibration pH sensor [1..3]\n");
  printf ("ph_set                    - set pH calibration data <> needed?\n");
  printf ("ph_dump                   - dump pH calibration data\n");
  printf ("ph_log                    - start logging pH sensors\n");
  printf ("ec_cal <num>              - start calibration EC sensor [1..3]\n");
  printf ("ec_dump                   - dump EC calibration data\n");
  printf ("ec_log                    - start logging EC sensors\n");
  printf ("ntc_err                   - write offset error NTC sensor (embedded in EC sensor)\n");
  printf ("ntc_dump                  - dump offset error NTC sensor  (embedded in EC sensor)\n");
  printf ("ntc_log                   - start logging NTC sensors     (embedded in EC sensor)\n");

  printf ("\n-- SimSun commands:\n");
  printf ("ss_dred <id> <num>        - set maximum light intensity Deep Red LEDs [%%PWM]\n");
  printf ("ss_rblu <id> <num>        - set maximum light intensity Royal Blu LEDs [%%PWM]\n");
  printf ("ss_clock                  - set SimSun time\n");
  printf ("ss_dawn <hh> <mm> <ss>    - set SimSun dawn\n");
  printf ("ss_dusk <hh> <mm> <ss>    - set SimSun dusk\n");
  printf ("ss_start <id>             - start SimSun light program\n");
  printf ("ss_stop <id>              - start SimSun light program\n");
  printf ("ss_ntcerr <id> <num>      - write NTC calibration parameter [10*oC]\n");

  printf ("\n-- HotBed commands:\n");
  printf ("hb_set [id] <temp>        - set HotBed temperature [10*oC]\n");
  printf ("hb_get <id>               - get HotBed temperature\n");
  printf ("hb_start                  - start HotBed PID temperature control  (multicast)\n");
  printf ("hb_stop                   - stop HotBed PID temperature control   (multicast)\n");
  printf ("hb_led [id] <pwm>         - set HotBed bottom LEDs intensity\n");
  printf ("hb_fan [id] <pwm>         - set HotBed fan speed\n");
  printf ("hb_show <on|off>          - start/stop HotBed bottom LEDs show    (multicast)\n");
  printf ("hb_pt100 [id]             - get HotBed Pt100 temperatures\n");
  printf ("hb_pterr <id> <num> <err> - write HotBed Pt100 offset error [10*oC]\n");

  printf ("\n-- misc. commands:\n");
  printf ("log_int                   - set log data display interval [s]");
  printf ("log                       - log SunWeb Master & Devicesn");
  printf ("q                         - quit logging & stuff\n");
  printf ("version                   - dump version information\n");
  printf ("help                      - dump this help\n");
}








