// file    : usb_hw.c
// author  : ao/rb
// purpose : USB low level routines
// date    : 200101
// last    : 220227
//

#include "../includes.h"

// SETUP packet
USB_stup_req usb_stup_req;
int stup_new  = 0;  

// 'codeline' buffer
uint8_t codeline[16] = {0};

// buffer for CLI data
uint32_t rx_buf[16];
int      rx_len;

// USB Rx data buffer
uint8_t  cli_buf[256];
uint16_t cli_cnt = 0;
uint16_t cli_new = 0;

// USB OTG_FS interrupt handler
void OTG_FS_IRQHandler (void)
{
  uint32_t ints;

  // read & clear USB OTG_FS core interrupts
  ints = USB_GBL->GINTSTS;                       // read interrupt register
  USB_GBL->GINTSTS = ints;                       // & write exact bits to clear
  
  // handle USB reset interrupt
  if (ints & USB_OTG_GINTSTS_USBRST)
  {
    usb_reset ();
    usb_core_reset ();
  }

  // handle OUT Endpoint interrupt - data from host
  if (ints & USB_OTG_GINTSTS_RXFLVL)
  {
    // disable RXFLVL interrupt
    USB_GBL->GINTMSK &= ~USB_OTG_GINTSTS_RXFLVL;

    usb_handle_rx ();

    // enable RXFLVL interrupt
    USB_GBL->GINTMSK |= USB_OTG_GINTSTS_RXFLVL;
  }

  // handle IN Endpoint interrupt - data to Host
  if (ints & USB_OTG_GINTSTS_IEPINT)
  {
    uint32_t daint = USB_DEV->DAINT;             // get all Endpoint interrupts 

    usb_handle_tx (daint);
  }
} 

// init USB OTG_FS hardware
void init_usb (void)
{
  // enable clock
  RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;
  (void) RCC->AHB2ENR;                           // dummy read
  
  USB_GBL->GRSTCTL |= USB_OTG_GRSTCTL_CSRST      // soft reset core
                   |  USB_OTG_GRSTCTL_HSRST;     // soft reset HCLK

  // relax a bit
  usleep (10);
  
  // configure AHB
  USB_GBL->GAHBCFG |= USB_OTG_GAHBCFG_GINT;      // enable ints
  
  // configure USB
  USB_GBL->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD      // force device mode
                   |  (6 << 10);                 // turn around time
  
  // configuer USB OTG_FS core
  USB_GBL->GCCFG |= USB_OTG_GCCFG_PWRDWN         // disable powerdown
                 |  USB_OTG_GCCFG_NOVBUSSENS;    // ignore Vbus
  
  // configure interrupts
  USB_GBL->GINTMSK |= USB_OTG_GINTMSK_USBRST     // reset
                 //|  USB_OTG_GINTMSK_ENUMDNEM   // enumeration done
                 //|  USB_OTG_GINTMSK_ESUSPM     // early suspend
                 //|  USB_OTG_GINTMSK_SOFM       // SOF
                   |   USB_OTG_GINTMSK_IEPINT    // IN interrupt
                   |   USB_OTG_GINTMSK_RXFLVLM   // Receive FIFO non empty
                   |   USB_OTG_GINTMSK_USBSUSPM; // suspend

  // enable interrupt & set priority
  NVIC_EnableIRQ   (OTG_FS_IRQn);
  NVIC_SetPriority (OTG_FS_IRQn, 8);

  usb_connect ();
}

// reset USB OTG_FS
void usb_reset (void)
{
  // flush all queues
  USB_GBL->GRSTCTL |= USB_OTG_GRSTCTL_TXFNUM_4   // flush all Tx FIFOs
                   |  USB_OTG_GRSTCTL_RXFFLSH    // TxFIFO flush
                   |  USB_OTG_GRSTCTL_TXFFLSH;   // RxFIFO flush
  
  // relax a bit
  usleep (2);
  
  // setup interrupts
  USB_DEV->DAINTMSK = (1 << 16) | (1 << 0);      // enable IN0/OUT0 interrupts
  USB_DEV->DOEPMSK  = USB_OTG_DOEPMSK_XFRCM;     // enable OUT Transfer Complete interrupts
  USB_DEV->DIEPMSK  = USB_OTG_DIEPMSK_XFRCM;     // enable IN Transfer Complete interrupts

  // setup FIFOs
  USB_GBL->GRXFSIZ    = 64;                      // RxFIFO depth                           [32-bit words]
  USB_GBL->DIEPTXF0   = (64 << 16) | 0x100;      // TxFIFO depth & start addres Endpoint 0 [32-bit words]
  USB_GBL->DIEPTXF[1] = (64 << 16) | 0x200;      // TxFIFO depth & start addres Endpoint 1 [32-bit words]
  USB_GBL->DIEPTXF[2] = (64 << 16) | 0x300;      // TxFIFO depth & start addres Endpoint 2 [32-bit words]
  USB_GBL->DIEPTXF[3] = (64 << 16) | 0x400;      // TxFIFO depth & start addres Endpoint 3 [32-bit words]

  // setup OTP_FS device configuartion
  USB_DEV->DCFG = 0x02200003;                    // full speed & keep reset value (0x02200000)

  // setup Endpoint 0 to handle SETUP packets
  USB_TX[0].DIEPCTL  = 0;                        // clear control register IN0 Endpoint 
  USB_RX[0].DOEPTSIZ = (3 << 29) |               // set number SETUP packets back-to-back OUT0 Endpoint 
                       (1 << 19) |               // set packet count (is decremented on received packet) OUT0 Endpoint
                       64;                       // trabsfer size OUT0 Endpoint [bytes]
  USB_RX[0].DOEPCTL |= USB_OTG_DOEPCTL_EPENA |   // enable Endpoint OUT0
                       USB_OTG_DOEPCTL_CNAK;     // clear NAK bit

  printf (">USB reset\n");
}

// USB connect
void usb_connect (void)
{
  // check for USB disconnected
  if (USB_DEV->DCTL & USB_OTG_DCTL_SDIS)
  {
    // generate Device connect event
    USB_DEV->DCTL &= ~USB_OTG_DCTL_SDIS;   
  }

  printf (">USB connect\n");
}

// USB disconnect
void usb_disconnect (void)
{
  // generate device disconnect event
  USB_DEV->DCTL |= USB_OTG_DCTL_SDIS;

  printf (">USB disconnect\n");
}

// set USB addres - called from 'usb_core.c'
void usb_set_address (int addr)
{
  uint32_t dat;

  dat = (USB_DEV->DCFG & ~0x7f0) |               // full speed
        (addr << 4);                             // Device address
  USB_DEV->DCFG = dat;

  printf (">Device address: %d\n", addr); 
}

// handle OUT or SETUP Endpoint data - called from ISR
void usb_handle_rx (void)
{
  // always first read RxFIFO Receive Status
  uint32_t rx_stat = USB_GBL->GRXSTSP;

  // ao workaround for bug in core <> needed?
  if (rx_stat >> 28)
  {
    led_red_on ();
    rx_stat = USB_GBL->GRXSTSP;
  }

  // parse RxFIFO Receive Status
  int type  = (rx_stat >> 17) & 0x0f;            // packet status
//int DPID  = (uint8_t)((rx_stat >> 15) & 0x03); // Data PID (DATA0/DATA1/etc)
  int len   = (rx_stat >>  4) & 0x7ff;           // byte count received data packet
  int epnum = (rx_stat >>  0) & 0xf;             // Endpoint number

  // read SETUP transactions - always 8 bytes / 2 words (i.e. 80 06 00 0100 00 4000)
  if (type == 6) 
  {
    // read RxFIFO Endpoint 0
    usb_stup_req.word[0] = *USB_FIFO(0);
    usb_stup_req.word[1] = *USB_FIFO(0);
  
    // clear NAK OUT Endpoint 0
    USB_RX[0].DOEPCTL |= USB_OTG_DOEPCTL_CNAK;
  }

  // flag SETUP transaction completed - parsed in poll loop
  if (type == 4)
  {
    stup_new = 1;  
  }

  // OUT packet received - OUT0: 'codeline' / OUT1: CDC data
  if (type == 2)
  {
    if (epnum == 1) // OUT Endpoint 1, CLI Rx data
    {
      uint32_t *p = rx_buf;
	  
      // set buffer length in bytes
      rx_len = len;
	  
      // & read RxFIFO packet in temp buffer
      while (len > 0)
      {
        *p++ = *USB_FIFO(epnum);
        len -= 4;
      }
    }
    else // read 'codeline' from OUT0 here
    {
      // handle CDC Class Request
      if ((usb_stup_req.request_type == 0x21) && (len == 7))
      {   
        uint32_t *p = (uint32_t *)codeline;

        // & read RxFIFO packet in temp buffer
        while (len > 0)
        {
          *p++ = *USB_FIFO(epnum);
          len -= 4;
        }
      }    
    }
  }

  // OUT transfer completed
  if (type == 3) 
  {
    // collect data from CLI - Endpoint 1 only
    if (epnum == 1)
    {
      usb_handle_cli ();
    }

    // enable OUT Endpoint again
    USB_RX[epnum].DOEPTSIZ = (1 << 19)
                           | 64;
    USB_RX[epnum].DOEPCTL |= USB_OTG_DOEPCTL_EPENA 
                          |  USB_OTG_DOEPCTL_CNAK;
  }
}

// handle IN Endpoint data - called from ISR
void usb_handle_tx (uint32_t daint)
{
  uint32_t diep;    

  // read & clear interrupts  
  if (daint & (1 << 0)) 
  {
    diep = USB_TX[0].DIEPINT;    
    USB_TX[0].DIEPINT = diep;
  }

  if (daint & (1 << 1)) 
  {
    diep = USB_TX[1].DIEPINT;    
    USB_TX[1].DIEPINT = diep;
  }

  if (daint & (1 << 2)) 
  {
    diep = USB_TX[2].DIEPINT;    
    USB_TX[2].DIEPINT = diep;
  }
}

// handle CLI data
void usb_handle_cli (void)
{
  uint8_t ch;
  uint8_t *p = (uint8_t *)rx_buf;

  for (int i = 0; i < rx_len; i++)
  {
    ch = p[i];

    if ((ch == '\r') | (ch == '\n')) 
    {
      cli_buf[cli_cnt] = '\0';
      cli_new = 1;
      cli_cnt = 0;
    }
    else
    {
      if (cli_cnt < 256)
        cli_buf[cli_cnt++] = ch;
    }
  }
}

// open OUT Endpoint - Bulk
void usb_open_rx (int epnum)
{
  // setup active OUT Endpoint
  USB_RX[epnum].DOEPTSIZ = (1 << 19) | 64;            // number USB packets & transfer size (== #packets*USB packet size)
  USB_RX[epnum].DOEPCTL |= USB_OTG_DOEPCTL_SD0PID     // set DATA0 PID
                        |  (2 << 18)                  // set Enpoint type to Bulk
                        |  64                         // maximum packet size
                        |  USB_OTG_DIEPCTL_USBAEP;    // USB active Endpoint 
  USB_RX[epnum].DOEPCTL |= USB_OTG_DOEPCTL_EPENA      // enable Endpoint
                        |  USB_OTG_DOEPCTL_CNAK;      // clear NAK
}

// open IN Endpoint - Bulk/Interrupt
void usb_open_tx (int epnum)
{
  int type = 2;

  // IN Endpoint 2 is interrupt
  if (epnum == 2)
    type = 3;

  // setup active IN Endpoint
  USB_TX[epnum].DIEPCTL |= USB_OTG_DIEPCTL_SD0PID     // set DATA0 PID
                        |  (type << 18)               // set Enpoint type to Bulk or Interrupt
                        |  64                         // maximum packet size
                        |  (epnum << 22)              // set TxFIFO number
                        |  USB_OTG_DIEPCTL_SNAK       // set NAK 
                        |  USB_OTG_DIEPCTL_USBAEP;    // USB active Endpoint 

  // enable interrupt on IN Endpoint event
  USB_DEV->DAINTMSK |= (1 << epnum);  
}

// set STALL handshake
void usb_set_stall (int epadr)
{
  int epnum = epadr & 0x03;  // EP0..EP3 only

  if (epadr & 0x80)
  {
    // set stall IN Enpoint
    USB_TX[epnum].DIEPCTL |= USB_OTG_DIEPCTL_STALL;
  }
  else
  {
    // set stall OUT Endpoint
    USB_RX[epnum].DOEPCTL |= USB_OTG_DOEPCTL_STALL;
  }
}

// clear STALL handshake
void usb_clr_stall (int epadr)
{
  int epnum = epadr & 0x03;  // EP0..EP3 only

  if (epadr & 0x80)
  {
    // clear stall IN Enpoint
    USB_TX[epnum].DIEPCTL &= ~USB_OTG_DIEPCTL_STALL;
  }
  else
  {
    // clear stall OUT Endpoint
    USB_RX[epnum].DOEPCTL &= ~USB_OTG_DOEPCTL_STALL;
  }
}

// check Endpoint stall status
int usb_is_stalled (int epadr)
{
  int epnum = epadr & 0x03;  // EP0..EP3 only

  if(epadr & 0x80)
  {
    // OUT Endpoint
    return (USB_TX[epnum].DIEPCTL & USB_OTG_DIEPCTL_STALL);
  }
  else
  {
    // OUT Endpoint
    return (USB_RX[epnum].DOEPCTL & USB_OTG_DOEPCTL_STALL);
  }
}

// send data to Host - polled version
void usb_transmit (int epnum, void *buf, int len)
{
  int cnt = 0;

  while (len > 64)
  {
    int words = 16;   // full packet
  
    uint32_t *wptr = (uint32_t *)&buf[cnt];
  
    // set packet count & packet length
    USB_TX[epnum].DIEPTSIZ = (1 << 19) | 64;
    
    // enable IN Endpoint 
    USB_TX[epnum].DIEPCTL |= (USB_OTG_DIEPCTL_CNAK | USB_OTG_DIEPCTL_EPENA);
    
    // send packet to Host
   //uint32_t free = USB_TX[epnum].DTXFSTS;
    
   //while (((free & 0xffff) >= words) && (bytes > 0))
    while (words > 0)
    {
      *USB_FIFO(epnum) = *wptr++;
      words--;
    }

    len -= 64;
    cnt += 64;  

    // wait Tx packet sent <> time out here!?
    while (USB_TX[epnum].DIEPTSIZ & (1 << 19))
      usleep (10);
  }

  // send short packet
  if (len > 0)
  {
    int words = (len + 3)/4;
  
    uint32_t *wptr = (uint32_t *)&buf[cnt];
  
    // set packet count & packet length
    USB_TX[epnum].DIEPTSIZ = (1 << 19) | len;
    
    // enable IN Endpoint 
    USB_TX[epnum].DIEPCTL |= (USB_OTG_DIEPCTL_CNAK | USB_OTG_DIEPCTL_EPENA);
  
    while (words > 0)
    {
      *USB_FIFO(epnum) = *wptr++;
      words--;
    }
  
    // wait Tx packet sent <> time out here!?
    while (USB_TX[epnum].DIEPTSIZ & (1 << 19))
      usleep (10);
  }
}

// dump SETUP packet
void dump_stup_req (void)
{
  printf ("STUP: ");
  printf ("t 0x%02x ", usb_stup_req.request_type);
  printf ("r 0x%02x ", usb_stup_req.request);
  printf ("v 0x%04x ", usb_stup_req.value);
  printf ("i 0x%04x ", usb_stup_req.index);
  printf ("l %d\n"   , usb_stup_req.length);
}


