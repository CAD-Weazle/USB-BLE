// file    : usb_hw.h
// author  : rb
// purpose : USB low level routines
// date    : 200101
// last    : 220227
//

#ifndef _USB_HW_H_
#define _USB_HW_H_

// SETUP packet buffer
typedef struct
{
  union
  {
    uint32_t word[2];

    struct
    {
      uint8_t  request_type;
      uint8_t  request;
      uint16_t value;
      uint16_t index;
      uint16_t length;
    };
  };
} USB_stup_req;

// -- globals
extern USB_stup_req usb_stup_req;
extern int stup_new;
extern uint8_t codeline[16];

extern uint8_t  cli_buf[256];
extern uint16_t cli_cnt;
extern uint16_t cli_new;

// -- prototypes
void poll_usb (void);

void OTG_FS_IRQHandler (void);

void init_usb (void);

void usb_reset (void);
void usb_connect (void);
void usb_disconnect (void);

void usb_set_address (int addr);

void usb_handle_rx (void);
void usb_handle_tx (uint32_t daint);
void usb_handle_cli (void);

void usb_open_rx (int epnum);
void usb_open_tx (int epnum);

void usb_set_stall (int epnum);
void usb_clr_stall (int epnum);
int usb_is_stalled (int epnum);

void usb_transmit (int epnum, void *buf, int len);

void dump_stup_req (void);

#endif