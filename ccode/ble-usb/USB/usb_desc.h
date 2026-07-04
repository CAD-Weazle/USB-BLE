// file    : usb_desc.h
// author  : st/ao/rb
// purpose : header file for usb_desc.c
// date    : 200101
// last    : 220227
//

#ifndef _USB_DESC_H_
#define _USB_DESC_H_

#include "../includes.h"

// -- defines 
#define VID                                0x1d6B  // vendor ID
#define PID                                0x4255  // UB (USB-BLE)

#define USBD_MAX_NUM_INTERFACES                 1
#define USBD_MAX_NUM_CONFIGURATION              1
#define USBD_MAX_STR_DESC_SIZ               0x100

// CDC defs 
#define CDC_SEND_ENCAPSULATED_COMMAND        0x00
#define CDC_GET_ENCAPSULATED_RESPONSE        0x01
#define CDC_SET_COMM_FEATURE                 0x02
#define CDC_GET_COMM_FEATURE                 0x03
#define CDC_CLEAR_COMM_FEATURE               0x04
#define CDC_SET_LINE_CODING                  0x20
#define CDC_GET_LINE_CODING                  0x21
#define CDC_SET_CONTROL_LINE_STATE           0x22
#define CDC_SEND_BREAK                       0x23

// -- globals
struct string_desc
{
  uint8_t  length;
  uint8_t  type;
  uint16_t value[];
};

extern struct string_desc manuf_string;
extern struct string_desc product_string;

// -- protoypes
void usb_if_class_req (USB_stup_req *setup);

uint8_t *GetDeviceDescriptor    (int *length);
uint8_t *GetConfigDescriptor    (int *length);
uint8_t *GetLangIDStrDescriptor (int *length);

char *GetProductStrDescriptor   (void);
char *GetSerialStrDescriptor    (void);
char *GetConfigStrDescriptor    (void);
char *GetInterfaceStrDescriptor (void);

void usb_set_configuration   (int cfgidx);
void usb_clear_configuration (int cfgidx);

#endif