// file    : main.c
// author  : ao/rb
// purpose : USB CDC/ACM routines
// date    : 200628
// last    : 220217
//

#define SETUP_DEBUG        0

#include "../includes.h"

// globals
int      dev_remote_wakeup;
uint8_t  dev_config;
uint8_t  dev_default_config;
uint16_t dev_config_status; 
int      dev_state;

// poll for new SETUP packet & handle
void usb_core (void)
{ 
  if (stup_new)
  { 
    handle_setup (&usb_stup_req);
    stup_new = 0;
  }
}

// checkUSB up and running
int usb_configured (void)
{
  if (dev_state == USBD_STATE_CONFIGURED)
    return 1;
  else
    return 0;
}

// handle SETUP packet
void handle_setup (USB_stup_req *setup)
{
  int type = setup->request_type;
 
  // switch on Recipient type
  switch (type & 0x1f)  
  {
    // Standard Device Request    - 0x00 (type 0x00/0x80)
    case USB_REQ_RECIPIENT_DEVICE:
    {
      usbd_device_request (setup);
      break;
    }

    // Standard Interface Request - 0x01 (type 0x21/0xa1)
    case USB_REQ_RECIPIENT_INTERFACE:
    {
      usbd_interface_request (setup);
      break;
    }

    // Standard Endpoint Request  - 0x02
    case USB_REQ_RECIPIENT_ENDPOINT:
    {
      usbd_endpoint_request (setup);
      break;
    }

    default:
    {
      usb_ctl_error ();

      printf (">ERR: unknown request type: 0x%02x\n", type & 0x1f);
      break;
    }
  }

#if SETUP_DEBUG
  // dump SETUP packet <> dev only
  dump_stup_req ();
#endif
}

// handle Standard Device Request
void usbd_device_request (USB_stup_req *setup)
{
  switch (setup->request)
  {
    case USB_REQ_GET_DESCRIPTOR:
    {
      usbd_get_descriptor (setup);
      break;
    }

    case USB_REQ_SET_ADDRESS:
    {
      usbd_set_address (setup);
      break;
    }
    
    case USB_REQ_SET_CONFIGURATION:
    {
      usbd_set_config (setup);
      break;
    }

    case USB_REQ_GET_CONFIGURATION:
    {
      usbd_get_config (setup);
      break;
    }
    
    case USB_REQ_GET_STATUS:
    {
      usbd_get_status (setup);
      break;
    }
    
    case USB_REQ_SET_FEATURE:
    {
      usbd_set_feature (setup);
      break;
    }
    
    case USB_REQ_CLEAR_FEATURE:
    {
      usbd_clr_feature (setup);
      break;
    }
    
    default:
    {
      usb_ctl_error ();

      printf (">ERR: unknown device request: 0x%02x\n", setup->request);
      break;
    }  
  }
}

// handle Standard Interface Requests
void usbd_interface_request (USB_stup_req *setup)
{
  switch (dev_state)
  {
    case USBD_STATE_CONFIGURED:
    {
      if (LOBYTE (setup->index) <= USBD_MAX_NUM_INTERFACES)
      {
        int type = setup->request_type & USB_REQ_TYPE_MASK;
        
        if (type == USB_REQ_TYPE_CLASS)
        {
          usb_if_class_req (setup);
        }
        else if (!setup->length)
        {
          usb_ctl_status ();
        }
      }
      else
      {
        usb_ctl_error ();
      }
      break;
    }
 
    default:
    {
      usb_ctl_error ();

      printf (">ERR: wrong interface request\n");
      break;
    }
  }
}

// handle Standard Endpoint Request
void usbd_endpoint_request (USB_stup_req *setup)
{
  int epadr = setup->index & 0x00ff;
  int epnum = epadr & 0x03;           // EP0..EP3 only
  int status;
  
  switch (setup->request)
  {
    // get Endpoint status - 0x00
    case USB_REQ_GET_STATUS:
    {
      switch (dev_state)
      {
        case USBD_STATE_ADDRESSED:
        {
          if (epadr != 0x00 && epadr != 0x80)
          {
            usb_ctl_error ();
            break;
          }

          status = 0;
          usb_ctl_data (&status, 2);
	    }

        case USBD_STATE_CONFIGURED:
        {
          if ((epadr == 0x00) && (epadr == 0x80))
          {
            status = 0;
          } 
          else if (usb_is_stalled (epadr))
          {
            status = 1;
          }
          else
          {
            status = 0;
          }
         
          usb_ctl_data (&status, 2);
          break;
        }
   
        default:
        {
          usb_ctl_error ();
          break;
        }
      }
      break;
    }

    // clear Endpoint halt state - 0x01
    case USB_REQ_CLEAR_FEATURE:
    {
      switch (dev_state)
      {
        case USBD_STATE_ADDRESSED:
        {
          if ((epadr != 0x00) && (epadr != 0x80))
          {
            usb_set_stall (epadr);
            usb_set_stall (0x80);
          }
          else
          {
            usb_ctl_error ();
          }

          break;	
        }

        case USBD_STATE_CONFIGURED:
        {
          if (setup->value == USB_FEATURE_EP_HALT) // == 0x00
          {
            if (epadr & 0x7f)
            {
              usb_clr_stall (epadr);
            }

            usb_ctl_status ();
          }
          break;
	    }

        default:
        {
          usb_ctl_error ();
          break;
        }
      }
      break;
    }

    // set Endpoint halt state - 0x03
    case USB_REQ_SET_FEATURE:
    {
      switch (dev_state)
      {
        case USBD_STATE_ADDRESSED:
        {
          if ((epadr != 0x00) && (epadr != 0x80))
          {
            usb_set_stall (epadr);
            usb_set_stall (0x80);
          }
          else
          {
            usb_ctl_error ();
          }
          break;	
        }

        case USBD_STATE_CONFIGURED:
        {
          if (setup->value == USB_FEATURE_EP_HALT)
          {
            if ((epadr != 0x00) && (epadr != 0x80) && (setup->length == 0))
            {
              usb_set_stall (epadr);
            }
          }

          usb_ctl_status();
          break;
        }
 
        default:
        {
          usb_ctl_error ();
          break;
        }
      }
      break;
    }

    default:
    {
      usb_ctl_error ();

      printf (">ERR: unknown endpoint request: 0x%02x\n", setup->request);
      break;
    }
  }
}

// handle Get Descriptor Request
void usbd_get_descriptor (USB_stup_req *setup)
{
  int len;
  uint16_t *buf;
  struct string_desc *str;

  switch (setup->value >> 8)
  {
    case USB_DESC_TYPE_DEVICE:
    {
      buf = (uint16_t *) GetDeviceDescriptor (&len);
      break;
    }

    case USB_DESC_TYPE_CONFIGURATION:
    {
      buf = (uint16_t *)GetConfigDescriptor (&len);
    //buf[1] = len;    // fill in wTotalLength
      break;
    }

    case USB_DESC_TYPE_STRING:
    {
      switch (setup->value & 0xff)
      {
        case USBD_IDX_LANGID_STR:
        {
          buf = (uint16_t *) GetLangIDStrDescriptor (&len);
          break;
        }

        case USBD_IDX_MFC_STR:
        {
          str = &manuf_string;
          len = str->length;
          buf = (uint16_t *) str;
          break;
        }

        case USBD_IDX_PRODUCT_STR:
        {
          str = &product_string;
          len = str->length;
          buf = (uint16_t *) str;
          break;
        }

        case USBD_IDX_SERIAL_STR:
        {
          str = &product_string;
          len = str->length;
          buf = (uint16_t *) str;
          break;
        }
#if 0
        case USBD_IDX_CONFIG_STR:
        {
          buf = unicode (GetConfigStrDescriptor(), &len);
          break;
        }

        case USBD_IDX_INTERFACE_STR:
        {
          buf = unicode (GetInterfaceStrDescriptor(), &len);
          break;
        }
#endif
        default:
          usb_ctl_error ();
          return;
      }
      break;
    }

    case USB_DESC_TYPE_DEVICE_QUALIFIER:
    {
      buf = (uint16_t *)GetDeviceQualifier(&len);
      break;
    }
       
    default:
    {
      usb_ctl_error ();

      printf (">ERR: unknown descriptor: 0x%02x\n", setup->value >> 8);
      return;
    }
  }

  // get smallest length & transmit via IN0 Endpoint
  len = MIN (len, setup->length);
  usb_transmit (0, buf, len);
}

// handle Get Descriptor Request
void usb_get_descriptor (USB_stup_req setup)
{
  int len;
  uint16_t *buf;
  struct string_desc *str;

  switch (usb_stup_req.value >> 8)
  {
    // return Device Descriptor - 0x01
    case USB_DESC_TYPE_DEVICE:
    {
      buf = (uint16_t *)GetDeviceDescriptor (&len);
      break;
    }
  
    // return Configuration Descriptor - 0x02
    case USB_DESC_TYPE_CONFIGURATION:
    {
      buf = (uint16_t *)GetConfigDescriptor (&len);
      break;
    }
  
    // return String - 0x03
    case USB_DESC_TYPE_STRING:
    {
      switch (usb_stup_req.value & 0x00ff)
      {
        // Language ID String - 0x00
        case USBD_IDX_LANGID_STR:
        {
          buf = (uint16_t *)GetLangIDStrDescriptor (&len);
          break;
        }

        // Manufacterer String - 0x01
        case USBD_IDX_MFC_STR:
        {
          str = &manuf_string;
          len = str->length;
          buf = (uint16_t *)str;
          break;
        }

        // Product String - 0x02
        case USBD_IDX_PRODUCT_STR:
        {
          str = &product_string;
          len = str->length;
          buf = (uint16_t *)str;
          break;
        }

        // Serial Number string - 0x03 <> also Product String??
        case USBD_IDX_SERIAL_STR:
        {
          str = &product_string;
          len = str->length;
          buf = (uint16_t *)str;
          break;
        }
#if 0
        case USBD_IDX_CONFIG_STR:
        {
          buf = unicode (GetConfigStrDescriptor(), &len);
          break;
        }

        case USBD_IDX_INTERFACE_STR:
        {
          buf = unicode (GetInterfaceStrDescriptor(), &len);
          break;
        }
#endif
        default:
          usb_ctl_error ();
          return; 
      }
      break;
    }

    // return Device Qualifier - 0x06
    case USB_DESC_TYPE_DEVICE_QUALIFIER:
    {
      buf = (uint16_t *)GetDeviceQualifier (&len);
      break;
    }
     
    default:
    {
      usb_ctl_error ();

      printf (">ERROR: get unknown descriptor: %d\n", usb_stup_req.value >> 8);
      return;
    }
  }  
  
  // return Descriptor (packet length is smallest of Descriptor Length & requested length)
  len = MIN(len, usb_stup_req.length);
  usb_transmit (0, buf, len);
}

// handle Set Device Address Request
void usbd_set_address (USB_stup_req *setup)
{
  if (!setup->index && !setup->length)
  {
    int addr = setup->value & 0x7f;
  
    if (dev_state == USBD_STATE_CONFIGURED)
    {
      usb_ctl_error ();
    }
    else
    {
      usb_set_address (addr);
      usb_ctl_status ();

      dev_state = USBD_STATE_ADDRESSED;
    }
  }
  else
  {
    usb_ctl_error ();
  }
}

// handle Set Device Configuration Request
void usbd_set_config (USB_stup_req *setup)
{
  static uint8_t cfgidx;
  int newcfg = setup->value & 0xff;
    
  if (newcfg > USBD_MAX_NUM_CONFIGURATION)
  {
    usb_ctl_error ();
    return;
  }

  cfgidx = newcfg;

  switch (dev_state)
  {
    case USBD_STATE_ADDRESSED:
    {
      if (cfgidx)
      {                
        dev_config = cfgidx;
        dev_state = USBD_STATE_CONFIGURED;
        usb_set_configuration (cfgidx);
      }

      usb_ctl_status ();
      break;
    }

    case USBD_STATE_CONFIGURED:
    {
      if (cfgidx == 0)
      {
        dev_state = USBD_STATE_ADDRESSED;
        dev_config = cfgidx;
      }
      else if (cfgidx != dev_config)
      {
        // set new configuration
        dev_config = cfgidx;
      }

      usb_ctl_status ();
      break;
    }

    default:					
    {
      usb_ctl_error ();
      break;
    }
  }
}

// handle Get Device Configuration Request
void usbd_get_config (USB_stup_req *setup)
{
  if (setup->length != 1)
  {
    usb_ctl_error ();
  }
  else
  {
    switch (dev_state)
    {
      case USBD_STATE_ADDRESSED:
      {
        dev_default_config = 0;
        usb_ctl_data (&dev_default_config, 1);
        break;
      }

      case USBD_STATE_CONFIGURED:
      {
        usb_ctl_data (&dev_config, 1);
        break;
      }

      default:
      {
        usb_ctl_error ();
        break;
      }
    }
  }
}

// handle Get Status Request
void usbd_get_status (USB_stup_req *setup)
{
  switch (dev_state)
  {
    case USBD_STATE_ADDRESSED:
    case USBD_STATE_CONFIGURED:
    {
      dev_config_status = USB_CONFIG_SELF_POWERED;
    
      if (dev_remote_wakeup)
      {
        dev_config_status |= USB_CONFIG_REMOTE_WAKEUP;
      }
    
      usb_ctl_data (&dev_config_status, 2);
      break;
    }

    default:
    {
      usb_ctl_error ();
      break;
    }
  }
}

// handle Set Device Feature Request
void usbd_set_feature (USB_stup_req *setup)
{
  if (setup->value == USB_FEATURE_REMOTE_WAKEUP)
  {
    dev_remote_wakeup = 1;
    usb_ctl_status ();
  }
}

// handle Clear Device Feature Request
void usbd_clr_feature (USB_stup_req *setup)
{
  switch (dev_state)
  {
    case USBD_STATE_ADDRESSED:
    case USBD_STATE_CONFIGURED:
    {
      if (setup->value == USB_FEATURE_REMOTE_WAKEUP)
      {
        dev_remote_wakeup = 0;
        usb_ctl_status ();
      }
      break;
    }
 
    default:
    {
      usb_ctl_error ();
      break;
    }
  }
}

// set state on USB Reset event
void usb_core_reset (void)
{
  dev_state = USBD_STATE_DEFAULT;
}

// send Control Data Endpoint 0
void usb_ctl_data (void *buf, uint16_t len)
{
  usb_transmit (0, buf, len);
}

// send Control Status Endpoint 0
void usb_ctl_status (void)
{
  // send ZLP (Zero Length Packet)
  USB_TX[0].DIEPTSIZ = (1 << 19) | 0;
  USB_TX[0].DIEPCTL |= (USB_OTG_DIEPCTL_CNAK | USB_OTG_DIEPCTL_EPENA);
}

// signal error Endpoint 0 
void usb_ctl_error (void)
{
  usb_set_stall (0x80);
  usb_set_stall (0x00);

  // HAL code:
//(void)USBD_LL_StallEP(pdev, 0x80U);
//(void)USBD_LL_StallEP(pdev, 0U);
}

// convert USB endpoint number to internal representation
int addr_to_num (int addr)
{
  return ((addr & 7) << 1) + !(addr & 0x80);
}
uint16_t unicode_buf[USBD_MAX_STR_DESC_SIZ];

// convert ascii string to unicode
uint16_t *unicode (char *ascii, int *len)
{
  int idx = 0;
  
  *len = strlen(ascii)*2 + 2;

  unicode_buf[idx++] = *len + (USB_DESC_TYPE_STRING << 8);

  while (*ascii)
    unicode_buf[idx++] = *ascii++;

  return unicode_buf;
}

