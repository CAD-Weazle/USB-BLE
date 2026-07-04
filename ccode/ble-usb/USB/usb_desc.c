// file    : usb.c
// author  : st/ao/rb
// purpose : application specific USB code
// date    : 200101
// last    : 220214
//

#include "../includes.h"

// globals
//static int configured;

// Manufacterer String Descriptor
struct string_desc manuf_string __attribute__ ((aligned(4))) = 
{
  20,
  USB_DESC_TYPE_STRING,
  {'C', 'A', 'D', 'W', 'e', 'a', 'z', 'l', 'e'}
};

// Product String Descriptor
struct string_desc product_string __attribute__ ((aligned(4))) = 
{
  16,
  USB_DESC_TYPE_STRING,
  {'B', 'L', 'E', '-', 'U', 'S', 'B'}
};


// USB Standard Device Descriptor
static uint8_t DeviceQual[] __attribute__ ((aligned(4))) = 
{
  USB_LEN_DEV_QUALIFIER_DESC,     // bLength: 10
  USB_DESC_TYPE_DEVICE_QUALIFIER, // bDescriptorType: 6
  0x00,                           // bcdUSB: 2.1
  0x02, 
  0x00,                           // bDeviceClass: 0
  0x00,                           // bDeviceSubClass: 0
  0x00,                           // bDeviceProtocol: 0
  0x40,                           // bMaxPacketSize0: 64
  0x01,                           // bNumConfigurations: 1
  0x00,                           // bReserved: 0
};

// Device Descriptor 
static uint8_t DeviceDesc[18] __attribute__ ((aligned(4))) =
{
  0x12,                           // bLength: 18
  USB_DESC_TYPE_DEVICE,           // bDescriptorType: 1
  0x00,                           // bcdUSB: 2.1
  0x02,
  0x02,                           // bDeviceClass: CDC
  0x02,                           // bDeviceSubClass: ACM
  0x01,                           // bDeviceProtocol: AT Commands: V.250 etc
  USB_MAX_EP0_SIZE,               // MaxPacketSize: 64
  LOBYTE(VID),                    // idVendor: 
  HIBYTE(VID),                    
  LOBYTE(PID),                    // idProduct: UB (USB-BLE)
  HIBYTE(PID),
  0x00,                           // bcdDevice rel. 2.00
  0x00,
  0x01,                           // index of manufacturer string
  0x02,                           // index of product string
  0x03,                           // index of serial number string
  0x01                            // bNumConfigurations: 1
}; 

// Configuration Descriptor 
static uint8_t ConfigDesc[67] __attribute__ ((aligned(4))) =
{
  // Configuration Descriptor 
  0x09,                           // bLength: 9
  USB_DESC_TYPE_CONFIGURATION,    // bDescriptorType: 2
  LOBYTE(62),                     // LO(wTotalLength)
  HIBYTE(62),                     // HI(wTotalLength)
  0x02,                           // bNumInterfaces: 2
  0x01,                           // bConfigurationValue: 1
  0x00,                           // iConfiguration: 0
  0xc0,                           // bmAttributes: 0xc0 (self powered)
  0x32,                           // MaxPower: 100 mA
  
  // Interface descriptor 
  0x09,                           // bLength: 9
  USB_DESC_TYPE_INTERFACE,        // bDescriptorType: 4
  0x00,                           // bInterfaceNumber: 0
  0x00,                           // bAlternateSetting: 0
  0x01,                           // bNumEndpoints: 1
  0x02,                           // bInterfaceClass: 2 (Communications & CDC Control)
  0x02,                           // bInterfaceSubClass: 2 (Abstract)
  0x01,                           // bInterfaceProtocol: 1 (AT modem)
  0x00,                           // iInterface: 0
  
  // Header Functional Descriptor 
  0x05,                           // bLength: 5
  0x24,                           // bDescriptorType: 0x24 (CS_INTERFACE)
  0x00,                           // bDescriptorSubtype: 0 (Header Func Desc)
  0x10,                           // bcdCDC: spec release number
  0x01,
  
  /* Call Management Functional Descriptor */
//0x05,                           /* bFunctionLength */
//0x24,                           /* bDescriptorType: CS_INTERFACE */
//0x01,                           /* bDescriptorSubtype: Call Management Func Desc */
//0x00,                           /* bmCapabilities: D0+D1 */
//0x01,                           /* bDataInterface: 1 */

  // ACM Functional Descriptor 
  0x04,                           // bFunctionLength: 4
  0x24,                           // bDescriptorType: 0x24 (CS_INTERFACE)
  0x02,                           // bDescriptorSubtype: 2 (Abstract Control Management desc)
  0x06,                           // bmCapabilities: 6
  
  // Union Functional Descriptor 
  0x05,                           // bFunctionLength: 5
  0x24,                           // bDescriptorType: 0x24 (CS_INTERFACE)
  0x06,                           // bDescriptorSubtype: 6 (Union func desc)
  0x00,                           // bMasterInterface: 0 (Communication class interface)
  0x01,                           // bSlaveInterface0: 1 (Data Class Interface)
  
  // Endpoint IN2 Descriptor for CDC ACM interrupts
  0x07,                           // bLength: 7
  USB_DESC_TYPE_ENDPOINT,         // bDescriptorType: 5
  0x82,                           // bEndpointAddress: IN2 
  0x03,                           // bmAttributes: 3 (Interrupt)
  LOBYTE(8),                      // wMaxPacketSize: 8
  HIBYTE(8),                     
  0xff,                           // bInterval: 255 [frames] (polling interval)
  
  // Data class interface descriptor
  0x09,                           // bLength: 9
  USB_DESC_TYPE_INTERFACE,        // bDescriptorType: 4
  0x01,                           // bInterfaceNumber: 1
  0x00,                           // bAlternateSetting: 0
  0x02,                           // bNumEndpoints: 2 
  0x0a,                           // bInterfaceClass: 0x0a (CDC-Data)
  0x00,                           // bInterfaceSubClass: 0
  0x00,                           // bInterfaceProtocol: 0
  0x01,                           // iInterface: 1
  
  // Endpoint OUT1 Descriptor
  0x07,                           // bLength: 7
  USB_DESC_TYPE_ENDPOINT,         // bDescriptorType: 5
  0x01,                           // bEndpointAddress: 0x01 (OUT1)
  0x02,                           // bmAttributes: 2 (Bulk )
  LOBYTE(64),                     // wMaxPacketSize: 64
  HIBYTE(64),                     
  0x00,                           // bInterval: 0 (ignore for Bulk transfer)
                                
  // Endpoint IN1 Descriptor     
  0x07,                           // bLength: 7
  USB_DESC_TYPE_ENDPOINT,         // bDescriptorType: 5
  0x81,                           // bEndpointAddress : 0x81 (IN1)
  0x02,                           // bmAttributes: 2 (Bulk )
  LOBYTE(64),                     // wMaxPacketSize: 64
  HIBYTE(64),                   
  0x00,                           // bInterval: 0 (ignore for Bulk transfer)
} ;

// Language Descriptor
uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __attribute__ ((aligned(4))) = 
{
  USB_LEN_LANGID_STR_DESC,        // bLength: 4 
  USB_DESC_TYPE_STRING,           // bDescriptorType: 3
  LOBYTE(0x0409),                 // wLANGID: 0x0409 (English-US)
  HIBYTE(0x0409), 
};

// get Device Descriptor
uint8_t *GetDeviceDescriptor (int *length)
{
  *length = sizeof(DeviceDesc);
  return DeviceDesc;
}

// get Device Qualifier
uint8_t *GetDeviceQualifier (int *length)
{
  *length = sizeof(DeviceQual);
  return DeviceQual;
}

// get Configuration Descriptor
uint8_t *GetConfigDescriptor (int *length)
{
  *length = sizeof(ConfigDesc);
  return ConfigDesc;
}

// get Language ID Descriptor
uint8_t *GetLangIDStrDescriptor (int *length)
{
  *length = sizeof(USBD_LangIDDesc);  
  return USBD_LangIDDesc;
}

// CDC Class request
// Request Type: 0x20 = SetLineCoding       (0x21 0x20 0x0000 0x0000        0x7)
// Request Type: 0x21 = GetLineCoding       (0xa1 0x21 0x0000 0x<interface> 0x<len> Data)
// Request Type: 0x22 = SetControlLineState (0x21 0x22 0x0000 0x0000        0x0
void usb_if_class_req (USB_stup_req *setup)
{
  int len = setup->length;
  uint8_t buf[64];

  // 0xa1 0x21 0x0000 0x<interface> 0x<len> Data  
  if (setup->request_type & 0x80) // send to Host
  {
    // send: Device to Host
    memcpy (buf, codeline, 7);
    usb_transmit (0, buf, 7);
  }
  else // data from Host 
  {
    // <> note codeline is read in ISR

    // send ZLP (Zero Length Packet)
    USB_TX[0].DIEPTSIZ = (1 << 19) | 0;
    USB_TX[0].DIEPCTL |= (USB_OTG_DIEPCTL_CNAK | USB_OTG_DIEPCTL_EPENA);
  }
}

// set configuration
void usb_set_configuration (int cfgidx)
{
  usb_open_rx (1);   // CDC data
  usb_open_tx (1);
  usb_open_tx (2);   // CDC interrupt
}

// clear configuration <> not used
void usb_clear_configuration (int cfgidx)
{
}



////////// not used ///////////////////

/* <> not used
// get Product Descriptor
char *GetProductStrDescriptor (void)
{
  return "USB_BLE";
}
*/

/* <> not used
static char id_string[32];

char *serial_string (void)
{
  uint32_t *id = (uint32_t *) UID_BASE;
  sprintf( id_string, "%08lx-%08lx-%08lx", id[0], id[1], id[2] );
  return id_string;
}
*/

/* <> not used
char *GetConfigStrDescriptor (void)
{
  return "ConfigStr";
}

char *GetInterfaceStrDescriptor (void)
{
  return "swd";
}
*/



