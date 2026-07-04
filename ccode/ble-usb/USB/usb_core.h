// file    : usb_core.h
// author  : ao/rb
// purpose : header file for usb_core.c
// date    : 200628
// last    : 220217
//

#ifndef _USB_CORE_H_
#define _USB_CORE_H_

// -- prototypes
void usb_core (void);

int usb_configured (void);

void handle_setup (USB_stup_req *setup);

void usbd_device_request    (USB_stup_req *setup);
void usbd_interface_request (USB_stup_req *setup);
void usbd_endpoint_request  (USB_stup_req *setup);

void usbd_get_descriptor (USB_stup_req *setup);
void usbd_set_address    (USB_stup_req *setup);
void usbd_set_config     (USB_stup_req *setup);
void usbd_get_config     (USB_stup_req *setup);
void usbd_get_status     (USB_stup_req *setup);
void usbd_set_feature    (USB_stup_req *setup);
void usbd_clr_feature    (USB_stup_req *setup);

void usb_core_reset (void);

void usb_ctl_data   (void *buf, uint16_t len);
void usb_ctl_status (void);
void usb_ctl_error  (void);

#endif

